//static int shell_get_colorized_line(EditState *e, unsigned int *buf, int buf_size, int offset, int line_num);

/* move to mode */
//static int shell_launched = 0;

//static int shell_mode_init(EditState *s, ModeSavedData *saved_data);

/* allocate one pty/tty pair */
//static int get_pty(char *tty_str);

//static int run_process(const char *path, const char **argv, int *fd_ptr, int *pid_ptr);

//static void tty_init(ShellState *s);

//static void tty_write(ShellState *s, const char *buf, int len);

/* compute offset the char at 'x' and 'y'. Can insert spaces or lines
   if needed */
/* XXX: optimize !!!!! */
//static void tty_gotoxy(ShellState *s, int x, int y, int relative);

typedef struct ShellState ShellState;

void tty_csi_m(ShellState *s, int c);

/* Well, almost a hack to update cursor */
//static void tty_update_cursor(ShellState *s);

/* CG: much cleaner way! */
/* would need a kill hook as well ? */
//static void shell_display_hook(EditState *e);

void shell_key(void *opaque, int key);

//static void tty_emulate(ShellState *s, int c);

/* modify the color according to the current one (may be incorrect if
   we are editing because we should write default color) */
//static void shell_color_callback(EditBuffer *b, void *opaque, enum LogOperation op, int offset, int size);

//static int shell_get_colorized_line(EditState *e, unsigned int *buf, int buf_size, int offset, int line_num);

/* buffer related functions */

/* called when characters are available on the tty */
//static void shell_read_cb(void *opaque);

void shell_pid_cb(void *opaque, int status);

//static void shell_close(EditBuffer *b);

EditBuffer *new_shell_buffer(const char *name, const char *path, const char **argv, int is_shell);

//static void do_shell(EditState *s, int force);

void shell_move_left_right(EditState *e, int dir);

void shell_move_word_left_right(EditState *e, int dir);

void shell_move_up_down(EditState *e, int dir);

void shell_scroll_up_down(EditState *e, int dir);

void shell_move_bol(EditState *e);

void shell_move_eol(EditState *e);

void shell_write_char(EditState *e, int c);

void do_shell_toggle_input(EditState *e);

/* CG: these variables should move to mode structure */
//static int error_offset = -1;
//static int last_line_num = -1;
//static char last_filename[MAX_FILENAME_SIZE];

//static void do_compile(EditState *e, const char *cmd);

//static void do_compile_error(EditState *s, int dir);

int shell_init(void);

