void php_colorize_line(unsigned int *buf, int len, 
                     int *colorize_state_ptr, int state_only);
//void do_c_indent_region(EditState *s);
//void do_c_electric(EditState *s, int key);

int php_mode_init(EditState *s, ModeSavedData *saved_data);
int php_init(void);
