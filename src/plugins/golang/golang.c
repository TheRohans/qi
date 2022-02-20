/**
 * Golang mode for Qi
 * Copyright (c) 2022, Rob Rohan
 * Copyright (c) 2001, 2002 Fabrice Bellard.
 */
#include "qe.h"
#include "../clang/clang.h"

static const char go_keywords[] = 
"|break|default|func|interface|select|case|defer|go|map|struct|"
"chan|else|goto|package|switch|const|fallthrough|if|range|type|"
"continue|for|import|return|var|";

static const char go_types[] = 
"|string|bool|int8|uint8|byte|int16|uint16|int32|rune|uint32|"
"int64|uint64|int|uint|uintptr|float32|float64|complex64|complex128|";

/* colorization states */
enum {
    GO_COMMENT = 1,
    GO_STRING,
    GO_STRING_Q,
};

static int get_keyword(char *buf, int buf_size, unsigned int **pp)
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

void go_colorize_line(unsigned int *buf, int len, 
    int *colorize_state_ptr, int state_only)
{
    int c, state, type_decl;
    unsigned int *p, *p_start, *p1;
    char kbuf[32];

    state = *colorize_state_ptr;
    p = buf;
    p_start = p;
    type_decl = 0;
    c = 0;

    /* if already in a state, go directly in the code parsing it */
    switch (state) {
    case GO_COMMENT:
        goto parse_comment;
    case GO_STRING:
    case GO_STRING_Q:
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
        case '/':
            p++;
            if (*p == '*') {
                /* normal comment */
                p++;
                state = GO_COMMENT;
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
        case '\'':
            state = GO_STRING_Q;
            goto string;
        case '\"':
            /* strings/chars */
            state = GO_STRING;
string:
            p++;
parse_string:
            while (*p != '\n') {
                if (*p == '\\') {
                    p++;
                    if (*p == '\n')
                        break;
                    p++;
                } else if ((*p == '\'' && state == GO_STRING_Q) ||
                           (*p == '\"' && state == GO_STRING)) {
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
                
                get_keyword(kbuf, sizeof(kbuf), &p);
                p1 = p;
                while (*p == ' ' || *p == '\t')
                    p++;
                if (strfind(go_keywords, kbuf, 0)) {
                    set_color(p_start, p1 - p_start, QE_STYLE_KEYWORD);
                } else
                if (strfind(go_types, kbuf, 0)) {
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

static int go_mode_probe(ModeProbeData *p)
{
    const char *r;
    r = extension(p->filename);
    if (*r) {
        if (strfind("|go|", r + 1, 1))
            return 100;
    }
    return 0;
}

int go_mode_init(EditState *s, ModeSavedData *saved_data)
{
    int ret;

    ret = text_mode_init(s, saved_data);
    if (ret)
        return ret;
    set_colorize_func(s, go_colorize_line);
    return ret;
}

void do_gofmt(EditState *s) 
{
	const char *argv[4];
	
	// XXX: configure option? Scripting option?
    argv[0] = "gofmt";
    argv[1] = "-w";
    argv[2] = s->b->filename;
    argv[3] = NULL;

	run_system_cmd(s, argv);
}

static CmdDef go_commands[] = {
    CMD0( KEY_META(';'), KEY_NONE, "go-comment", do_c_comment)
    CMD0( KEY_CTRLX(';'), KEY_NONE, "go-comment-region", do_c_comment_region)
    
	CMDV( '{', KEY_NONE, "go-electric-obrace", do_c_electric, '{', "*v")
	CMDV( '(', KEY_NONE, "go-electric-paren", do_c_electric, '(', "*v")
    CMDV( KEY_RET, KEY_NONE, "go-electric-newline", do_c_electric, '\n', "*v")
    
    CMD0( KEY_CTRLX('y'), KEY_NONE, "go-fmt", do_gofmt)
    
    CMD_DEF_END,
};

static ModeDef go_mode;

int go_init(void)
{
    /* c mode is almost like the text mode, so we copy and patch it */
    memcpy(&go_mode, &text_mode, sizeof(ModeDef));
    
    go_mode.name = "Go";
    go_mode.mode_probe = go_mode_probe;
    go_mode.mode_init = go_mode_init;

    qe_register_mode(&go_mode);

    qe_register_cmd_table(go_commands, "Go");

    return 0;
}

