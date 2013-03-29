
void python_colorize_line(unsigned int *buf, int len, 
                     int *colorize_state_ptr, int state_only);
void do_python_indent(EditState *s);

void do_python_indent_region(EditState *s);
void do_python_electric(EditState *s, int key);

int python_mode_init(EditState *s, ModeSavedData *saved_data);
int python_init(void);
