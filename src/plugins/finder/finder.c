/*
 * finder.c - fuzzy file opener and content search for qi
 *
 * M-o  finder-open  Live fuzzy file search; Ctrl+N/P to navigate, Enter to open.
 * M-s  finder-grep  Grep across files; results in navigable popup.
 *
 * File list is built once (lazily on first M-o) by running "rg --files"
 * with a fallback to "find . -type f".
 * Content search runs "rg --vimgrep" with a fallback to "grep -rn".
 */
#include "qe.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>

/* -------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------- */
#define FINDER_FILES_MAX   65536
#define FINDER_READ_BUF    (64 * 1024)
#define FINDER_DISPLAY_MAX 200

/* -------------------------------------------------------------------------
 * State
 * ------------------------------------------------------------------------- */
typedef struct {
    /* async file list (rg --files / find) */
    StringArray files;
    int         files_ready;
    int         files_pid;
    int         files_fd;
    char        files_buf[FINDER_READ_BUF];
    int         files_len;

    /* open-file popup */
    EditState  *opener_es;
    EditState  *open_popup;
    char        last_input[512];  /* detect changes in display_hook */

    /* grep popup */
    int         grep_pid;
    int         grep_fd;
    char        grep_buf[FINDER_READ_BUF];
    int         grep_len;
    int         grep_count;
    EditState  *grep_target_es;
    EditState  *grep_popup;
} FinderState;

static FinderState finder_state;
static ModeDef     finder_minibuf_mode;
static ModeDef     grep_results_mode;
static StringArray finder_hist;
static StringArray grep_hist;

/* -------------------------------------------------------------------------
 * Helpers
 * ------------------------------------------------------------------------- */

static int finder_window_alive(EditState *target)
{
    QEmacsState *qs = &qe_state;
    for (EditState *w = qs->first_window; w; w = w->next_window)
        if (w == target) return 1;
    return 0;
}

/* Case-insensitive fuzzy match: all pattern chars appear in str in order. */
static int fuzzy_match(const char *pat, const char *str)
{
    if (!*pat) return 1;
    while (*pat && *str) {
        if (tolower((unsigned char)*pat) == tolower((unsigned char)*str))
            pat++;
        str++;
    }
    return *pat == '\0';
}

/* -------------------------------------------------------------------------
 * Async file-list scan (rg --files → find fallback)
 * ------------------------------------------------------------------------- */

static void finder_files_exit(void *opaque, int status)
{
    (void)opaque; (void)status;
    QEmacsState *qs = &qe_state;
    set_read_handler(finder_state.files_fd, NULL, NULL);
    close(finder_state.files_fd);
    finder_state.files_fd  = -1;
    finder_state.files_pid = 0;
    finder_state.files_ready = 1;
    if (qs->active_window)
        put_status(qs->active_window, "Finder: %d files indexed",
                   finder_state.files.nb_items);
}

static void finder_files_handler(void *opaque)
{
    (void)opaque;
    int space = FINDER_READ_BUF - finder_state.files_len - 1;
    if (space <= 0) return;
    int n = read(finder_state.files_fd,
                 finder_state.files_buf + finder_state.files_len, space);
    if (n <= 0) return;
    finder_state.files_len += n;
    finder_state.files_buf[finder_state.files_len] = '\0';

    char *buf = finder_state.files_buf;
    char *end = finder_state.files_buf + finder_state.files_len;
    char *nl;
    while ((nl = (char *)memchr(buf, '\n', end - buf)) != NULL) {
        *nl = '\0';
        if (buf[0] && finder_state.files.nb_items < FINDER_FILES_MAX)
            add_string(&finder_state.files, buf);
        buf = nl + 1;
    }
    int remaining = (int)(end - buf);
    if (remaining > 0)
        memmove(finder_state.files_buf, buf, remaining);
    finder_state.files_len = remaining;
}

static void finder_start_file_scan(void)
{
    if (finder_state.files_pid || finder_state.files_ready)
        return;

    int pfd[2];
    if (pipe(pfd) < 0) return;

    int pid = fork();
    if (pid < 0) { close(pfd[0]); close(pfd[1]); return; }
    if (pid == 0) {
        dup2(pfd[1], STDOUT_FILENO);
        close(pfd[0]); close(pfd[1]);
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) { dup2(devnull, STDERR_FILENO); close(devnull); }
        execlp("rg", "rg", "--files", NULL);
        execlp("find", "find", ".", "-type", "f", "-not", "-path", "*/.*", NULL);
        exit(1);
    }
    close(pfd[1]);
    finder_state.files_fd  = pfd[0];
    finder_state.files_pid = pid;
    set_read_handler(pfd[0], finder_files_handler, NULL);
    set_pid_handler(pid, finder_files_exit, NULL);
}

/* -------------------------------------------------------------------------
 * Open-file popup management
 * ------------------------------------------------------------------------- */

static void finder_close_open_popup(void)
{
    if (!finder_state.open_popup) return;
    EditBuffer *pb = finder_state.open_popup->b;
    edit_close(finder_state.open_popup);
    eb_free(pb);
    finder_state.open_popup = NULL;
}

static void finder_populate_popup(const char *input)
{
    if (!finder_state.open_popup) return;

    EditBuffer *b = finder_state.open_popup->b;
    b->flags &= ~BF_READONLY;
    eb_delete(b, 0, b->total_size);

    if (!finder_state.files_ready) {
        eb_printf(b, " (indexing files...)\n");
        b->flags |= BF_READONLY;
        finder_state.open_popup->offset = 0;
        return;
    }

    int count = 0;
    for (int i = 0; i < finder_state.files.nb_items && count < FINDER_DISPLAY_MAX; i++) {
        const char *path  = finder_state.files.items[i]->str;
        const char *fname = strrchr(path, '/');
        fname = fname ? fname + 1 : path;
        if (!input[0] || fuzzy_match(input, fname) || fuzzy_match(input, path)) {
            eb_printf(b, " %s\n", path);
            count++;
        }
    }
    if (count == 0)
        eb_printf(b, " (no matches)\n");

    b->flags |= BF_READONLY;
    finder_state.open_popup->offset = 0;
    finder_state.open_popup->force_highlight = 1;
}

/* display_hook: called each time the minibuffer is rendered; re-filters on change */
static void finder_display_hook(EditState *s)
{
    char input[512];
    eb_get_str(s->b, input, sizeof(input));
    if (strcmp(input, finder_state.last_input) == 0)
        return;
    pstrcpy(finder_state.last_input, sizeof(finder_state.last_input), input);
    finder_populate_popup(input);
}

/* -------------------------------------------------------------------------
 * Finder minibuffer commands (Ctrl+N/P navigate popup, Enter/Esc confirm/abort)
 * ------------------------------------------------------------------------- */

static void finder_next(EditState *s)
{
    (void)s;
    if (finder_state.open_popup)
        do_up_down(finder_state.open_popup, 1);
}

static void finder_prev(EditState *s)
{
    (void)s;
    if (finder_state.open_popup)
        do_up_down(finder_state.open_popup, -1);
}

static void finder_abort(EditState *s)
{
    finder_close_open_popup();
    do_minibuffer_exit(s, 1);
}

static void finder_confirm(EditState *s)
{
    QEmacsState *qs = s->qe_state;
    char path[MAX_FILENAME_SIZE] = {0};

    if (finder_state.open_popup) {
        int off = list_get_offset(finder_state.open_popup);
        char linebuf[512];
        eb_get_strline(finder_state.open_popup->b, linebuf, sizeof(linebuf), &off);
        char *p = linebuf + 1;  /* skip list_mode marker */
        if (*p && *p != '(')   /* ignore "(indexing...)" / "(no matches)" */
            pstrcpy(path, sizeof(path), p);
    }

    finder_close_open_popup();
    do_minibuffer_exit(s, 1);  /* abort (callback is a no-op) */

    if (path[0]) {
        EditState *tes = finder_state.opener_es;
        if (tes && finder_window_alive(tes)) {
            qs->active_window = tes;
            do_load(tes, path);
            do_refresh_complete(tes);
        }
    }
}

/* -------------------------------------------------------------------------
 * M-o: finder-open
 * ------------------------------------------------------------------------- */

static void finder_noop_cb(void *opaque, char *path)
{
    (void)opaque;
    if (path) free(path);
}

static void do_finder_open(EditState *s)
{
    QEmacsState *qs = &qe_state;

    if (!finder_state.files_ready && !finder_state.files_pid)
        finder_start_file_scan();

    /* Find the right target window to load files into.
     * If M-o is invoked from a left panel (dired/bufed, flagged WF_RSEPARATOR),
     * use the window to the right — same pattern as dired_select. */
    EditState *target = s;
    if (s->flags & WF_RSEPARATOR) {
        EditState *right = find_window(s, KEY_RIGHT);
        if (right)
            target = right;
    }
    /* also skip any popup or minibuf that might somehow be active */
    if ((target->flags & WF_POPUP) || target->minibuf) {
        for (EditState *w = qs->first_window; w; w = w->next_window) {
            if (!(w->flags & (WF_POPUP | WF_RSEPARATOR)) && !w->minibuf) {
                target = w;
                break;
            }
        }
    }
    finder_state.opener_es = target;
    finder_state.last_input[0] = '\0';

    /* create the results popup, positioned above the minibuffer */
    int scr_w = qs->screen->width;
    int scr_h = qs->screen->height - qs->status_height;
    int ph    = scr_h * 3 / 4;
    int pw    = scr_w;
    int py    = scr_h - ph;  /* bottom portion, above minibuffer */

    EditBuffer *pb = eb_find("*finder-files*");
    if (!pb) {
        pb = eb_new("*finder-files*", BF_SYSTEM);
    } else {
        pb->flags &= ~BF_READONLY;
        eb_delete(pb, 0, pb->total_size);
    }
    EditState *popup = edit_new(pb, 0, py, pw, ph, WF_POPUP);
    do_set_mode(popup, &list_mode, NULL);
    popup->force_highlight = 1;
    popup->wrap = WRAP_TRUNCATE;
    finder_state.open_popup = popup;

    /* open minibuffer and patch to live-filter mode */
    minibuffer_edit("", "Find: ", &finder_hist, NULL, finder_noop_cb, s);
    if (qs->active_window && qs->active_window->minibuf)
        qs->active_window->mode = &finder_minibuf_mode;

    /* initial population */
    finder_populate_popup("");
    edit_display(qs);
    dpy_flush(qs->screen);
}

/* -------------------------------------------------------------------------
 * Async grep (rg --vimgrep / grep -rn fallback)
 * ------------------------------------------------------------------------- */

static void finder_close_grep_popup(void)
{
    if (!finder_state.grep_popup) return;
    EditBuffer *pb = finder_state.grep_popup->b;
    edit_close(finder_state.grep_popup);
    eb_free(pb);
    finder_state.grep_popup = NULL;
}

static void finder_grep_exit(void *opaque, int status)
{
    (void)opaque; (void)status;
    QEmacsState *qs = &qe_state;
    set_read_handler(finder_state.grep_fd, NULL, NULL);
    close(finder_state.grep_fd);
    finder_state.grep_fd  = -1;
    finder_state.grep_pid = 0;

    if (!finder_state.grep_popup) {
        if (qs->active_window)
            put_status(qs->active_window, "Grep: 0 matches");
        return;
    }

    EditBuffer *b = finder_state.grep_popup->b;
    b->flags |= BF_READONLY;
    finder_state.grep_popup->offset = 0;
    finder_state.grep_popup->force_highlight = 1;

    put_status(finder_state.grep_popup, "Grep: %d match%s — Enter to open, Esc to close",
               finder_state.grep_count,
               finder_state.grep_count == 1 ? "" : "es");

    edit_display(qs);
    dpy_flush(qs->screen);
}

static void finder_grep_handler(void *opaque)
{
    (void)opaque;
    QEmacsState *qs = &qe_state;
    int space = FINDER_READ_BUF - finder_state.grep_len - 1;
    if (space <= 0) return;
    int n = read(finder_state.grep_fd,
                 finder_state.grep_buf + finder_state.grep_len, space);
    if (n <= 0) return;
    finder_state.grep_len += n;
    finder_state.grep_buf[finder_state.grep_len] = '\0';

    /* create popup on first data */
    if (!finder_state.grep_popup) {
        int scr_w = qs->screen->width;
        int scr_h = qs->screen->height - qs->status_height;
        int pw    = scr_w * 4 / 5;
        int ph    = scr_h * 3 / 4;
        int px    = (scr_w - pw) / 2;
        int py    = (scr_h - ph) / 2;
        EditBuffer *gb = eb_find("*grep-results*");
        if (!gb) {
            gb = eb_new("*grep-results*", BF_SYSTEM);
        } else {
            gb->flags &= ~BF_READONLY;
            eb_delete(gb, 0, gb->total_size);
        }
        EditState *e = edit_new(gb, px, py, pw, ph, WF_POPUP);
        do_set_mode(e, &grep_results_mode, NULL);
        e->wrap  = WRAP_TRUNCATE;
        e->force_highlight = 1;
        finder_state.grep_popup = e;
        qs->active_window = e;
    }

    EditBuffer *b = finder_state.grep_popup->b;
    char *buf = finder_state.grep_buf;
    char *end = finder_state.grep_buf + finder_state.grep_len;
    char *nl;
    while ((nl = (char *)memchr(buf, '\n', end - buf)) != NULL) {
        *nl = '\0';
        if (buf[0]) {
            eb_printf(b, " %s\n", buf);
            finder_state.grep_count++;
        }
        buf = nl + 1;
    }
    int remaining = (int)(end - buf);
    if (remaining > 0)
        memmove(finder_state.grep_buf, buf, remaining);
    finder_state.grep_len = remaining;
}

static void finder_grep_cb(void *opaque, char *pattern)
{
    if (!pattern || !pattern[0]) { if (pattern) free(pattern); return; }

    EditState *tes = (EditState *)opaque;
    finder_state.grep_target_es = tes;
    finder_state.grep_count = 0;
    finder_state.grep_len   = 0;

    if (finder_state.grep_popup)
        finder_close_grep_popup();

    int pfd[2];
    if (pipe(pfd) < 0) { free(pattern); return; }

    int pid = fork();
    if (pid < 0) { close(pfd[0]); close(pfd[1]); free(pattern); return; }
    if (pid == 0) {
        dup2(pfd[1], STDOUT_FILENO);
        close(pfd[0]); close(pfd[1]);
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) { dup2(devnull, STDERR_FILENO); close(devnull); }
        execlp("rg", "rg", "--vimgrep", pattern, ".", NULL);
        execlp("grep", "grep", "-rn", "--include=*", pattern, ".", NULL);
        exit(1);
    }
    close(pfd[1]);
    finder_state.grep_fd  = pfd[0];
    finder_state.grep_pid = pid;
    set_read_handler(pfd[0], finder_grep_handler, NULL);
    set_pid_handler(pid, finder_grep_exit, NULL);
    free(pattern);
}

/* -------------------------------------------------------------------------
 * M-s: finder-grep
 * ------------------------------------------------------------------------- */

static void do_finder_grep(EditState *s)
{
    QEmacsState *qs = &qe_state;
    EditState *target = s;
    if (s->flags & WF_RSEPARATOR) {
        EditState *right = find_window(s, KEY_RIGHT);
        if (right) target = right;
    }
    if ((target->flags & WF_POPUP) || target->minibuf) {
        for (EditState *w = qs->first_window; w; w = w->next_window) {
            if (!(w->flags & (WF_POPUP | WF_RSEPARATOR)) && !w->minibuf) {
                target = w;
                break;
            }
        }
    }
    minibuffer_edit("", "Grep: ", &grep_hist, NULL, finder_grep_cb, target);
}

/* -------------------------------------------------------------------------
 * Grep results mode: Ctrl+N/P navigate, Enter open, Esc close
 * ------------------------------------------------------------------------- */

static void grep_next(EditState *s)  { do_up_down(s,  1); }
static void grep_prev(EditState *s)  { do_up_down(s, -1); }

static void grep_close(EditState *s)
{
    QEmacsState *qs = s->qe_state;
    EditBuffer  *pb = s->b;
    EditState   *restore = finder_state.grep_target_es;
    edit_close(s);
    eb_free(pb);
    finder_state.grep_popup = NULL;
    if (restore && finder_window_alive(restore))
        qs->active_window = restore;
    do_refresh(qs->active_window);
}

static void grep_open(EditState *s)
{
    QEmacsState *qs = s->qe_state;

    /* read the selected line from the grep results buffer */
    int off = list_get_offset(s);
    char linebuf[1024];
    eb_get_strline(s->b, linebuf, sizeof(linebuf), &off);
    char *line = linebuf + 1;  /* skip list_mode marker */

    /* parse "filepath:linenum[:col:content]" */
    char *colon = strchr(line, ':');
    if (!colon) { grep_close(s); return; }

    char filepath[MAX_FILENAME_SIZE];
    int  plen = (int)(colon - line);
    if (plen <= 0 || plen >= (int)sizeof(filepath)) { grep_close(s); return; }
    memcpy(filepath, line, plen);
    filepath[plen] = '\0';

    int linenum = atoi(colon + 1);
    if (linenum < 1) linenum = 1;

    EditState *tes = finder_state.grep_target_es;
    grep_close(s);  /* closes popup, restores active_window */

    if (tes && finder_window_alive(tes)) {
        qs->active_window = tes;
        do_load(tes, filepath);
        do_goto_line(tes, linenum);
        do_refresh_complete(tes);
    }
}

/* -------------------------------------------------------------------------
 * Command tables
 * ------------------------------------------------------------------------- */

static CmdDef finder_minibuf_commands[] = {
    CMD0(KEY_CTRL('n'), KEY_DOWN,      "finder-next",    finder_next)
    CMD0(KEY_CTRL('p'), KEY_UP,        "finder-prev",    finder_prev)
    CMD0(KEY_RET,       KEY_NONE,      "finder-confirm", finder_confirm)
    CMD0(KEY_ESC,       KEY_CTRL('g'), "finder-abort",   finder_abort)
    CMD_DEF_END,
};

static CmdDef grep_results_commands[] = {
    CMD0(KEY_CTRL('n'), KEY_DOWN,      "grep-next",  grep_next)
    CMD0(KEY_CTRL('p'), KEY_UP,        "grep-prev",  grep_prev)
    CMD0(KEY_RET,       KEY_NONE,      "grep-open",  grep_open)
    CMD0(KEY_ESC,       KEY_CTRL('g'), "grep-close", grep_close)
    CMD_DEF_END,
};

static CmdDef finder_commands[] = {
    CMD0(KEY_META('o'), KEY_NONE, "finder-open", do_finder_open)
    CMD0(KEY_META('s'), KEY_NONE, "finder-grep", do_finder_grep)
    CMD_DEF_END,
};

/* -------------------------------------------------------------------------
 * Init
 * ------------------------------------------------------------------------- */

int finder_init(void)
{
    memset(&finder_state, 0, sizeof(finder_state));
    finder_state.files_fd = -1;
    finder_state.grep_fd  = -1;

    /* finder-minibuf: text_mode + display_hook for live filtering */
    memcpy(&finder_minibuf_mode, &text_mode, sizeof(ModeDef));
    finder_minibuf_mode.name         = "finder-minibuf";
    finder_minibuf_mode.mode_flags   = MODEF_NOCMD;
    finder_minibuf_mode.display_hook = finder_display_hook;
    qe_register_mode(&finder_minibuf_mode);
    qe_register_cmd_table(finder_minibuf_commands, "finder-minibuf");

    /* grep-results: list_mode + open-on-Enter */
    memcpy(&grep_results_mode, &list_mode, sizeof(ModeDef));
    grep_results_mode.name       = "grep-results";
    grep_results_mode.mode_flags = MODEF_NOCMD;
    qe_register_mode(&grep_results_mode);
    qe_register_cmd_table(grep_results_commands, "grep-results");

    /* global commands */
    qe_register_cmd_table(finder_commands, NULL);
    return 0;
}

