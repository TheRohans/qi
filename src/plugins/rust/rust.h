#ifndef QI_PLUG_RUST_H
#define QI_PLUG_RUST_H

void rust_colorize_line(unsigned int *buf, int len, int *colorize_state_ptr,
                     int state_only);

void do_rust_comment(EditState *s);
void do_rust_comment_region(EditState *s);
void do_rust_electric(EditState *s, int key);

int rust_mode_init(EditState *s, ModeSavedData *saved_data);
int rust_init(void);

#endif
