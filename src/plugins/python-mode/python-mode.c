/*
 * Python mode for Qi.
 * Copyright (c) 2013 Rob Rohan.
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

static const char python_keywords[] = 
"|and|del|for|is|raise|assert|elif|from|lambda|return|"
"break|else|global|not|try|class|except|if|or|while|"
"continue|exec|import|pass|yield|def|finally|in|print";

static const char python_types[] = 
"|char|double|float|int|long|unsigned|short|signed|void|var|function|";


/* colorization states */
enum {
    PYTHON_COMMENT = 1,
    PYTHON_STRING,
    PYTHON_STRING_Q,
    PYTHON_PREPROCESS,
};

#define MAX_BUF_SIZE    512
#define MAX_STACK_SIZE  64

enum {
    INDENT_NORM,
    INDENT_FIND_EQ,
};

static int get_python_keyword(char *buf, int buf_size, unsigned int **pp)
{
    unsigned int *p, c;
    char *q;

    p = *pp;
    c = *p;
    q = buf;
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') || 
        (c == '_')) {
        do {
            if ((q - buf) < buf_size - 1)
                *q++ = c;
            p++;
            c = *p;
        } while ((c >= 'a' && c <= 'z') ||
                 (c >= 'A' && c <= 'Z') ||
                 (c == '_') ||
                 (c >= '0' && c <= '9'));
    }
    *q = '\0';
    *pp = p;
    return q - buf;
}

void python_colorize_line(unsigned int *buf, int len, 
                     int *colorize_state_ptr, int state_only)
{
    int c, state, type_decl;
    unsigned int *p, *p_start, *p1;
    char kbuf[32];

    state = *colorize_state_ptr;
    p = buf;
    p_start = p;
    type_decl = 0;
    c = 0;      /* turn off stupid egcs-2.91.66 warning */

    /* if already in a state, go directly in the code parsing it */
    switch (state) {
    case PYTHON_COMMENT:
        //goto parse_comment;
    case PYTHON_STRING:
    case PYTHON_STRING_Q:
        goto parse_string;
    default:
        break;
    }

    for (;;) {
        p_start = p;
        c = *p;
        switch (c) {
        case '\n':
            goto the_end;
        case '#':
            p = buf + len;
            set_color(p_start, p - p_start, QE_STYLE_COMMENT);
            state = QE_STYLE_COMMENT;
            //if (p > buf && (p[-1] & CHAR_MASK) == '\\') 
            //    state = QE_STYLE_COMMENT;
            //else
            //    state = QE_STYLE_DEFAULT;
            goto the_end;
        case '\'':
            state = PYTHON_STRING_Q;
            goto string;
        case '\"':
            // strings/chars
            state = PYTHON_STRING;
string:
            p++;
parse_string:
            while (*p != '\n') {
                if (*p == '\\') {
                    p++;
                    if (*p == '\n')
                        break;
                    p++;
                } else if ((*p == '\'' && state == PYTHON_STRING_Q) ||
                           (*p == '\"' && state == PYTHON_STRING)) {
                    p++;
                    state = QE_STYLE_DEFAULT;
                    break;
                } else {
                    p++;
                }
            }
            set_color(p_start, p - p_start, QE_STYLE_STRING);
            break;
        case '=':
            p++;
            // exit type declaration 
            type_decl = 0;
            break;
        case '\t':
            p++;
            set_color(p_start, p - p_start, QE_STYLE_HIGHLIGHT);
            break;
        default:
            if ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') || 
                (c == '_')) {
                
                get_python_keyword(kbuf, sizeof(kbuf), &p);
                p1 = p;
                while (*p == ' ' || *p == '\t')
                    p++;
                if (strfind(python_keywords, kbuf, 0)) {
                    set_color(p_start, p1 - p_start, QE_STYLE_KEYWORD);
                } else
                if (strfind(python_types, kbuf, 0)) {
                    /* c type */
                    /* if not cast, assume type declaration */
                    if (*p != ')') {
                        type_decl = 1;
                    }
                    set_color(p_start, p1 - p_start, QE_STYLE_TYPE);
                } else {
                    /* assume typedef if starting at first column */
                    if (p_start == buf)
                        type_decl = 1;

                    if (type_decl) {
                        if (*p == '(') {
                            /* function definition case */
                            set_color(p_start, p1 - p_start, QE_STYLE_FUNCTION);
                            type_decl = 1;
                        } else if (p_start == buf) {
                            /* assume type if first column */
                            set_color(p_start, p1 - p_start, QE_STYLE_TYPE);
                        } else {
                            set_color(p_start, p1 - p_start, QE_STYLE_VARIABLE);
                        }
                    }
                }
            } else {
                p++;
            }
            break;
        }
    }
the_end:
    *colorize_state_ptr = state;
}

static int python_mode_probe(ModeProbeData *p)
{
    const char *r;

    //currently, only use the file extension
    r = extension(p->filename);
    if (*r) {
        if (strfind("|py|", r + 1, 1))
            return 100;
    }
    return 0;
}

int python_mode_init(EditState *s, ModeSavedData *saved_data)
{
    int ret;

    ret = text_mode_init(s, saved_data);
    if (ret)
        return ret;
    set_colorize_func(s, python_colorize_line);
    return ret;
}

void do_python_electric(EditState *s, int key)
{
    do_char(s, key);
    do_indent_lastline(s);
}

static CmdDef python_commands[] = {
    CMDV( KEY_RET, KEY_NONE, "python-electric-newline", do_python_electric, '\n', "*v")

    CMD_DEF_END,
};

static ModeDef python_mode;

int python_init(void)
{
    /* python mode is almost like the text mode, so we copy and patch it */
    memcpy(&python_mode, &text_mode, sizeof(ModeDef));
    python_mode.name = "Python";
    python_mode.mode_probe = python_mode_probe;
    python_mode.mode_init = python_mode_init;

    qe_register_mode(&python_mode);

    qe_register_cmd_table(python_commands, "Python");

    return 0;
}

