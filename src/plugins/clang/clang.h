//static int get_c_keyword(char *buf, int buf_size, unsigned int **pp);
void c_colorize_line(unsigned int *buf, int len, 
                     int *colorize_state_ptr, int state_only);
//static int find_indent1(EditState *s, unsigned int *buf);
//static int find_pos(EditState *s, unsigned int *buf, int size);
//static void insert_spaces(EditState *s, int *offset_ptr, int i);
//static void do_c_indent(EditState *s);

void do_c_indent(EditState *s);

void do_c_indent_region(EditState *s);
void do_c_electric(EditState *s, int key);
//static int c_mode_probe(ModeProbeData *p);
int c_mode_init(EditState *s, ModeSavedData *saved_data);
int c_init(void);
