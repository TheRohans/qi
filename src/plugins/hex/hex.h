#ifndef QI_PLUG_HEX_H
#define QI_PLUG_HEX_H

void do_goto_byte(EditState *s, int offset);

void do_set_width(EditState *s, int w);

void do_incr_width(EditState *s, int incr);

void do_toggle_hex(EditState *s);

int detect_binary(const unsigned char *buf, int size);

void hex_move_bol(EditState *s);

void hex_move_eol(EditState *s);

void hex_move_left_right(EditState *s, int dir);

void hex_move_up_down(EditState *s, int dir);

void hex_write_char(EditState *s, int key);

void hex_mode_line(EditState *s, char *buf, int buf_size);

int hex_init(void);

#endif
