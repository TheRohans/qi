//static void dired_view_file(EditState *s, const char *filename);

extern inline int dired_get_index(EditState *s);

void dired_free(EditState *s);

/* sort alphabetically with directories first */
//static int dired_sort_func(const void *p1, const void *p2);

/* select current item */
//static void do_dired_sort(EditState *s);

//static void dired_mark(EditState *s, int mark);

//static void dired_sort(EditState *s, const char *sort_order);

void build_dired_list(EditState *s, const char *path);


//static char *get_dired_filename(EditState *s, char *buf, int buf_size, int index);

/* select current item */
//static void dired_select(EditState *s);

//static void dired_view_file(EditState *s, const char *filename);

//static void dired_execute(EditState *s);

//static void dired_parent(EditState *s);

//static void dired_refresh(EditState *s);

//static void dired_display_hook(EditState *s);

//static int dired_mode_init(EditState *s, ModeSavedData *saved_data);

//static void dired_mode_close(EditState *s);

/* can only apply dired mode on directories */
//static int dired_mode_probe(ModeProbeData *p);

/* open dired window on the left. The directory of the current file is
   used */
void do_dired(EditState *s);

int dired_init(void);

