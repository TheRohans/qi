#ifndef QE_H
#define QE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include <signal.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h> // for wait
#include <sys/wait.h>  // for wait
    
#include <errno.h>
#include <inttypes.h>

#include "unicode.h"
#include "config.h"

#include "cutils.h"
#include "log.h"

/////////////////////////////////////////
// low level I/O events 
void set_read_handler(int fd, void (*cb)(void *opaque), void *opaque);
void set_write_handler(int fd, void (*cb)(void *opaque), void *opaque);
int set_pid_handler(int pid, 
                    void (*cb)(void *opaque, int status), void *opaque);
void url_exit(void);
void register_bottom_half(void (*cb)(void *opaque), void *opaque);
void unregister_bottom_half(void (*cb)(void *opaque), void *opaque);

struct QETimer;
typedef struct QETimer QETimer;
QETimer *qe_add_timer(int delay, void *opaque, void (*cb)(void *opaque));
void qe_kill_timer(QETimer *ti);

/* main loop for Unix programs using liburlio */
void url_main_loop(void (*init)(void *opaque), void *opaque);

typedef unsigned char u8;
struct EditState;

#define MAXINT 0x7fffffff
#define MAX_FILENAME_SIZE 1024
#define NO_ARG MAXINT

/* ///////////////////////////////////////////////////////////////////////// */
/* util.c */

/* media definitions */
#define CSS_MEDIA_TTY     0x0001
#define CSS_MEDIA_ALL     0xffff

#define INT2VOIDP(i) (void*)(uintptr_t)(i)

typedef struct CSSRect {
    int x1, y1, x2, y2;
} CSSRect;

struct FindFileState;
typedef struct FindFileState FindFileState;

FindFileState *find_file_open(const char *path, const char *pattern);
int find_file_next(FindFileState *s, char *filename, int filename_size_max);
void find_file_close(FindFileState *s);
void canonize_path(char *buf, int buf_size, const char *path);
void canonize_absolute_path(char *buf, int buf_size, const char *path1);
const char *basename(const char *filename);
const char *extension(const char *filename);
char *makepath(char *buf, int buf_size, const char *path, const char *filename);
void splitpath(char *dirname, int dirname_size,
               char *filename, int filename_size, const char *pathname);

int find_resource_file(char *path, int path_size, const char *pattern);
int strfind(const char *keytable, const char *str, int casefold);
void umemmove(unsigned int *dest, unsigned int *src, int len);
void skip_spaces(const char **pp);
int ustristart(const unsigned int *str, const char *val, const unsigned int **ptr);
void css_strtolower(char *buf, int buf_size);

void get_str(const char **pp, char *buf, int buf_size, const char *stop);
int compose_keys(unsigned int *keys, int *nb_keys);
int strtokey(const char **pp);
int strtokeys(const char *keystr, unsigned int *keys, int max_keys);
void keytostr(char *buf, int buf_size, int key);
int css_define_color(const char *name, const char *value);
int css_get_color(unsigned int *color_ptr, const char *p);
int css_get_font_family(const char *str);
void css_union_rect(CSSRect *a, CSSRect *b);
static inline int css_is_null_rect(CSSRect *a)
{
    return (a->x2 <= a->x1 ||
            a->y2 <= a->y1);
}
static inline void css_set_rect(CSSRect *a, int x1, int y1, int x2, int y2)
{
    a->x1 = x1;
    a->y1 = y1;
    a->x2 = x2;
    a->y2 = y2;
}
/* return true if a and b intersect */
static inline int css_is_inter_rect(CSSRect *a, CSSRect *b)
{
    return (!(a->x2 <= b->x1 ||
              a->x1 >= b->x2 ||
              a->y2 <= b->y1 ||
              a->y1 >= b->y2));
}

static inline int css_is_space(int ch)
{
    return (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t');
}

int css_get_enum(const char *str, const char *enum_str);

int get_clock_ms(void);

typedef int (CSSAbortFunc)(void *);

static inline int max(int a, int b)
{
    if (a > b)
        return a;
    else
        return b;
}

static inline int min(int a, int b)
{
    if (a < b)
        return a;
    else
        return b;
}

/* string arrays */

typedef struct StringItem {
    void *opaque;  //!< opaque data that the user can use
    char selected; //!< true if selected 
    char str[1];
} StringItem;

typedef struct StringArray {
    int nb_allocated;
    int nb_items;
    StringItem **items;
} StringArray;

StringItem *set_string(StringArray *cs, int index, const char *str);
StringItem *add_string(StringArray *cs, const char *str);
void free_strings(StringArray *cs);

// command line option
#define CMD_OPT_ARG      0x0001 //!< argument
#define CMD_OPT_STRING   0x0002 //!< string
#define CMD_OPT_BOOL     0x0004 //!< boolean 
#define CMD_OPT_INT      0x0008 //!< int

typedef struct CmdOptionDef {
    const char *name;
    const char *shortname;
    const char *argname;
    int flags;
    const char *help;
    union {
        const char **string_ptr;
        int *int_ptr;
        void (*func_noarg)();
        void (*func_arg)(const char *);
        struct CmdOptionDef *next;
    } u;
} CmdOptionDef;

void qe_register_cmd_line_options(CmdOptionDef *table);

/**
 * qe event handling 
 */
enum QEEventType {
    QE_KEY_EVENT,
    QE_EXPOSE_EVENT,          //!< full redraw
    QE_UPDATE_EVENT,          //!< update content
    QE_BUTTON_PRESS_EVENT,    //!< mouse button press event 
    QE_BUTTON_RELEASE_EVENT,  //!< mouse button release event
    QE_MOTION_EVENT,          //!< mouse motion event
    QE_SELECTION_CLEAR_EVENT, //!< request selection clear (X11 type selection)
};

#define KEY_CTRL(c)     ((c) & 0x001f)
#define KEY_META(c)     ((c) | 0xe000)
#define KEY_ESC1(c)     ((c) | 0xe100)
#define KEY_CTRLX(c)    ((c) | 0xe200)
#define KEY_CTRLXRET(c) ((c) | 0xe300)
#define KEY_CTRLH(c)    ((c) | 0xe500)
#define KEY_SPECIAL(c)  (((c) >= 0xe000 && (c) < 0xf000) || ((c) >= 0 && (c) < 32))

#define KEY_NONE        0xffff
#define KEY_DEFAULT     0xe401          //!< to handle all non special keys 

#define KEY_TAB         KEY_CTRL('i')
#define KEY_RET         KEY_CTRL('m')
#define KEY_ESC         KEY_CTRL('[')
#define KEY_SPC         0x0020
#define KEY_DEL         127             //!< kbs
#define KEY_BS          KEY_CTRL('h')   //!< kbs

#define KEY_UP          KEY_ESC1('A')   //!< kcuu1
#define KEY_DOWN        KEY_ESC1('B')   //!< kcud1
#define KEY_RIGHT       KEY_ESC1('C')   //!< kcuf1
#define KEY_LEFT        KEY_ESC1('D')   //!< kcub1

// TODO: these don't work for some reason
#define KEY_CTRL_UP     KEY_ESC1('a')   // 5A
#define KEY_CTRL_DOWN   KEY_ESC1('b')   // 5B
#define KEY_CTRL_RIGHT  KEY_ESC1('c')   // 5C
#define KEY_CTRL_LEFT   KEY_ESC1('d')   // 5D
#define KEY_CTRL_END    KEY_ESC1('f')   // 5F
#define KEY_CTRL_HOME   KEY_ESC1('h')   // 5H

#define KEY_CTRL_PAGEUP KEY_ESC1('i')
#define KEY_CTRL_PAGEDOWN KEY_ESC1('j')
#define KEY_SHIFT_TAB   KEY_ESC1('Z')   //!< kcbt
#define KEY_HOME        KEY_ESC1(1)     //!< khome
#define KEY_INSERT      KEY_ESC1(2)     //!< kich1
#define KEY_DELETE      KEY_ESC1(3)     //!< kdch1
#define KEY_END         KEY_ESC1(4)     //!< kend
#define KEY_PAGEUP      KEY_ESC1(5)     //!< kpp
#define KEY_PAGEDOWN    KEY_ESC1(6)     //!< knp
#define KEY_F1          KEY_ESC1(11)
#define KEY_F2          KEY_ESC1(12)
#define KEY_F3          KEY_ESC1(13)
#define KEY_F4          KEY_ESC1(14)
#define KEY_F5          KEY_ESC1(15)
#define KEY_F6          KEY_ESC1(17)
#define KEY_F7          KEY_ESC1(18)
#define KEY_F8          KEY_ESC1(19)
#define KEY_F9          KEY_ESC1(20)
#define KEY_F10         KEY_ESC1(21)
#define KEY_F11         KEY_ESC1(23)
#define KEY_F12         KEY_ESC1(24)
#define KEY_F13         KEY_ESC1(25)
#define KEY_F14         KEY_ESC1(26)
#define KEY_F15         KEY_ESC1(28)
#define KEY_F16         KEY_ESC1(29)
#define KEY_F17         KEY_ESC1(31)
#define KEY_F18         KEY_ESC1(32)
#define KEY_F19         KEY_ESC1(33)
#define KEY_F20         KEY_ESC1(34)

typedef struct QEKeyEvent {
    enum QEEventType type;
    int key;
} QEKeyEvent;

typedef struct QEExposeEvent {
    enum QEEventType type;
    /* currently, no more info */
} QEExposeEvent;

#define QE_BUTTON_LEFT   0x0001
#define QE_BUTTON_MIDDLE 0x0002
#define QE_BUTTON_RIGHT  0x0004
#define QE_WHEEL_UP      0x0008
#define QE_WHEEL_DOWN    0x0010

/**
 * should probably go somewhere else, or in the config file
 * how many text lines to scroll when mouse wheel is used 
 */
#define WHEEL_SCROLL_STEP 4

typedef struct QEButtonEvent {
    enum QEEventType type;
    int x;
    int y;
    int button;
} QEButtonEvent;

typedef struct QEMotionEvent {
    enum QEEventType type;
    int x;
    int y;
} QEMotionEvent;

typedef union QEEvent {
    enum QEEventType type;
    QEKeyEvent key_event;
    QEExposeEvent expose_event;
    QEButtonEvent button_event;
    QEMotionEvent motion_event;
} QEEvent;

void qe_handle_event(QEEvent *ev);
void qe_grab_keys(void (*cb)(void *opaque, int key), void *opaque);
void qe_ungrab_keys(void);

/* ///////////////////////////////////////////////////////////////////////// */
/* buffer.c */

/**
 * begin to mmap files from this size 
 */
#define MIN_MMAP_SIZE (1024*1024)

#define MAX_PAGE_SIZE 4096
//#define MAX_PAGE_SIZE 16

#define NB_LOGS_MAX 50

#define PG_READ_ONLY    0x0001 //!< the page is read only
#define PG_VALID_POS    0x0002 //!< set if the nb_lines / col fields are up to date
#define PG_VALID_CHAR   0x0004 //!< nb_chars is valid
#define PG_VALID_COLORS 0x0008 //!< color state is valid 

typedef struct Page {
    int size; //!< data size
    u8 *data;
    int flags;
    // the following are needed to handle line / column computation 
    int nb_lines; //!< Number of '\n' in data 
    int col;      //!< Number of chars since the last '\n'
    // the following is needed for char offset computation 
    int nb_chars;
} Page;

#define DIR_LTR 0
#define DIR_RTL 1

typedef int DirType;

enum LogOperation {
    LOGOP_FREE = 0,
    LOGOP_WRITE,
    LOGOP_INSERT,
    LOGOP_DELETE
};

// defined for real later... 
struct EditBuffer;

/**
 * each buffer modification can be catched with this callback 
 */
typedef void (*EditBufferCallback)(struct EditBuffer *,
                                   void *opaque,
                                   enum LogOperation op,
                                   int offset,
                                   int size);

typedef struct EditBufferCallbackList {
    void *opaque;
    EditBufferCallback callback;
    struct EditBufferCallbackList *next;
} EditBufferCallbackList;

// buffer flags 
#define BF_SAVELOG   0x0001  //!< activate buffer logging (for undo)
#define BF_SYSTEM    0x0002  //!< buffer system, cannot be seen by the user
#define BF_READONLY  0x0004  //!< read only buffer
#define BF_PREVIEW   0x0008  //!< used in dired mode to mark previewed files 
#define BF_LOADING   0x0010  //!< buffer is being loaded 
#define BF_SAVING    0x0020  //!< buffer is being saved 
#define BF_DIRED     0x0100  //!< buffer is interactive dired 

#define BUF_SCRATCH "*scratch*" //!< scratch buffer system always creates
#define BUF_DEBUG   "*debug*"   //!< debug buffer, runtime logs

typedef struct EditBuffer {
    Page *page_table;
    int nb_pages;
    int mark;       //!< current mark (moved with text) 
    int total_size; //!< total size of the buffer 
    int modified;

    // page cache 
    Page *cur_page;
    int cur_offset;
    int file_handle; //!< if the file is kept open because it is mapped, its handle is there
    int flags;

    struct EditBufferDataType *data_type;    //!< buffer data type (default is raw) 
    void *data; //!< associated buffer data, used if data_type != raw_data

    // undo system 
    int save_log;    //!< if true, each buffer operation is logged 
    int log_new_index, log_current;
    struct EditBuffer *log_buffer;
    int nb_logs;

    EditBufferCallbackList *first_callback;    //!< modification callbacks
    
    struct BufferIOState *io_state; //!< asynchronous loading/saving support
    
    int probed; //!< used during loading
    
    void *priv_data; //!< buffer polling & private data
    void (*close)(struct EditBuffer *); //!< called when deleting the buffer

    // saved data from the last opened mode, needed to restore mode 
    // CG: should instead keep a pointer to last window using this
    // buffer, even if no longer on screen
    struct ModeSavedData *saved_data; 

    struct EditBuffer *next; //!< next editbuffer in qe_state buffer_loadffer list
    char name[256];     //!< buffer name 
    char filename[MAX_FILENAME_SIZE]; //!< file name
} EditBuffer;

struct ModeProbeData;

/**
 * high level buffer type handling
 */
typedef struct EditBufferDataType {
    const char *name; //!< name of buffer data type (text, image, ...)
    int (*buffer_load)(EditBuffer *b, FILE *f);
    int (*buffer_save)(EditBuffer *b, const char *filename);
    void (*buffer_close)(EditBuffer *b);
    struct EditBufferDataType *next;
} EditBufferDataType;

/** 
 * the log buffer is used for the undo operation 
 * header of log operation 
 */
typedef struct LogBuffer {
    u8 op;
    u8 was_modified;
    int offset;
    int size;
} LogBuffer;

extern EditBuffer *trace_buffer;

void eb_init(void);
int eb_read(EditBuffer *b, int offset, void *buf, int size);
void eb_write(EditBuffer *b, int offset, void *buf, int size);
void eb_insert_buffer(EditBuffer *dest, int dest_offset, 
                      EditBuffer *src, int src_offset, 
                      int size);
void eb_insert(EditBuffer *b, int offset, const void *buf, int size);
void eb_delete(EditBuffer *b, int offset, int size);
void log_reset(EditBuffer *b);
EditBuffer *eb_new(const char *name, int flags);
void eb_free(EditBuffer *b);
EditBuffer *eb_find(const char *name);
EditBuffer *eb_find_file(const char *filename);

int eb_nextc(EditBuffer *b, int offset, int *next_ptr);
int eb_prevc(EditBuffer *b, int offset, int *prev_ptr);
int eb_goto_pos(EditBuffer *b, int line1, int col1);
int eb_get_pos(EditBuffer *b, int *line_ptr, int *col_ptr, int offset);
int eb_goto_char(EditBuffer *b, int pos);
int eb_get_char_offset(EditBuffer *b, int offset);
void do_undo(struct EditState *s);
void do_left_right(struct EditState *s, int dir);

int raw_load_buffer1(EditBuffer *b, FILE *f, int offset);
int save_buffer(EditBuffer *b);
void set_buffer_name(EditBuffer *b, const char *name1);
void set_filename(EditBuffer *b, const char *filename);
int eb_add_callback(EditBuffer *b, EditBufferCallback cb,
                    void *opaque);
void eb_free_callback(EditBuffer *b, EditBufferCallback cb,
                      void *opaque);
void eb_offset_callback(EditBuffer *b,
                        void *opaque,
                        enum LogOperation op,
                        int offset,
                        int size);
void eb_printf(EditBuffer *b, const char *fmt, ...);
void eb_line_pad(EditBuffer *b, int n);

// TODO: These are not utf8
int eb_get_str(EditBuffer *b, char *buf, int buf_size);
int eb_get_substr(EditBuffer *b, char *buf, int offset_start, int buf_size);
int eb_get_line(EditBuffer *b, unsigned int *buf, int buf_size,
                int *offset_ptr);
int eb_get_strline(EditBuffer *b, char *buf, int buf_size,
                   int *offset_ptr);
// 
int eb_goto_bol(EditBuffer *b, int offset);
int eb_is_empty_line(EditBuffer *b, int offset);
int eb_next_line(EditBuffer *b, int offset);

void eb_register_data_type(EditBufferDataType *bdt);
EditBufferDataType *eb_probe_data_type(const char *filename, int mode,
                                       uint8_t *buf, int buf_size);
void eb_set_data_type(EditBuffer *b, EditBufferDataType *bdt);
void eb_invalidate_raw_data(EditBuffer *b);
void eb_refresh();

extern EditBufferDataType raw_data_type;

/* qe module handling */

#ifdef QE_MODULE

/* dynamic module case */

#define qe_module_init(fn) \
        int __qe_module_init(void) { return fn(); }

#define qe_module_exit(fn) \
        void __qe_module_exit(void) { fn(); }

#else /* QE_MODULE */

#if (defined(__GNUC__) || defined(__TINYC__)) && defined(CONFIG_INIT_CALLS)

/* make sure that the keyword is not disabled by glibc (TINYC case) */
#undef __attribute__ 

#if __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 3)
/* same method as the linux kernel... */
#define __init_call     __attribute__((unused, __section__ (".initcall.init")))
#define __exit_call     __attribute__((unused, __section__ (".exitcall.exit")))
#else
#undef __attribute_used__
#define __attribute_used__	__attribute__((__used__))
#define __init_call	__attribute_used__ __attribute__((__section__ (".initcall.init")))
#define __exit_call	__attribute_used__ __attribute__((__section__ (".exitcall.exit")))
#endif

#define qe_module_init(fn) \
        static int (*__initcall_##fn)(void) __init_call = fn

#define qe_module_exit(fn) \
        static void (*__exitcall_##fn)(void) __exit_call = fn
#else

#define __init_call
#define __exit_call

#define qe_module_init(fn) \
        int module_ ## fn (void) { return fn(); }

#define qe_module_exit(fn)

#endif

#endif /* QE_MODULE */

/* ///////////////////////////////////////////////////////////////////////// */
/* display.c */

#include "display.h"

/* ///////////////////////////////////////////////////////////////////////// */
/* qe.c */

/* colorize & transform a line, lower level then ColorizeFunc */
typedef int (*GetColorizedLineFunc)(struct EditState *s, 
                                    unsigned int *buf, int buf_size,
                                    int offset1, int line_num);

/* colorize a line : this function modifies buf to set the char
   styles. 'buf' is guaranted to have one more char after its len
   (it is either '\n' or '\0') */
typedef void (*ColorizeFunc)(unsigned int *buf, int len, 
                             int *colorize_state_ptr, int state_only);

/* contains all the information necessary to uniquely identify a line,
   to avoid displaying it */
typedef struct QELineShadow {
    short x_start;
    short y;
    short height;
    unsigned int crc;
} QELineShadow;

enum WrapType {
    WRAP_TRUNCATE = 0,
    WRAP_LINE,
    WRAP_WORD
};

#define DIR_LTR 0
#define DIR_RTL 1

typedef struct EditState {
	/* -- If you change the layout or size of any of these, see SAVED_DATA_SIZE -- */
    int offset;                   //!< 4 offset of the cursor
    int offset_top;               //!< 4 text display state
    int y_disp;                   //!< 4 virtual position of the displayed text
    int x_disp[2];                //!< 4 position for LTR and RTL text resp.
    int minibuf;                  //!< 4 true if single line editing 
    int disp_width;               //!< 4 width in hex or ascii mode
    int hex_mode;                 //!< 4 true if we are currently editing hexa
    int unihex_mode;              //!< 4 true if unihex editing (hex_mode must be true too) 
    int hex_nibble;               //!< 4 current hexa nibble 
    int insert;                   //!< 4
	                              // -- 40
    int bidir;                    //!< 4
    int cur_rtl;                  //!< 4 TRUE if the cursor on over RTL chars 
    enum WrapType wrap;           //!< 4
    int line_numbers;             //!< 4
    int tab_size;                 //!< 4
    int indent_size;              //!< 4
    int indent_tabs_mode;         //!< 4 if true, use tabs to indent
    int interactive;              //!< 4 true if interaction is done instead of editing (e.g. for shell mode or HTML)
    int force_highlight;          //!< 4 if true, force showing of cursor even if window not focused (list mode only)
    int mouse_force_highlight;    //!< 4 if true, mouse can force highlight (list mode only)
		                          // -- 40
    GetColorizedLineFunc get_colorized_line_func; //!< 8 low level colorization function
    ColorizeFunc colorize_func;                   //!< 8 colorization function
	                              // -- 16
    int default_style;            //!< 4 default text style
    int end_of_saved_data;        //!< 4
	                              // -- 8 ===> 104
	/* -- after this limit, the fields are not saved into the buffer -- */

    struct ModeDef *mode;         //!< mode specific info
    void *mode_data;              //!< mode private data
    EditBuffer *b;
    unsigned char *colorize_states;   //!< state before line n, one byte per line 
    int colorize_nb_lines;
    int colorize_nb_valid_lines;
    // maximum valid offset, MAXINT if not modified. Needed to invalide
    //   'colorize_states' 
    int colorize_max_valid_offset; 

    int busy;                     //!< true if editing cannot be done if the window (e.g. the parser HTML is parsing the buffer to produce the display
    int display_invalid;          //!< true if the display was invalidated. Full redraw should be done
    int borders_invalid;          //!< true if window borders should be redrawn 
    int show_selection;           //!< if true, the selection is displayed
    int width, height;	          //!< display area info 
    int ytop, xleft;
    int x1, y1, x2, y2;           //!< full window size, including borders
    int flags;                    //!< display flags
#define WF_POPUP      0x0001 //!< popup window (with borders)
#define WF_MODELINE   0x0002 //!< mode line must be displayed 
#define WF_RSEPARATOR 0x0004 //!< right window separator
    char *prompt;
    struct QEmacsState *qe_state;
    struct QEditScreen *screen;   //!< copy of qe_state->screen
    char modeline_shadow[MAX_SCREEN_WIDTH]; //!< display shadow to optimize redraw
    QELineShadow *line_shadow;              //!< per window shadow 
    int shadow_nb_lines;
    int compose_len;
    int compose_start_offset;
    unsigned int compose_buf[20];
    struct EditState *next_window;
} EditState;

// TODO: Need to sort out what this is doing. This is erroring:
// qe.h:773:5: error: variably modified `generic_data` at file scope [-Werror]
// 
// becuase this value is being used to create an array size at compile
// time, but this... is some kind of magic.
// I am pretty sure it's looking a the structure above, grabing a pointer to 
// version of it, and then seeing how many bytes it has up until the property
// 'end_of_saved_data'. This is pretty cool, but it's causing errors so I've 
// hardcoded it to the calculated value. If you move them or change data types
// this number will be wrong. Uncomment the next line and output the value to
// see the new answer.
// This is not as easy as it first seems, this does different things based on
// architecture. Need more input...
#define SAVED_DATA_SIZE ((int)(uintptr_t)&((EditState *)0)->end_of_saved_data)
// #define SAVED_DATA_SIZE 108

int to_hex(int key);

struct DisplayState;

typedef struct ModeProbeData {
    char *filename;
    unsigned char *buf;
    int buf_size;
    int mode;                           //!< unix mode
} ModeProbeData;

/**
 * private data saved by a mode so that it can be restored 
 * when the mode is started again on a buffer 
 */
typedef struct ModeSavedData {
    struct ModeDef *mode;               //!< the mode is saved there 
    char generic_data[SAVED_DATA_SIZE]; //!< generic text data
    int data_size;                      //!< mode specific saved data
    char data[1];
} ModeSavedData;

typedef struct ModeDef {
    const char *name;
    int instance_size; //!< size of malloced instance 
    int (*mode_probe)(ModeProbeData *); //!< return the percentage of confidence 
    int (*mode_init)(EditState *, ModeSavedData *);
    void (*mode_close)(EditState *);
    /* save the internal state of the mode so that it can be opened
       again in the same state */
    ModeSavedData *(*mode_save_data)(EditState *s);

    /* low level display functions (must be NULL to use text relatedx
       functions)*/
    void (*display_hook)(EditState *);
    void (*display)(EditState *);

    /* text related functions */
    int (*text_display)(EditState *, struct DisplayState *, int);
    int (*text_backward_offset)(EditState *, int);

    /* common functions are defined here */
    void (*move_up_down)(EditState *, int);
    void (*move_left_right)(EditState *, int);
    void (*move_bol)(EditState *);
    void (*move_eol)(EditState *);
    void (*move_word_left_right)(EditState *, int);
    void (*scroll_up_down)(EditState *, int);
    void (*scroll_line_up_down)(EditState *, int);
    void (*write_char)(EditState *, int);
    void (*mouse_goto)(EditState *, int x, int y);
    int mode_flags;
#define MODEF_NOCMD 0x0001 /* do not register xxx-mode command automatically */
    EditBufferDataType *data_type; /* native buffer data type (NULL = raw) */
    void (*mode_line)(EditState *s, char *buf, int buf_size); /* return mode line */
    struct ModeDef *next;
} ModeDef;

/* special bit to indicate tty styles (for shell mode) */
#define QE_STYLE_TTY     0x800
#define TTY_GET_COLOR(fg, bg) (((fg) << 3) | (bg))
#define TTY_GET_FG(color) (((color) >> 3) & 7)
#define TTY_GET_BG(color) ((color) & 7)

extern unsigned int tty_colors[]; /* from tty.c */

/* special selection style (cumulative with another style) */
#define QE_STYLE_SEL     0x400

enum QEStyle {
#define STYLE_DEF(constant, name, fg_color, bg_color, \
                  font_style, font_size, text_style) \
                  constant,

#include "qestyles.h"

#undef STYLE_DEF
    QE_STYLE_NB,
};

typedef struct QEStyleDef {
    const char *name;
    // if any style is 0, then default edit style applies
    QEColor fg_color, bg_color; 
    short font_style;
    short font_size;
    short text_style;
} QEStyleDef;

extern QEStyleDef qe_styles[QE_STYLE_NB];
    
#define NB_YANK_BUFFERS 10

typedef struct QErrorContext {
    const char *function;
    const char *filename;
    int lineno;
} QErrorContext;

typedef struct QEmacsState {
    QEditScreen *screen;
    struct EditState *first_window;
    struct EditState *active_window; //!< window in which we edit
    struct EditBuffer *first_buffer;
    // global layout info : DO NOT modify these directly. 
	// call `do_refresh` it does it
    int status_height;
    int mode_line_height;
    int content_height;       //!< height excluding status line
    int width, height;
    int border_width;
    int separator_width;
    // full screen state
    int hide_status;          //!< true if status should be hidden
    int complete_refresh;
    int is_full_screen;
    // commands 
    int flag_split_window_change_focus;
    void *last_cmd_func;      //!< last executed command function call 
    // keyboard macros
    int defining_macro;
    unsigned short *macro_keys;
    int nb_macro_keys;
    int macro_keys_size;
    int macro_key_index;      //!< -1 means no macro is being executed 
    int ungot_key;
    // yank buffers
    EditBuffer *yank_buffers[NB_YANK_BUFFERS];
    int yank_current;
    char res_path[1024];
    char status_shadow[MAX_SCREEN_WIDTH];
    QErrorContext ec;
    char system_fonts[NB_FONT_FAMILIES][256];
} QEmacsState;

extern QEmacsState qe_state;

/* key bindings definitions */

/**
 * dynamic key binding storage 
 */
typedef struct KeyDef {
    int nb_keys;
    struct CmdDef *cmd;
    ModeDef *mode;           //!< if non NULL, key is only active in this mode
    struct KeyDef *next;
    unsigned int keys[1];
} KeyDef;

void unget_key(int key);

/**
 * command definitions 
 */
typedef struct CmdDef {
    rune key;       //!< normal key
    rune alt_key;   //!< alternate key
    const char *name;
    union {
        void *func;
        struct CmdDef *next;
    } action;
    void *val;
} CmdDef;

/** New style command that takes no parameters */
#define CMD_(key, key_alt, name, func, args) { key, key_alt, name "\0" args, { (void *)(func) }, 0 },
/** New style command that takes any number of parameters */
#define CMDV(key, key_alt, name, func, val, args) { key, key_alt, name "\0" args, { (void *)(func) }, (void *)(val) },

/** 
 * Old style command that takes no parameters (only function pointer) 
 * @see CMD_
 */
#define CMD0(key, key_alt, name, func) { key, key_alt, name "\0", { (void *)(func) } },
/** 
 * Old style command that takes 1 parameter 
 * @see CMDV
 */
#define CMD1(key, key_alt, name, func, val) { key, key_alt, name "\0v", { (void *)(func) }, (void*)(val) },
/** Mark the end of the key -> function mapping */
#define CMD_DEF_END { 0, 0, NULL, { NULL }, 0 }

void qe_register_mode(ModeDef *m);
void qe_register_cmd_table(CmdDef *cmds, const char *mode);
void qe_register_binding(int key, const char *cmd_name, 
                         const char *mode_names);

/**
 * text display system
 */
typedef struct TextFragment {
    unsigned short embedding_level;
    short width;              //!< fragment width 
    short ascent; 
    short descent;
    short style;              //!< style index
    short line_index;         //!< index in line_buf 
    short len;                //!< number of glyphs 
    short dummy;              //!< align, must be assigned for CRC 
} TextFragment;

#define MAX_WORD_SIZE 128
#define NO_CURSOR 0x7fffffff

#define STYLE_BITS       12
#define STYLE_SHIFT      (32 - STYLE_BITS)
#define STYLE_MASK       (((1 << STYLE_BITS) - 1) << STYLE_SHIFT)
#define CHAR_MASK        ((1 << STYLE_SHIFT) - 1)

typedef struct DisplayState {
    int do_disp;               //!< true if real display 
    int width;                 //!< display window width
    int height;                //!< display window height
    int eol_width;             //!< width of eol char 
    int default_line_height;   //!< line height if no chars 
    int tab_width;             //!< width of tabulation 
    int x_disp;                //!< starting x display
    int x;                     //!< current x position
    int y;                     //!< current y position
    int line_num;              //!< current text line number
    int cur_hex_mode;          //!< true if current char is in hex mode 
    int hex_mode;              //!< hex mode from edit_state, -1 if all chars wanted
    void *cursor_opaque;
    int (*cursor_func)(struct DisplayState *, 
                       int offset1, int offset2, int line_num,
                       int x, int y, int w, int h, int hex_mode);
    int eod;                   //!< end of display requested 
    // if base == RTL, then all x are equivalent to width - x 
    DirType base; 
    int embedding_level_max;
    int wrap;
    int eol_reached;
    EditState *edit_state;
    
    /* fragment buffers */
    TextFragment fragments[MAX_SCREEN_WIDTH];
    int nb_fragments;
    int last_word_space;       //!< true if last word was a space 
    int word_index;            //!< fragment index of the start of the current word 
    unsigned int line_chars[MAX_SCREEN_WIDTH]; //!< line char (in fact glyph) buffer
    short line_char_widths[MAX_SCREEN_WIDTH];
    int line_offsets[MAX_SCREEN_WIDTH][2]; 
    unsigned char line_hex_mode[MAX_SCREEN_WIDTH];
    int line_index;

    unsigned int fragment_chars[MAX_WORD_SIZE];    //!< fragment temporary buffer
    int fragment_offsets[MAX_WORD_SIZE][2];
    unsigned char fragment_hex_mode[MAX_WORD_SIZE];
    int fragment_index;
    int last_space;
    int last_style;
    int last_embedding_level;
} DisplayState;

enum DisplayType {
    DISP_CURSOR,
    DISP_PRINT,
    DISP_CURSOR_SCREEN,
};

void display_init(DisplayState *s, EditState *e, enum DisplayType do_disp);
void display_bol(DisplayState *s);
void display_setcursor(DisplayState *s, DirType dir);
int display_char_bidir(DisplayState *s, int offset1, int offset2,
                       int embedding_level, int ch);
void display_eol(DisplayState *s, int offset1, int offset2);

void display_printf(DisplayState *ds, int offset1, int offset2,
                    const char *fmt, ...);
void display_printhex(DisplayState *s, int offset1, int offset2, 
                      unsigned int h, int n);

static inline int display_char(DisplayState *s, int offset1, int offset2,
                               int ch)
{
    return display_char_bidir(s, offset1, offset2, 0, ch);
}

/* ///////////////////////////////////////////////////////////////////////// */

/* the following will be suppressed */
#define LINE_MAX_SIZE 256

static inline int align(int a, int n)
{
    return (a/n)*n;
}

/* minibuffer & status */

void less_mode_init(void);
void minibuffer_init(void);

typedef void (*CompletionFunc)(StringArray *cs, const char *input);

typedef struct CompletionEntry {
    const char *name;
    CompletionFunc completion_func;
    struct CompletionEntry *next;
} CompletionEntry;

void register_completion(const char *name, CompletionFunc completion_func);
void put_status(EditState *s, const char *fmt, ...);
void put_error(EditState *s, const char *fmt, ...);
void minibuffer_edit(const char *input, const char *prompt, 
                     StringArray *hist, CompletionFunc completion_func,
                     void (*cb)(void *opaque, char *buf), void *opaque);
void command_completion(StringArray *cs, const char *input);
void file_completion(StringArray *cs, const char *input);
void buffer_completion(StringArray *cs, const char *input);
void color_completion(StringArray *cs, const char *input);

extern int __fast_test_event_poll_flag;
int __is_user_input_pending(void);

static inline int is_user_input_pending(void)
{
    if (__fast_test_event_poll_flag) {
        __fast_test_event_poll_flag = 0;
        return __is_user_input_pending();
    } else {
        return 0;
    }
}

/* config file support */
void parse_config(EditState *e, const char *file);
void do_load_qirc(EditState *e, const char *file);

/* popup / low level window handling */
void show_popup(EditBuffer *b);
EditState *insert_window_left(EditBuffer *b, int width, int flags);
EditState *find_window(EditState *s, int key);
void do_find_window(EditState *s, int key);

/* window handling */
void edit_close(EditState *s);
EditState *edit_new(EditBuffer *b,
                    int x1, int y1, int width, int height, int flags);
void edit_detach(EditState *s);
void edit_append(EditState *s, EditState *e);
EditState *edit_find(EditBuffer *b);
void do_refresh(EditState *s);
void do_other_window(EditState *s);
void do_delete_window(EditState *s, int force);
void edit_display(QEmacsState *qs);
void edit_invalidate(EditState *s);

void do_revert_buffer(EditState *s);

/* text mode */
extern ModeDef text_mode;
int text_mode_init(EditState *s, ModeSavedData *saved_data);
void text_mode_close(EditState *s);
int text_backward_offset(EditState *s, int offset);
int text_display(EditState *s, DisplayState *ds, int offset);

void set_colorize_func(EditState *s, ColorizeFunc colorize_func);
int get_colorized_line(EditState *s, unsigned int *buf, int buf_size,
                       int offset1, int line_num);
void set_color(unsigned int *buf, int len, int style);

void do_char(EditState *s, int key);
void do_switch_to_buffer(EditState *s, const char *bufname);
void do_set_mode(EditState *s, ModeDef *m, ModeSavedData *saved_data);
void text_move_left_right_visual(EditState *s, int dir);
void text_move_word_left_right(EditState *s, int dir);
void text_move_up_down(EditState *s, int dir);
void text_scroll_up_down(EditState *s, int dir);
void text_write_char(EditState *s, int key);
void do_return(EditState *s);
void do_backspace(EditState *s);
void do_delete_char(EditState *s);
void do_tab(EditState *s);
void do_kill_region(EditState *s, int kill);
void do_kill_buffer(EditState *s, const char *bufname1);
void text_move_bol(EditState *s);
void text_move_eol(EditState *s);
void do_load(EditState *s, const char *filename);
void do_load_from_path(EditState *s, const char *filename);
void do_goto_line(EditState *s, int line);
void switch_to_buffer(EditState *s, EditBuffer *b);
void do_up_down(EditState *s, int dir);
void display_mode_line(EditState *s);
// void text_mouse_goto(EditState *s, int x, int y);
EditBuffer *new_yank_buffer(void);
void basic_mode_line(EditState *s, char *buf, int buf_size, int c1);
void text_mode_line(EditState *s, char *buf, int buf_size);
void do_toggle_full_screen(EditState *s);

void do_bof(EditState *s);
void do_eof(EditState *s);
void do_bol(EditState *s);
void do_eol(EditState *s);
void do_word_right(EditState *s, int dir);

void do_noop(EditState *s);

/* ///////////////////////////////////////////////////////////////////////// */
void do_indent_lastline(EditState *s);
void run_system_cmd(EditState *s, const char **cmd);
/* ///////////////////////////////////////////////////////////////////////// */

/* ///////////////////////////////////////////////////////////////////////// */
/* hex.c */
void hex_write_char(EditState *s, int key);

/* ///////////////////////////////////////////////////////////////////////// */
/* list.c */
extern ModeDef list_mode;

void list_toggle_selection(EditState *s);
int list_get_pos(EditState *s);
int list_get_offset(EditState *s);

void get_style(EditState *e, QEStyleDef *style, int style_index);


int gxml_mode_init(EditState *s, 
                   ModeSavedData *saved_data,
                   int is_html, const char *default_stylesheet);
#endif
