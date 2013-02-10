
//static void tty_resize(int sig);
//static void term_exit(void);
//static void tty_read_handler(void *opaque);

//static int term_probe(void);

//static int term_init(QEditScreen *s, int w, int h);

//static void term_close(QEditScreen *s);

//static void term_exit(void);

//static void tty_resize(int sig);

//static void term_invalidate(void);

//static void term_cursor_at(QEditScreen *s, int x1, int y1, int w, int h);

//static int term_is_user_input_pending(QEditScreen *s);

//static void tty_read_handler(void *opaque);

//static inline int color_dist(unsigned int c1, unsigned c2);

//static int get_tty_color(QEColor color);

//static void term_fill_rectangle(QEditScreen *s, int x1, int y1, int w, int h, QEColor color);

/* XXX: could alloc font in wrapper */
//static QEFont *term_open_font(QEditScreen *s, int style, int size);

//static void term_close_font(QEditScreen *s, QEFont *font);

/*
 * Modified implementation of wcwidth() from Markus Kuhn. We do not
 * handle non spacing and enclosing combining characters and control
 * chars.  
 */
//static int term_glyph_width(QEditScreen *s, unsigned int ucs);

//static void term_text_metrics(QEditScreen *s, QEFont *font, QECharMetrics *metrics, const unsigned int *str, int len);

//static void term_draw_text(QEditScreen *s, QEFont *font, int x, int y, const unsigned int *str, int len, QEColor color);

//static void term_set_clip(QEditScreen *s, int x, int y, int w, int h);

//static void term_flush(QEditScreen *s);
 
int tty_init(void);

