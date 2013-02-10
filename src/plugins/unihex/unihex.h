//static int unihex_mode_init(EditState *s, ModeSavedData *saved_data);

//static int unihex_backward_offset(EditState *s, int offset);

//static int unihex_display(EditState *s, DisplayState *ds, int offset);

void unihex_move_bol(EditState *s);

void unihex_move_eol(EditState *s);

void unihex_move_left_right(EditState *s, int dir);

void unihex_move_up_down(EditState *s, int dir);

int unihex_init(void);

