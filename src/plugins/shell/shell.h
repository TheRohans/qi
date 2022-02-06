#ifndef QI_PLUG_SHELL_H
#define QI_PLUG_SHELL_H

typedef struct ShellState ShellState;

void tty_csi_m(ShellState *s, int c);

void shell_key(void *opaque, int key);

void shell_pid_cb(void *opaque, int status);

EditBuffer *new_shell_buffer(const char *name, const char *path, const char **argv, int is_shell);

void shell_move_left_right(EditState *e, int dir);

void shell_move_word_left_right(EditState *e, int dir);

void shell_move_up_down(EditState *e, int dir);

void shell_scroll_up_down(EditState *e, int dir);

void shell_move_bol(EditState *e);

void shell_move_eol(EditState *e);

void shell_write_char(EditState *e, int c);

void do_shell_toggle_input(EditState *e);

int shell_init(void);

#endif

