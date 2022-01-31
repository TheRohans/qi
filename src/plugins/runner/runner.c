/**
 * Runner mode for Qi.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "qe.h"

#define MAX_ESC_PARAMS 3
#define	PTYCHAR1 "pqrstuvwxyz"
#define	PTYCHAR2 "0123456789abcdef"
#define TTY_YSIZE 25

static int error_offset = -1;
static int last_line_num = -1;
static char last_filename[1024];

enum TTYState {
    TTY_STATE_NORM,
    TTY_STATE_ESC,
    TTY_STATE_CSI,
};

typedef struct ShellState {
    /* buffer state */
    int pty_fd;
    int pid; /* -1 if not launched */
    int color, def_color;
    int cur_offset; /* current offset at position x, y */
    int esc_params[MAX_ESC_PARAMS];
    int nb_esc_params;
    int state;
    EditBuffer *b;
    EditBuffer *b_color; /* color buffer, one byte per char */
    int is_shell; /* only used to display final message */
} ShellState;


///////////////////////////////

static void tty_write(ShellState *s, char *buf, int len)
{
    int ret;

    if (len < 0)
        len = strlen((char *)buf);
    while (len > 0) {
        ret = write(s->pty_fd, buf, len);
        if (ret == -1 && (errno == EAGAIN || errno == EINTR))
            continue;
        if (ret <= 0)
            break;
        buf += ret;
        len -= ret;
    }
}

/* compute offset the char at 'x' and 'y'. Can insert spaces or lines
   if needed */
/* XXX: optimize !!!!! */
static void tty_gotoxy(ShellState *s, int x, int y)
{
    int total_lines, line_num, col_num, offset, offset1, c;
    unsigned char buf1[10];

    /* compute offset */
    eb_get_pos(s->b, &total_lines, &col_num, s->b->total_size);
    line_num = total_lines - TTY_YSIZE;
    if (line_num < 0)
        line_num = 0;
    line_num += y;
    /* add lines if necessary */
    while (line_num >= total_lines) {
        buf1[0] = '\n';
        eb_insert(s->b, s->b->total_size, buf1, 1);
        total_lines++;
    }
    offset = eb_goto_pos(s->b, line_num, 0);
    for(;x > 0; x--) {
        c = eb_nextc(s->b, offset, &offset1);
        if (c == '\n') {
            buf1[0] = ' ';
            for(;x > 0; x--) {
                eb_insert(s->b, offset, buf1, 1);
            }
            break;
        } else {
            offset = offset1;
        }
    }
    s->cur_offset = offset;
}

void tty_csi_m(ShellState *s, int c)
{
    /* we handle only a few possible modes */
    switch(c) {
    case 0:
        s->color = s->def_color;
        break;
    default:
        if (c >= 30 && c <= 37)
            s->color = TTY_GET_COLOR(c - 30, TTY_GET_BG(s->color));
        else if (c >= 40 && c <= 47) {
            s->color = TTY_GET_COLOR(TTY_GET_FG(s->color), c - 40);
        }
        break;
    }
}

/* Well, almost a hack to update cursor */
static void tty_update_cursor(ShellState *s)
{
    QEmacsState *qs = &qe_state;
    EditState *e;

    if (s->cur_offset == -1)
        return;

    for(e = qs->first_window; e != NULL; e = e->next_window) {
        if (s->b == e->b && e->interactive) {
            e->offset = s->cur_offset;
        }
    }
}

static void tty_emulate(ShellState *s, int c)
{
    int i, offset, offset1, offset2, n;
    unsigned char buf1[10];
    
    switch(s->state) {
    case TTY_STATE_NORM:
        switch(c) {
        case 8:
            {
                int c1;
                c1 = eb_prevc(s->b, s->cur_offset, &offset);
                if (c1 != '\n')
                    s->cur_offset = offset;
            }
            break;
        case 10:
            /* go to next line */
            offset = s->cur_offset;
            for(;;) {
                if (offset == s->b->total_size) {
                    /* add a new line */
                    buf1[0] = '\n';
                    eb_insert(s->b, offset, buf1, 1);
                    offset = s->b->total_size;
                    break;
                }
                c = eb_nextc(s->b, offset, &offset);
                if (c == '\n')
                    break;
            }
            s->cur_offset = offset;
            break;
        case 13:
            /* move to bol */
            for(;;) {
                c = eb_prevc(s->b, s->cur_offset, &offset1);
                if (c == '\n')
                    break;
                s->cur_offset = offset1;
            }
            break;
        case 27:
            s->state = TTY_STATE_ESC;
            break;
        default:
            if (c >= 32 || c == 9) {
                int c1, cur_len, len;
                /* write char (should factorize with do_char() code */
                len = unicode_to_charset(buf1, c, s->b->charset);
                c1 = eb_nextc(s->b, s->cur_offset, &offset);
                /* XXX: handle tab case */
                if (c1 == '\n') {
                    /* insert */
                    eb_insert(s->b, s->cur_offset, buf1, len);
                } else {
                    cur_len = offset - s->cur_offset;
                    if (cur_len == len) {
                        eb_write(s->b, s->cur_offset, buf1, len);
                    } else {
                        eb_delete(s->b, s->cur_offset, cur_len);
                        eb_insert(s->b, s->cur_offset, buf1, len);
                    }
                }
                s->cur_offset += len;
            }
            break;
        }
        break;
    case TTY_STATE_ESC:
        if (c == '[') {
            for(i=0;i<MAX_ESC_PARAMS;i++)
                s->esc_params[i] = 0;
            s->nb_esc_params = 0;
            s->state = TTY_STATE_CSI;
        } else {
            s->state = TTY_STATE_NORM;
        }
        break;
    case TTY_STATE_CSI:
        if (c >= '0' && c <= '9') {
            if (s->nb_esc_params < MAX_ESC_PARAMS) {
                s->esc_params[s->nb_esc_params] = 
                    s->esc_params[s->nb_esc_params] * 10 + c - '0';
            }
        } else {
            s->nb_esc_params++;
            if (c == ';')
                break;
            s->state = TTY_STATE_NORM;
            switch(c) {
            case 'H':
                /* goto xy */
                {
                    int x, y;
                    y = s->esc_params[0] - 1;
                    x = s->esc_params[1] - 1;
                    if (y < 0)
                        y = 0;
                    else if (y >= TTY_YSIZE)
                        y = TTY_YSIZE - 1;
                    if (x < 0)
                        x = 0;
                    tty_gotoxy(s, x, y);
                }
                break;
            case 'K':
                /* clear to eol */
                offset1 = s->cur_offset;
                for(;;) {
                    c = eb_nextc(s->b, offset1, &offset2);
                    if (c == '\n')
                        break;
                    offset1 = offset2;
                }
                eb_delete(s->b, s->cur_offset, offset1 - s->cur_offset);
                break;
            case 'P':
                /* delete chars */
                n = s->esc_params[0];
                if (n <= 0)
                    n = 1;
                offset1 = s->cur_offset;
                for(;n > 0;n--) {
                    c = eb_nextc(s->b, offset1, &offset2);
                    if (c == '\n')
                        break;
                    offset1 = offset2;
                }
                eb_delete(s->b, s->cur_offset, offset1 - s->cur_offset);
                break;
            case '@':
                /* insert chars */
                n = s->esc_params[0];
                if (n <= 0)
                    n = 1;
                buf1[0] = ' ';
                for(;n > 0;n--) {
                    eb_insert(s->b, s->cur_offset, buf1, 1);
                }
                break;
            case 'm':
                /* colors */
                n = s->nb_esc_params;
                if (n == 0)
                    n = 1;
                for(i=0;i<n;i++)
                    tty_csi_m(s, s->esc_params[i]);
                break;
            case 'n':
                if (s->esc_params[0] == 6) {
                    /* XXX: send cursor position, just to be able to
                       launch qemacs in qemacs !
                    char buf2[20];
                    snprintf(buf2, sizeof(buf2), "\033[%d;%dR", 1, 1);
                    tty_write(s, buf2, -1);
                    */
                }
                break;
            default:
                break;
            }
        }
        break;
    }
    tty_update_cursor(s);
}

/////////////////////////////////////////////////////////////////////////

static void tty_init(ShellState *s)
{
    s->state = TTY_STATE_NORM;
    s->cur_offset = 0;
    s->def_color = TTY_GET_COLOR(7, 0);
    s->color = s->def_color;
}

/* allocate one pty/tty pair (old way) */
static int get_pty_old(char *tty_str, int buf_size)
{
   int fd;
   char ptydev[] = "/dev/pty??";
   char ttydev[] = "/dev/tty??";
   int len = strlen (ttydev);
   char *c1, *c2;

   for (c1 = PTYCHAR1; *c1; c1++) {
       ptydev [len-2] = ttydev [len-2] = *c1;
       for (c2 = PTYCHAR2; *c2; c2++) {
           ptydev [len-1] = ttydev [len-1] = *c2;
           if ((fd = open (ptydev, O_RDWR)) >= 0) {
               if (access (ttydev, R_OK|W_OK) == 0) {
                   pstrcpy(tty_str, buf_size, ttydev);
                   return fd;
               }
               close (fd);
           }
       }
   }
   return -1;
}

/* allocate one pty/tty pair (Unix 98 way) */
static int get_pty(char *tty_buf, int tty_buf_size)
{
    int fd;
    char *str;

    fd = posix_openpt(O_RDWR | O_NOCTTY);
    if (fd < 0) {
        return get_pty_old(tty_buf, tty_buf_size);
    }
    if (grantpt(fd) < 0)
        goto fail;
    if (unlockpt(fd) < 0)
        goto fail;
    
    str = ptsname(fd);
    if (!str)
        goto fail;
    
    pstrcpy(tty_buf, tty_buf_size, str);
    
    return fd;
 fail:
    close(fd);
    return -1;
}

static int run_process(const char *path, char **argv, 
                       int *fd_ptr, int *pid_ptr)
{
    int pty_fd, pid, i, nb_fds;
    char tty_name[1024];
    struct winsize ws;

    pty_fd = get_pty(tty_name, sizeof(tty_name));
    fcntl(pty_fd, F_SETFL, O_NONBLOCK);
    if (pty_fd < 0)
        return -1;

    /* set dummy screen size */
    ws.ws_col = 80;
    ws.ws_row = 25;
    ws.ws_xpixel = ws.ws_col;
    ws.ws_ypixel = ws.ws_row;
    ioctl(pty_fd, TIOCSWINSZ, &ws);
    
    pid = fork();
    if (pid < 0)
        return -1;
    if (pid == 0) {
        /* child process */
        nb_fds = getdtablesize();
        for(i=0;i<nb_fds;i++)
            close(i);
        /* open pseudo tty for standard i/o */
        open(tty_name, O_RDWR);
        dup(0);
        dup(0);

        setsid();

        execv(path, (char *const*)argv);
        exit(1);
    }
    /* return file info */
    *fd_ptr = pty_fd;
    *pid_ptr = pid;
    return 0;
}

/////////////////////////////////////////////////////////////////////////

static void shell_close(EditBuffer *b)
{
    ShellState *s = b->priv_data;
    int status;

    eb_free_callback(b, eb_offset_callback, &s->cur_offset);

    if (s->pid != -1) {
        kill(s->pid, SIGINT);
        /* wait first 100 ms */
        usleep(100 * 1000);
        if (waitpid(s->pid, &status, WNOHANG) != s->pid) {
            /* if still not killed, then try harder (useful for
               shells) */
            kill(s->pid, SIGKILL);
            while (waitpid(s->pid, &status, 0) != s->pid);
        }
        s->pid = -1;
    }
    if (s->pty_fd >= 0) {
        set_read_handler(s->pty_fd, NULL, NULL);
    }
    free(s);
}

/* modify the color according to the current one (may be incorrect if
   we are editing because we should write default color) */
static void shell_color_callback(EditBuffer *b,
                                 void *opaque,
                                 enum LogOperation op,
                                 int offset,
                                 int size)
{
    ShellState *s = opaque;
    unsigned char buf[32];
    int len;

    switch(op) {
    case LOGOP_WRITE:
        while (size > 0) {
            len = size;
            if (len > sizeof(buf))
                len = sizeof(buf);
            memset(buf, s->color, len);
            eb_write(s->b_color, offset, buf, len);
            size -= len;
            offset += len;
        }
        break;
    case LOGOP_INSERT:
        while (size > 0) {
            len = size;
            if (len > sizeof(buf))
                len = sizeof(buf);
            memset(buf, s->color, len);
            eb_insert(s->b_color, offset, buf, len);
            size -= len;
        }
        break;
    case LOGOP_DELETE:
        eb_delete(s->b_color, offset, size);
        break;
    default:
        break;
    }
}

static void shell_read_cb(void *opaque)
{
    ShellState *s = opaque;
    unsigned char buf[1024];
    int len, i;

    len = read(s->pty_fd, buf, sizeof(buf));
    if (len <= 0)
        return;
    
    for(i=0;i<len;i++) tty_emulate(s, buf[i]);

    /* now we do some refresh */
    edit_display(&qe_state);
    dpy_flush(qe_state.screen);
}

void shell_pid_cb(void *opaque, int status)
{
    ShellState *s = opaque;
    EditBuffer *b = s->b;
    QEmacsState *qs = &qe_state;
    EditState *e;
    char buf[1024];

    if (s->is_shell) {
        snprintf(buf, sizeof(buf), "\nProcess shell finished\n");
    } else {
        time_t ti;
        char *time_str;
        
        ti = time(NULL);
        time_str = ctime(&ti);
        if (WIFEXITED(status))
            status = WEXITSTATUS(status);
        else
            status = -1;
        if (status == 0) {
            snprintf(buf, sizeof(buf), "\nCompilation finished at %s", 
                     time_str);
        } else {
            snprintf(buf, sizeof(buf), "\nCompilation exited abnormally with code %d at %s", 
                     status, time_str);
        }
    }
    eb_write(b, b->total_size, (unsigned char *)buf, strlen(buf));
    set_pid_handler(s->pid, NULL, NULL);
    s->pid = -1;
    /* no need to leave the pty opened */
    if (s->pty_fd >= 0) {
        set_read_handler(s->pty_fd, NULL, NULL);
        close(s->pty_fd);
        s->pty_fd = -1;
    }

    /* remove shell input mode */
    for(e = qs->first_window; e != NULL; e = e->next_window) {
        if (e->b == b)
            e->interactive = 0;
    }
    edit_display(&qe_state);
    dpy_flush(qe_state.screen);
}

EditBuffer *new_runner_buffer(const char *name, const char *path,
                             const char **argv, int is_shell)
{
    ShellState *s;
    EditBuffer *b, *b_color;

    b = eb_new("", BF_SAVELOG);
    if (!b)
        return NULL;
    set_buffer_name(b, name); /* ensure that the name is unique */
    eb_set_charset(b, &charset_vt100);

    s = malloc(sizeof(ShellState));
    if (!s) {
        eb_free(b);
        return NULL;
    }
    memset(s, 0, sizeof(ShellState));
    b->priv_data = s;
    b->close = shell_close;
    eb_add_callback(b, eb_offset_callback, &s->cur_offset);
    s->b = b;
    s->pty_fd = -1;
    s->pid = -1;
    s->is_shell = is_shell;
    // s->qe_state = &qe_state;
    tty_init(s);

    /* add color buffer */
    if (is_shell) {
        b_color = eb_new("*color*", BF_SYSTEM);
        if (!b_color) {
            eb_free(b);
            free(s);
            return NULL;
        }
        /* no undo info in this color buffer */
        b_color->save_log = 0;
        eb_add_callback(b, shell_color_callback, s);
        // s->b_color = b_color;
    }

    /* launch shell */
    if (run_process(path, argv, 
                    &s->pty_fd, &s->pid) < 0) {
        eb_free(b);
        return NULL;
    }

    set_read_handler(s->pty_fd, shell_read_cb, s);
    set_pid_handler(s->pid, shell_pid_cb, s);
    return b;
}

static void do_runner(EditState *e, const char *cmd)
{
    const char *argv[4];
    EditBuffer *b;

    /* if the buffer already exists, kill it */
    b = eb_find("*compilation*");
    if (b) {
        /* XXX: e should not become invalid */
        b->modified = 0;
        do_kill_buffer(e, "*compilation*");
    }

    error_offset = -1;
    last_line_num = -1;
    
    /* create new buffer */
    argv[0] = "/bin/sh";
    argv[1] = "-c";
    argv[2] = (char *)cmd;
    argv[3] = NULL;
    b = new_runner_buffer("*compilation*", "/bin/sh", argv, 0);
    if (!b)
        return;
    
    /* XXX: try to split window if necessary */
    switch_to_buffer(e, b);
}

/* specific runner commands */
static CmdDef runner_commands[] = {
    CMD_( KEY_CTRLX(KEY_CTRL('r')), KEY_NONE, "runner", do_runner,
          "s{Runner command: }|runner|")
    CMD_DEF_END,
};


int runner_init(void)
{
    /* commands and default keys */
    qe_register_cmd_table(runner_commands, "runner");
    
    return 0;
}

