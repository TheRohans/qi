/*
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

/**
 * Look at the last line, and try to copy the indent
 * level of that line to the current line.
 */
void do_go_indent(EditState *s)
{
    int line_num, col_num, offset;

    // Find start of line
    eb_get_pos(s->b, &line_num, &col_num, s->offset);
    LOG("Current line: %d:%d", line_num, col_num);
    if((line_num - 1) <= 0) return;
    
    LOG("Current offset: %d", s->offset);
    int current_start_offset = s->offset;
    
    // Find the start of the last line
    int last_line_pos = eb_goto_pos(s->b, line_num - 1, 0);
    s->offset = last_line_pos;
    
    LOG("Last line offset: %d", s->offset);
    
    // find the number of spaces on the last line and assume that
    // the number of whitespaces is the indent level
    int pos = 0;
    int next_char;
    // (we are still focused on the last line)
    int offset1 = s->offset;
    for (;;) {
        int ch = eb_nextc(s->b, offset1, &next_char);
        if (ch != ' ' && ch != '\t')
            break;
        offset1 = next_char;
        pos++;
    }
    
    // move back to the start of the last line
    // since eb_nextc might have moved us
    s->offset = last_line_pos;
    
    // now get the previous lines indent as a string
    char *indent = calloc(sizeof(char), (pos + 1));
    eb_get_substr(s->b, indent, last_line_pos, pos+1);
    
    LOG("%d >%s<", pos, indent);
    
    // Now move back to the position we want to move
    // to and add in the indent whitespace
    s->offset = current_start_offset;
    if(pos > 0) {
        // make sure this is a null term string
        indent[pos] = '\0';
        eb_insert(s->b, s->offset, indent, pos);
    }
    s->offset = current_start_offset + pos;
    
    free(indent);
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

void do_go_electric(EditState *s, int key)
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
    
	do_go_indent(s);
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

static CmdDef go_commands[] = {
    CMD0( KEY_META(';'), KEY_NONE, "c-comment", do_c_comment)
    CMD0( KEY_CTRLX(';'), KEY_NONE, "c-comment-region", do_c_comment_region)
    
	CMDV( '{', KEY_NONE, "c-electric-obrace", do_go_electric, '{', "*v")
	CMDV( '(', KEY_NONE, "c-electric-paren", do_go_electric, '(', "*v")
	
    CMDV( KEY_RET, KEY_NONE, "go-electric-newline", do_go_electric, '\n', "*v")
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

