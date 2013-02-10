
//static int list_get_colorized_line(EditState *s, unsigned int *buf, int buf_size, int offset, int line_num)

/* get current position (index) in list */
int list_get_pos(EditState *s);

/* get current offset of the line in list */
int list_get_offset(EditState *s);

void list_toggle_selection(EditState *s);

//static int list_mode_init(EditState *s, ModeSavedData *saved_data);

//static void list_mode_close(EditState *s)

int list_init(void);

