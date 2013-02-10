/*
 * Utilities for qemacs.
 * Copyright (c) 2001 Fabrice Bellard.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "qe.h"
#include <dirent.h>

#ifdef WIN32
#include <sys/timeb.h>

/* XXX: not suffisant, but OK for basic operations */
int fnmatch(const char *pattern, const char *string, int flags)
{
    if (pattern[0] == '*')
        return 0;
    else
        return strcmp(pattern, string) != 0;
}

#else
#include <fnmatch.h>
#endif

struct FindFileState {
    char path[MAX_FILENAME_SIZE];
    char dirpath[MAX_FILENAME_SIZE]; /* current dir path */
    char pattern[MAX_FILENAME_SIZE]; /* search pattern */
    const char *bufptr;
    DIR *dir;
};

FindFileState *find_file_open(const char *path, const char *pattern)
{
    FindFileState *s;

    s = malloc(sizeof(FindFileState));
    if (!s)
        return NULL;
    pstrcpy(s->path, sizeof(s->path), path);
    pstrcpy(s->pattern, sizeof(s->pattern), pattern);
    s->bufptr = s->path;
    s->dir = NULL;
    return s;
}
                     

int find_file_next(FindFileState *s, char *filename, int filename_size_max)
{
    struct dirent *dirent;
    const char *p;
    char *q;
    
    if (s->dir == NULL)
        goto redo;

    for (;;) {
        dirent = readdir(s->dir);
        if (dirent == NULL) {
        redo:
            if (s->dir) {
                closedir(s->dir);
                s->dir = NULL;
            }
            p = s->bufptr;
            if (*p == '\0')
                return -1;
            /* CG: get_str(&p, s->dirpath, sizeof(s->dirpath), ":") */
            q = s->dirpath;
            while (*p != ':' && *p != '\0') {
                if ((q - s->dirpath) < (int)sizeof(s->dirpath) - 1)
                    *q++ = *p;
                p++;
            }
            *q = '\0';
            if (*p == ':')
                p++;
            s->bufptr = p;
            s->dir = opendir(s->dirpath);
            if (!s->dir)
                goto redo;
        } else {
            if (fnmatch(s->pattern, dirent->d_name, 0) == 0) {
                makepath(filename, filename_size_max,
                         s->dirpath, dirent->d_name);
                return 0;
            }
        }
    }
}

void find_file_close(FindFileState *s)
{
    if (s->dir) 
        closedir(s->dir);
    free(s);
}

#ifdef WIN32
/* convert '/' to '\' */
static void path_win_to_unix(char *buf)
{
    char *p;
    p = buf;
    while (*p) {
        if (*p == '\\')
            *p = '/';
        p++;
    }
}
#endif

/* suppress redundant ".", ".." and "/" from paths */
/* XXX: make it better */
static void canonize_path1(char *buf, int buf_size, const char *path)
{
    const char *p;
    char *q, *q1;
    int c, abs_path;
    char file[MAX_FILENAME_SIZE];

    p = path;
    abs_path = (p[0] == '/');
    buf[0] = '\0';
    for (;;) {
        /* extract file */
        q = file;
        for (;;) {
            c = *p;
            if (c == '\0')
                break;
            p++;
            if (c == '/')
                break;
            if ((q - file) < (int)sizeof(file) - 1)
                *q++ = c;
        }
        *q = '\0';

        if (file[0] == '\0') {
            /* nothing to do */
        } else if (file[0] == '.' && file[1] == '\0') {
            /* nothing to do */
        } else if (file[0] == '.' && file[1] == '.' && file[2] == '\0') {
            /* go up one dir */
            if (buf[0] == '\0') {
                if (!abs_path)
                    goto copy;
            } else {
                /* go to previous directory, if possible */
                q1 = strrchr(buf, '/');
                /* if already going up, cannot do more */
                if (!q1 || (q1[1] == '.' && q1[2] == '.' && q1[3] == '\0'))
                    goto copy;
                else
                    *q1 = '\0';
            }
        } else {
        copy:
            /* add separator if needed */
            if (buf[0] != '\0' || (buf[0] == '\0' && abs_path))
                pstrcat(buf, buf_size, "/");
            pstrcat(buf, buf_size, file);
        }
        if (c == '\0')
            break;
    }

    /* add at least '.' or '/' */
    if (buf[0] == '\0') {
        if (abs_path)
            pstrcat(buf, buf_size, "/");
        else
            pstrcat(buf, buf_size, ".");
    }
}

void canonize_path(char *buf, int buf_size, const char *path)
{
    const char *p;
    /* check for URL protocol or windows drive */
    /* CG: should not skip '/' */
    p = strchr(path, ':');
    if (p) {
        if ((p - path) == 1) {
            /* windows drive : we canonize only the following path */
            buf[0] = p[0];
            buf[1] = p[1];
            /* CG: this will not work for non current drives */
            canonize_path1(buf + 2, buf_size - 2, p);
        } else {
            /* URL: it is already canonized */
            pstrcpy(buf, buf_size, path);
        }
    } else {
        /* simple unix path */
        canonize_path1(buf, buf_size, path);
    }
}

/* return TRUE if absolute path. works for files and URLs */
static int is_abs_path(const char *path)
{
    const char *p;
    p = strchr(path, ':');
    if (p)
        p++;
    else
        p = path;
    return *p == '/';
}

/* canonize the path and make it absolute */
void canonize_absolute_path(char *buf, int buf_size, const char *path1)
{
    char cwd[MAX_FILENAME_SIZE];
    char path[MAX_FILENAME_SIZE];
    char *homedir;

    if (!is_abs_path(path1)) {
        /* XXX: should call it again */
        if (*path1 == '~' && (path1[1] == '\0' || path1[1] == '/')) {
            homedir = getenv("HOME");
            if (homedir) {
                pstrcpy(path, sizeof(path), homedir);
                pstrcat(path, sizeof(path), path1 + 1);
                goto next;
            }
        }
        /* CG: not sufficient for windows drives */
        getcwd(cwd, sizeof(cwd));
#ifdef WIN32
        path_win_to_unix(cwd);
#endif
        makepath(path, sizeof(path), cwd, path1);
    } else {
        pstrcpy(path, sizeof(path), path1);
    }
next:
    canonize_path(buf, buf_size, path);
}

/* last filename in a path */
const char *basename(const char *filename)
{
    const char *p;
    p = strrchr(filename, '/');
    if (!p) {
        /* should also scan for ':' */
        return filename;
    } else {
        p++;
        return p;
    }
}

/* last extension in a path, ignoring leading dots */
const char *extension(const char *filename)
{
    const char *p, *ext;

    ext = NULL;
    p = filename;
restart:
    while (*p == '.')
        p++;
    ext = NULL;
    for (; *p; p++) {
        if (*p == '/') {
            p++;
            goto restart;
        }
        if (*p == '.')
            ext = p;
    }
    return ext ? ext : p;
}

char *makepath(char *buf, int buf_size, const char *path,
               const char *filename)
{
    int len;

    pstrcpy(buf, buf_size, path);
    len = strlen(path);
    if (len > 0 && path[len - 1] != '/' && len + 1 < buf_size) {
        buf[len++] = '/';
        buf[len] = '\0';
    }
    return pstrcat(buf, buf_size, filename);
}

void splitpath(char *dirname, int dirname_size,
               char *filename, int filename_size, const char *pathname)
{
    const char *base;

    base = basename(pathname);
    if (dirname)
        pstrncpy(dirname, dirname_size, pathname, base - pathname);
    if (filename)
        pstrcpy(filename, filename_size, base);
}

/* find a word in a list using '|' as separator,
 * optionally fold case to lower case.
 */
int strfind(const char *keytable, const char *str, int casefold)
{
    char buf[128];
    int c, len;
    const char *p;

    if (casefold) {
        pstrcpy(buf, sizeof(buf), str);
        str = buf;
        css_strtolower(buf, sizeof(buf));
    }
    c = *str;
    len = strlen(str);
    /* need to special case the empty string */
    if (len == 0)
        return strstr(keytable, "||") != NULL;

    /* initial and trailing | are optional */
    /* they do not cause the empty string to match */
    for (p = keytable;;) {
        if (!memcmp(p, str, len) && (p[len] == '|' || p[len] == '\0'))
            return 1;
        for (;;) {
            p = strchr(p + 1, c);
            if (!p)
                return 0;
            if (p[-1] == '|')
                break;
        }
    }
}

void skip_spaces(const char **pp)
{
    const char *p;

    p = *pp;
    while (css_is_space(*p))
        p++;
    *pp = p;
}

/* need this for >= 256 */
static inline int utoupper(int c)
{
    if (c >= 'a' && c <= 'z')
        c += 'A' - 'a';
    return c;
}

int ustristart(const unsigned int *str, const char *val,
               const unsigned int **ptr)
{
    const unsigned int *p;
    const char *q;
    p = str;
    q = val;
    while (*q != '\0') {
        if (utoupper(*p) != utoupper(*q))
            return 0;
        p++;
        q++;
    }
    if (ptr)
        *ptr = p;
    return 1;
}

/* Read a token from a string, stop a set of characters.
 * Skip spaces before and after token.
 */ 
void get_str(const char **pp, char *buf, int buf_size, const char *stop)
{
    char *q;
    const char *p;
    int c;

    skip_spaces(pp);
    p = *pp;
    q = buf;
    for (;;) {
        c = *p;
        /* Should stop on spaces and eat them */
        if (c == '\0' || css_is_space(c) || strchr(stop, c))
            break;
        if ((q - buf) < buf_size - 1)
            *q++ = c;
        p++;
    }
    *q = '\0';
    *pp = p;
    skip_spaces(pp);
}

int css_get_enum(const char *str, const char *enum_str)
{
    int val, len;
    const char *s, *s1;

    s = enum_str;
    val = 0;
    len = strlen(str);
    for (;;) {
        s1 = strchr(s, ',');
        if (!s1) {
            if (!strcmp(s, str))
                return val;
            else
                break;
        } else {
            if (len == (s1 - s) && !memcmp(s, str, len))
                return val;
            s = s1 + 1;
        }
        val++;
    }
    return -1;
}

unsigned short keycodes[] = {
    KEY_SPC, KEY_DEL, KEY_RET, KEY_ESC, KEY_TAB, KEY_SHIFT_TAB,
    KEY_CTRL(' '), KEY_DEL, KEY_CTRL('\\'),
    KEY_CTRL(']'), KEY_CTRL('^'), KEY_CTRL('_'),
    KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN,
    KEY_HOME, KEY_END, KEY_PAGEUP, KEY_PAGEDOWN,
    KEY_CTRL_LEFT, KEY_CTRL_RIGHT, KEY_CTRL_UP, KEY_CTRL_DOWN,
    KEY_CTRL_HOME, KEY_CTRL_END, KEY_CTRL_PAGEUP, KEY_CTRL_PAGEDOWN,
    KEY_PAGEUP, KEY_PAGEDOWN, KEY_CTRL_PAGEUP, KEY_CTRL_PAGEDOWN,
    KEY_INSERT, KEY_DELETE, KEY_DEFAULT,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
    KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,
    KEY_F11, KEY_F12, KEY_F13, KEY_F14, KEY_F15,
    KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20,
};

const char *keystr[] = {
    "SPC", "DEL", "RET", "ESC", "TAB", "S-TAB",
    "C-SPC", "C-?", "C-\\", "C-]", "C-^", "C-_",
    "left", "right", "up", "down",
    "home", "end", "prior", "next",
    "C-left", "C-right", "C-up", "C-down",
    "C-home", "C-end", "C-prior", "C-next",
    "pageup", "pagedown", "C-pageup", "C-pagedown", 
    "insert", "delete", "default",
    "f1", "f2", "f3", "f4", "f5",
    "f6", "f7", "f8", "f9", "f10",
    "f11", "f12", "f13", "f14", "f15",
    "f16", "f17", "f18", "f19", "f20",
};

int compose_keys(unsigned int *keys, int *nb_keys)
{
    unsigned int *keyp;

    if (*nb_keys < 2)
        return 0;

    /* compose KEY_ESC as META prefix */
    keyp = keys + *nb_keys - 2;
    if (keyp[0] == KEY_ESC) {
        if (keyp[1] <= 0xff) {
            keyp[0] = KEY_META(keyp[1]);
            --*nb_keys;
            return 1;
        }
    }
    return 0;
}

/* CG: this code is still quite inelegant */
static int strtokey1(const char **pp)
{
    const char *p, *p1, *q;
    int i, key;

    /* should return KEY_NONE at end and KEY_UNKNOWN if unrecognized */
    p = *pp;

    /* scan for separator */
    for (p1 = p; *p1 && *p1 != ' '; p1++)
        continue;

    for (i = 0; i < sizeof(keycodes)/sizeof(keycodes[0]); i++) {
        if (strstart(p, keystr[i], &q) && q == p1) {
            key = keycodes[i];
            *pp = p1;
            return key;
        }
    }
#if 0
    if (p[0] == 'f' && p[1] >= '1' && p[1] <= '9') {
        i = p[1] - '0';
        p += 2;
        if (p1 == isdigit((unsigned char)*p))
            i = i * 10 + *p++ - '0';
        key = KEY_F1 + i - 1;
        *pp = p1;
        return key;
    }
#endif
    if (p[0] == 'C' && p[1] == '-' && p1 == p + 3) {
        /* control */
        key = KEY_CTRL(p[2]);
    } else {
        key = utf8_decode(&p);
    }
    *pp = p1;

    return key;
}

int strtokey(const char **pp)
{
    const char *p;
    int key;

    p = *pp;
    if (p[0] == 'M' && p[1] == '-') {
        p += 2;
        key = KEY_META(strtokey1(&p));
    } else
    if (p[0] == 'C' && p[1] == '-' && p[0] == 'M' && p[1] == '-') {
        p += 4;
        key = KEY_META(KEY_CTRL(strtokey1(&p)));
    } else {
        key = strtokey1(&p);
    }
    *pp = p;
    return key;
}

int strtokeys(const char *keystr, unsigned int *keys, int max_keys)
{
    int key, nb_keys;
    const char *p;

    p = keystr;
    nb_keys = 0;

    for (;;) {
        skip_spaces(&p);
        if (*p == '\0')
            break;
        key = strtokey(&p);
        keys[nb_keys++] = key;
        compose_keys(keys, &nb_keys);
        if (nb_keys >= max_keys)
            break;
    }
    return nb_keys;
}

void keytostr(char *buf, int buf_size, int key)
{
    int i;
    char buf1[32];
    
    for (i = 0; i < (int)(sizeof(keycodes)/sizeof(keycodes[0])); i++) {
        if (keycodes[i] == key) {
            pstrcpy(buf, buf_size, keystr[i]);
            return;
        }
    }
    if (key >= KEY_META(0) && key <= KEY_META(0xff)) {
        keytostr(buf1, sizeof(buf1), key & 0xff);
        snprintf(buf, buf_size, "M-%s", buf1);
    } else if (key >= KEY_CTRL('a') && key <= KEY_CTRL('z')) {
        snprintf(buf, buf_size, "C-%c", key + 'a' - 1);
    } else if (key >= KEY_F1 && key <= KEY_F20) {
        snprintf(buf, buf_size, "f%d", key - KEY_F1 + 1);
    } else if (key > 32 && key < 127 && buf_size >= 2) {
        buf[0] = key;
        buf[1] = '\0';
    } else {
        char *q;
        q = utf8_encode(buf1, key);
        *q = '\0';
        pstrcpy(buf, buf_size, buf1);
    }
}

int to_hex(int key)
{
    if (key >= '0' && key <= '9')
        return key - '0';
    else if (key >= 'a' && key <= 'f')
        return key - 'a' + 10;
    else if (key >= 'A' && key <= 'F')
        return key - 'A' + 10;
    else
        return -1;
}

typedef struct ColorDef {
    const char *name;
    unsigned int color;
} ColorDef;

static ColorDef css_colors[] = {
    /*from HTML 4.0 spec */
    { "black", QERGB(0x00, 0x00, 0x00) },
    { "green", QERGB(0x00, 0x80, 0x00) },
    { "silver", QERGB(0xc0, 0xc0, 0xc0) },
    { "lime", QERGB(0x00, 0xff, 0x00) },

    { "gray", QERGB(0xbe, 0xbe, 0xbe) },
    { "olive", QERGB(0x80, 0x80, 0x00) },
    { "white", QERGB(0xff, 0xff, 0xff) },
    { "yellow", QERGB(0xff, 0xff, 0x00) },

    { "maroon", QERGB(0x80, 0x00, 0x00) },
    { "navy", QERGB(0x00, 0x00, 0x80) },
    { "red", QERGB(0xff, 0x00, 0x00) },
    { "blue", QERGB(0x00, 0x00, 0xff) },

    { "purple", QERGB(0x80, 0x00, 0x80) },
    { "teal", QERGB(0x00, 0x80, 0x80) },
    { "fuchsia", QERGB(0xff, 0x00, 0xff) },
    { "aqua", QERGB(0x00, 0xff, 0xff) },
    
    /* more colors */
    { "cyan", QERGB(0x00, 0xff, 0xff) },
    { "magenta", QERGB(0xff, 0x00, 0xff) },
    { "transparent", COLOR_TRANSPARENT },
};
#define nb_css_colors (sizeof(css_colors) / sizeof(css_colors[0]))

static ColorDef *custom_colors = css_colors;
static int nb_custom_colors;

void color_completion(StringArray *cs, const char *input)
{
    ColorDef *def;
    int len, count;

    len = strlen(input);
    def = custom_colors;
    count = nb_css_colors + nb_custom_colors;
    while (count > 0) {
        if (!strncmp(def->name, input, len))
            add_string(cs, def->name);
        def++;
        count--;
    }
}

static ColorDef *css_lookup_color(ColorDef *def, int count,
                                  const char *name)
{
    while (count > 0) {
        if (!strcasecmp(name, def->name)) {
            return def;
        }
        def++;
        count--;
    }
    return NULL;
}

int css_define_color(const char *name, const char *value)
{
    ColorDef *def;
    QEColor color;

    /* Check color validity */
    if (css_get_color(&color, value))
        return -1;

    /* Make room: reallocate table in chunks of 8 entries */
    if ((nb_custom_colors & 7) == 0) {
        def = malloc((nb_css_colors + nb_custom_colors + 8) *
                     sizeof(ColorDef));
        if (!def)
            return -1;
        memcpy(def, custom_colors,
               (nb_css_colors + nb_custom_colors) * sizeof(ColorDef));
            
        if (custom_colors != css_colors)
            free(custom_colors);
        custom_colors = def;
    }
    /* Check for redefinition */
    def = css_lookup_color(custom_colors, nb_css_colors + nb_custom_colors,
                           name);
    if (def) {
        def->color = color;
        return 0;
    }

    def = &custom_colors[nb_css_colors + nb_custom_colors];
    def->name = strdup(name);
    def->color = color;
    nb_custom_colors++;

    return 0;
}

/* XXX: make HTML parsing optional ? */
int css_get_color(QEColor *color_ptr, const char *p)
{
    const ColorDef *def;
    int len, v, i, n;
    unsigned char rgba[4];

    /* search in table */
    def = css_lookup_color(custom_colors, nb_css_colors + nb_custom_colors, p);
    if (def) {
        *color_ptr = def->color;
        return 0;
    }
    
    rgba[3] = 0xff;
    if (isxdigit((unsigned char)*p)) {
        goto parse_num;
    } else if (*p == '#') {
        /* handle '#' notation */
        p++;
    parse_num:
        len = strlen(p);
        switch (len) {
        case 3:
            for (i = 0; i < 3; i++) {
                v = to_hex(*p++);
                rgba[i] = v | (v << 4);
            }
            break;
        case 6:
            for (i = 0; i < 3; i++) {
                v = to_hex(*p++) << 4;
                v |= to_hex(*p++);
                rgba[i] = v;
            }
            break;
        default:
            /* error */
            return -1;
        }
    } else if (strstart(p, "rgb(", &p)) {
        n = 3;
        goto parse_rgba;
    } else if (strstart(p, "rgba(", &p)) {
        /* extension for alpha */
        n = 4;
    parse_rgba:
        for (i = 0; i < n; i++) {
            /* XXX: floats ? */
            skip_spaces(&p);
            v = strtol(p, (char **)&p, 0);
            if (*p == '%') {
                v = (v * 255) / 100;
                p++;
            }
            rgba[i] = v;
            skip_spaces(&p);
            if (*p == ',')
                p++;
        }
    } else {
        return -1;
    }
    *color_ptr = (rgba[0] << 16) | (rgba[1] << 8) |
        (rgba[2]) | (rgba[3] << 24);
    return 0;
}

/* return 0 if unknown font */
int css_get_font_family(const char *str)
{
    int v;
    if (!strcasecmp(str, "serif") ||
        !strcasecmp(str, "times"))
        v = QE_FAMILY_SERIF;
    else if (!strcasecmp(str, "sans") ||
             !strcasecmp(str, "arial") ||
             !strcasecmp(str, "helvetica"))
        v = QE_FAMILY_SANS;
    else if (!strcasecmp(str, "fixed") ||
             !strcasecmp(str, "monospace") ||
             !strcasecmp(str, "courier"))
        v = QE_FAMILY_FIXED;
    else
        v = 0; /* inherit */
    return v;
}

/* a = a union b */
void css_union_rect(CSSRect *a, CSSRect *b)
{
    if (css_is_null_rect(b))
        return;
    if (css_is_null_rect(a)) {
        *a = *b;
    } else {
        if (b->x1 < a->x1)
            a->x1 = b->x1;
        if (b->y1 < a->y1)
            a->y1 = b->y1;
        if (b->x2 > a->x2)
            a->x2 = b->x2;
        if (b->y2 > a->y2)
            a->y2 = b->y2;
    }
}

#ifdef __TINYC__

/* the glibc folks use wrappers, but forgot to put a compatibility
   function for non GCC compilers ! */
int stat(__const char *__path,
         struct stat *__statbuf)
{
    return __xstat(_STAT_VER, __path, __statbuf);
}
#endif

int get_clock_ms(void)
{
#ifdef CONFIG_WIN32
    struct _timeb tb;
    _ftime(&tb);
    return tb.time * 1000 + tb.millitm;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + (tv.tv_usec / 1000);
#endif
}

/* set one string. */
StringItem *set_string(StringArray *cs, int index, const char *str)
{
    StringItem *v;
    if (index >= cs->nb_items)
        return NULL;

    v = malloc(sizeof(StringItem) + strlen(str));
    if (!v)
        return NULL;
    v->selected = 0;
    strcpy(v->str, str);
    if (cs->items[index])
        free(cs->items[index]);
    cs->items[index] = v;
    return v;
}

/* make a generic array alloc */
StringItem *add_string(StringArray *cs, const char *str)
{
    StringItem **tmp;
    int n;

    if (cs->nb_items >= cs->nb_allocated) {
        n = cs->nb_allocated + 32;
        tmp = realloc(cs->items, n * sizeof(StringItem *));
        if (!tmp)
            return NULL;
        cs->items = tmp;
        cs->nb_allocated = n;
    }
    cs->items[cs->nb_items++] = NULL;
    return set_string(cs, cs->nb_items - 1, str);
}

void free_strings(StringArray *cs)
{
    int i;

    for (i = 0; i < cs->nb_items; i++)
        free(cs->items[i]);
    free(cs->items);
    memset(cs, 0, sizeof(StringArray));
}

void set_color(unsigned int *buf, int len, int style)
{
    int i;

    style <<= STYLE_SHIFT;
    for (i = 0; i < len; i++)
        buf[i] |= style;
}

void css_strtolower(char *buf, int buf_size)
{
    int c;
    /* XXX: handle unicode */
    while (*buf) {
        c = tolower(*buf);
        *buf++ = c;
    }
}

void umemmove(unsigned int *dest, unsigned int *src, int len)
{
    memmove(dest, src, len * sizeof(unsigned int));
}

/* copy the n first char of a string and truncate it. */
char *pstrncpy(char *buf, int buf_size, const char *s, int len)
{
        char *q;
        int c;

        if (buf_size > 0) {
                q = buf;
                if (len >= buf_size)
                        len = buf_size - 1;
                while (len > 0) {
                        c = *s++;
                        if (c == '\0')
                                break;
                        *q++ = c;
                        len--;
                }
                *q = '\0';
        }
        return buf;
}

/**
 * Add a memory region to a dynamic string. In case of allocation
 * failure, the data is not added. The dynamic string is guaranted to
 * be 0 terminated, although it can be longer if it contains zeros.
 *
 * @return 0 if OK, -1 if allocation error.  
 */
int qmemcat(QString *q, const unsigned char *data1, int len1)
{
        int new_len, len, alloc_size;
        unsigned char *data;

        data = q->data;
        len = q->len;
        new_len = len + len1;
    /* see if we got a new power of two */
    /* NOTE: we got this trick from the excellent 'links' browser */
        if ((len ^ new_len) >= len) {
        /* find immediately bigger 2^n - 1 */
                alloc_size = new_len;
                alloc_size |= (alloc_size >> 1);
                alloc_size |= (alloc_size >> 2);
                alloc_size |= (alloc_size >> 4);
                alloc_size |= (alloc_size >> 8);
                alloc_size |= (alloc_size >> 16);
        /* allocate one more byte for end of string marker */
                data = realloc(data, alloc_size + 1);
                if (!data)
                        return -1;
                q->data = data;
        }
        memcpy(data + len, data1, len1);
        data[new_len] = '\0'; /* we force a trailing '\0' */
        q->len = new_len;
        return 0;
}

/*
 * add a string to a dynamic string
 */
int qstrcat(QString *q, const char *str)
{
        return qmemcat(q, (unsigned char *)str, strlen(str));
}

/* XXX: we use a fixed size buffer */
int qprintf(QString *q, const char *fmt, ...)
{
        char buf[4096];
        va_list ap;
        int len, ret;

        va_start(ap, fmt);
        len = vsnprintf(buf, sizeof(buf), fmt, ap);
    /* avoid problems for non C99 snprintf() which can return -1 if overflow */
        if (len < 0)
                len = strlen(buf);
        ret = qmemcat(q, (unsigned char *)buf, len);
        va_end(ap);
        return ret;
}

