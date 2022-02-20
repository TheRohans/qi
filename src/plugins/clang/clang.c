/*
 * C mode for QEmacs.
 * Copyright (c) 2001, 2002 Fabrice Bellard.
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

/* NOTE: 'var' is added for javascript */
static const char c_keywords[] = 
"|auto|break|case|const|continue|do|else|enum|extern|for|goto|"
"if|register|return|static|struct|switch|typedef|union|volatile|while|"
"package|var|let|go|";

static const char c_types[] = 
"|char|double|float|int|long|unsigned|short|signed|void|"
"func|function|bool|int8|int16|int32|int64|int|uint8|"
"uint16|uint32|uint64|uint|float32|float64|complex64|"
"complex128|byte|rune|string|";       


static int get_c_keyword(char *buf, int buf_size, unsigned int **pp)
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

/* colorization states */
enum {
    C_COMMENT = 1,
    C_STRING,
    C_STRING_Q,
    C_PREPROCESS,
};

void c_colorize_line(unsigned int *buf, int len, 
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
    case C_COMMENT:
        goto parse_comment;
    case C_STRING:
    case C_STRING_Q:
        goto parse_string;
    case C_PREPROCESS:
        goto parse_preprocessor;
    default:
        break;
    }

    for (;;) {
        p_start = p;
        c = *p;
        switch (c) {
        case '\n':
            goto the_end;
        case '/':
            p++;
            if (*p == '*') {
                /* normal comment */
                p++;
                state = C_COMMENT;
            parse_comment:
                while (*p != '\n') {
                    if (p[0] == '*' && p[1] == '/') {
                        p += 2;
                        state = 0;
                        break;
                    } else {
                        p++;
                    }
                }
                set_color(p_start, p - p_start, QE_STYLE_COMMENT);
            } else if (*p == '/') {
                /* line comment */
                while (*p != '\n') 
                    p++;
                set_color(p_start, p - p_start, QE_STYLE_COMMENT);
            }
            break;
        case '#':
            /* preprocessor */
        parse_preprocessor:
            p = buf + len;
            set_color(p_start, p - p_start, QE_STYLE_PREPROCESS);
            if (p > buf && (p[-1] & CHAR_MASK) == '\\') 
                state = C_PREPROCESS;
            else
                state = 0;
            goto the_end;
        case '\'':
            state = C_STRING_Q;
            goto string;
        case '\"':
            /* strings/chars */
            state = C_STRING;
        string:
            p++;
        parse_string:
            while (*p != '\n') {
                if (*p == '\\') {
                    p++;
                    if (*p == '\n')
                        break;
                    p++;
                } else if ((*p == '\'' && state == C_STRING_Q) ||
                           (*p == '\"' && state == C_STRING)) {
                    p++;
                    state = 0;
                    break;
                } else {
                    p++;
                }
            }
            set_color(p_start, p - p_start, QE_STYLE_STRING);
            break;
        case '=':
            p++;
            /* exit type declaration */
            type_decl = 0;
            break;
        default:
            if ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') || 
                (c == '_')) {
                
                get_c_keyword(kbuf, sizeof(kbuf), &p);
                p1 = p;
                while (*p == ' ' || *p == '\t')
                    p++;
                if (strfind(c_keywords, kbuf, 0)) {
                    set_color(p_start, p1 - p_start, QE_STYLE_KEYWORD);
                } else
                if (strfind(c_types, kbuf, 0)) {
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

#define MAX_BUF_SIZE    512
#define MAX_STACK_SIZE  64

enum {
    INDENT_NORM,
    INDENT_FIND_EQ,
};

void do_c_comment(EditState *s)
{
    do_bol(s);
    do_char(s, '/');
    do_char(s, '/');
}

void do_c_comment_region(EditState *s)
{
    int col_num, p1, p2, tmp;
    eb_get_pos(s->b, &p1, &col_num, s->offset);
    eb_get_pos(s->b, &p2, &col_num, s->b->mark);

    if (p1 > p2) {
        tmp = p1;
        p1 = p2;
        p2 = tmp;
    }

    for (;p1 <= p2; p1++) {
        s->offset = eb_goto_pos(s->b, p1, 0);
        do_c_comment(s);
    }
    do_eol(s);
}

void do_c_electric(EditState *s, int key)
{
	do_char(s, key);
    if(key == '{') {
        do_char(s, '}');
        do_left_right(s, -1);
        return;
    }

    if(key == '(') {
        do_char(s, ')');
        do_left_right(s, -1);
        return;
    }
    do_indent_lastline(s);
    // XXX: this is cheating. If you take this redraw out
    // the end of the line can paint a funny colour.
    // this fixes it, but this is kind of a hammer.
    // do_refresh(s);
}

static int c_mode_probe(ModeProbeData *p)
{
    const char *r;

    //currently, only use the file extension
    r = extension(p->filename);
    if (*r) {
        if (strfind("|c|h|cxx|cpp|e|js|ts|jsx|tsx|json|cs|jav|java|tf|", r + 1, 1))
            return 100;
    }
    return 0;
}

int c_mode_init(EditState *s, ModeSavedData *saved_data)
{
    int ret;

    ret = text_mode_init(s, saved_data);
    if (ret)
        return ret;
    set_colorize_func(s, c_colorize_line);
    return ret;
}

void do_clangfmt(EditState *s) 
{
	const char *argv[4];
	
	// XXX: configure option? Scripting option?
    argv[0] = "clang-format";
    argv[1] = "-i";
    argv[2] = s->b->filename;
    argv[3] = NULL;

	run_system_cmd(s, argv);
}

/* specific C commands */
static CmdDef c_commands[] = {
    CMD0( KEY_META(';'), KEY_NONE, "c-comment", do_c_comment)
    CMD0( KEY_CTRLX(';'), KEY_NONE, "c-comment-region", do_c_comment_region)
    
	CMDV( '{', KEY_NONE, "c-electric-obrace", do_c_electric, '{', "*v")
	CMDV( '(', KEY_NONE, "c-electric-paren", do_c_electric, '(', "*v")
    CMDV( KEY_RET, KEY_NONE, "c-electric-newline", do_c_electric, '\n', "*v")
   
    CMD0( KEY_CTRLX('y'), KEY_NONE, "c-fmt", do_clangfmt)
    CMD_DEF_END,
};

static ModeDef c_mode;

int c_init(void)
{
    /* c mode is almost like the text mode, so we copy and patch it */
    memcpy(&c_mode, &text_mode, sizeof(ModeDef));
    
    c_mode.name = "C";
    c_mode.mode_probe = c_mode_probe;
    c_mode.mode_init = c_mode_init;

    qe_register_mode(&c_mode);

    qe_register_cmd_table(c_commands, "C");

    return 0;
}

