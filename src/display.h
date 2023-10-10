#ifndef QE_DISPLAY_H
#define QE_DISPLAY_H

#define MAX_SCREEN_WIDTH 1024  /* in chars */
#define MAX_SCREEN_LINES 256  /* in text lines */

#include "color.h"

/* XXX: use different name prefix to avoid conflict */
#define QE_STYLE_NORM         0x0001
#define QE_STYLE_BOLD         0x0002
#define QE_STYLE_ITALIC       0x0004
#define QE_STYLE_UNDERLINE    0x0008
#define QE_STYLE_LINE_THROUGH 0x0010
#define QE_STYLE_MASK         0x00ff

#define NB_FONT_FAMILIES 3
#define QE_FAMILY_SHIFT       8
#define QE_FAMILY_MASK        0xff00
#define QE_FAMILY_FIXED       0x0100
#define QE_FAMILY_SERIF       0x0200
#define QE_FAMILY_SANS        0x0300 /* sans serif */

/* fallback font handling */
#define QE_FAMILY_FALLBACK_SHIFT  16
#define QE_FAMILY_FALLBACK_MASK   0xff0000

enum TextStyle {
  TS_RESET = 0,     // 0  Reset (default)
  TS_BRIGHT = 1,    // 1  Bright (bold)
  TS_DIM = 2,       // 2  Dim (faint)
  TS_ITALIC = 3,    // 3  *Italic
  TS_UNDERSCORE = 4,// 4  Underscore (underline)
  TS_BLINK_S = 5,   // 5  Blink (slow)
  TS_BLINK_F = 6,   // 6  *Blink (fast)
  TS_REVERSE = 7,   // 7  Reverse (negative)
  TS_HIDDEN = 8,    // 8  Hidden (conceal)
  TS_STRIKE = 9,    // 9  *Strike Through
  TS_NONE = 100
};

typedef struct QEFont {
    int refcount;
    int ascent;
    int descent;
    void *private;
    int system_font; /* TRUE if system font */
    /* cache data */
    int style;
    int size;
    int timestamp;
} QEFont;

typedef struct QECharMetrics {
    int font_ascent;    /* maximum font->ascent */
    int font_descent;   /* maximum font->descent */
    int width;          /* sum of glyph widths */
} QECharMetrics;


struct QEditScreen;
typedef struct QEditScreen QEditScreen;

typedef struct QEDisplay {
    const char *name;
    int (*dpy_probe)(void);
    int (*dpy_init)(QEditScreen *s, int w, int h);
    void (*dpy_close)(QEditScreen *s);
    void (*dpy_cursor_at)(QEditScreen *s, int x1, int y1, int w, int h);
    void (*dpy_flush)(QEditScreen *s);
    int (*dpy_is_user_input_pending)(QEditScreen *s);
    void (*dpy_fill_rectangle)(QEditScreen *s,
                               int x, int y, int w, int h, QEColor color);
    QEFont *(*dpy_open_font)(QEditScreen *s, int style, int size);
    void (*dpy_close_font)(QEditScreen *s, QEFont *font);
    void (*dpy_text_metrics)(QEditScreen *s, QEFont *font, 
                             QECharMetrics *metrics,
                             const unsigned int *str, int len);
    void (*dpy_draw_text)(QEditScreen *s, QEFont *font,
                          int x, int y, const unsigned int *str, int len,
                          QEColor color, enum TextStyle tstyle);
    void (*dpy_set_clip)(QEditScreen *s,
                         int x, int y, int w, int h);
    void (*dpy_selection_activate)(QEditScreen *s);
    void (*dpy_selection_request)(QEditScreen *s);
    
    void (*dpy_invalidate)(void);
    /* full screen support */
    void (*dpy_full_screen)(QEditScreen *s, int full_screen);
    struct QEDisplay *next;
} QEDisplay;

struct QEditScreen {
    struct QEDisplay dpy;
    int width, height;
    int media;          /* media type (see CSS_MEDIA_xxx) */
    int bitmap_format;  /* supported bitmap format */
    int video_format;   /* supported video format */
                        /* clip region handling */
    int clip_x1, clip_y1;
    int clip_x2, clip_y2;
    void *private;
};

static inline void draw_text(QEditScreen *s, QEFont *font,
                             int x, int y, const unsigned int *str, int len,
                             QEColor color, enum TextStyle tstyle)
{
    s->dpy.dpy_draw_text(s, font, x, y, str, len, color, tstyle);
}

static inline QEFont *open_font(QEditScreen *s, 
                                int style, int size)
{
    return s->dpy.dpy_open_font(s, style, size);
}

static inline void close_font(QEditScreen *s, QEFont *font)
{
    if (!font->system_font)
        s->dpy.dpy_close_font(s, font);
}

static inline void text_metrics(QEditScreen *s, QEFont *font, 
                                QECharMetrics *metrics,
                                const unsigned int *str, int len)
{
    s->dpy.dpy_text_metrics(s, font, metrics, str, len);
}

/* XXX: only needed for backward compatibility */
static inline int glyph_width(QEditScreen *s, QEFont *font, int ch)
{
    unsigned int buf[1];
    QECharMetrics metrics;
    buf[0] = ch;
    text_metrics(s, font, &metrics, buf, 1);
    return metrics.width;
}

/**
 * Flush the display
 */
static inline void dpy_flush(QEditScreen *s)
{
    s->dpy.dpy_flush(s);
}

static inline void dpy_close(QEditScreen *s)
{
    s->dpy.dpy_close(s);
}

void fill_rectangle(QEditScreen *s,
                    int x1, int y1, int w, int h, QEColor color);
void set_clip_rectangle(QEditScreen *s, CSSRect *r);
void push_clip_rectangle(QEditScreen *s, CSSRect *or, CSSRect *r);

int qe_register_display(QEDisplay *dpy);
QEDisplay *probe_display(void);
QEFont *select_font(QEditScreen *s, int style, int size);

static inline QEFont *lock_font(QEditScreen *s, QEFont *font) {
    if (font && font->refcount)
        font->refcount++;
    return font;
}
static inline void release_font(QEditScreen *s, QEFont *font) {
    if (font && font->refcount)
        font->refcount--;
}

void selection_activate(QEditScreen *s);
void selection_request(QEditScreen *s);

#endif
