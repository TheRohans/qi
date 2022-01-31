/*
 * Markdown mode for Qi
 * Copyright (c) 2013 Rob Rohan
 * Based on c-mode by Fabrice Bellard
 */
#include "qe.h"

#define MAX_BUF_SIZE    512

static ModeDef config_mode;

static const char config_keywords[] = 
"";

static const char config_types[] = 
"";

static int get_config_keyword(char *buf, int buf_size, unsigned int **pp)
{
    unsigned int *p, c;
    char *q;

    p = *pp;
    c = *p;
    q = buf;
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') || 
        (c == '_') ||
        (c == '.') ||
        (c == '-')) {
        do {
            if ((q - buf) < buf_size - 1)
                *q++ = c;
            p++;
            c = *p;
        } while ((c >= 'a' && c <= 'z') ||
                 (c >= 'A' && c <= 'Z') ||
                 (c == '_') ||
                 (c == '.') ||
                 (c == '-') ||
                 (c >= '0' && c <= '9'));
    }
    *q = '\0';
    *pp = p;
    return q - buf;
}

enum {
    CONFIG_COMMENT = 1,
    CONFIG_STRING,
};

void config_colorize_line(unsigned int *buf, int len, 
                                int *colorize_state_ptr, int state_only)
{
    int c, state, l;
    unsigned int *p, *p_start, *p1;
    char kbuf[32];
    
    state = *colorize_state_ptr;
    p = buf;
    p_start = p;

    // if already in a state, go directly in the code parsing it
    switch (state) {
    case CONFIG_COMMENT:
        // goto parse_comment;
    case CONFIG_STRING:
        //goto parse_string;
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
            p++;
            while (*p != '\n') {
                p++;
                // if(*p == ' ' || *p == ';' || *p == '}' || *p == '[') break;
            }
            
            set_color(p_start, p - p_start, QE_STYLE_COMMENT);
            break;
            
        case '-':
            p++;
            short dash = 0;
            while (*p != '\n') {
                p++;
                dash++;
                if(*p != '-') break;
            }
            if(dash >= 2) {
                set_color(p_start, p - p_start, QE_STYLE_HIGHLIGHT);
            }
            break;
 
//        case ' ':
//            p++;
//            set_color(p_start, p - p_start, QE_STYLE_HIGHLIGHT);
//            break;
        case '\t':
            p++;
            set_color(p_start, p - p_start, QE_STYLE_HIGHLIGHT);
            break;
            
        case '[':
            p++;
            while (*p != '\n') {
                p++;
                if(*p == ']') {
                    p++;
                    break;
                }
            }
            set_color(p_start, p - p_start, QE_STYLE_TYPE);
            break;            
        
        case '"':
            p++;
            while (*p != '\n') {
                p++;
                if(*p == '"') break;
            }
            set_color(p_start, p - p_start, QE_STYLE_STRING);
            break;
            
        case '\'':
            p++;
            while (*p != '\n') {
                p++;
                if(*p == '\'') break;
            }
            set_color(p_start, p - p_start, QE_STYLE_STRING);
            break;

        default:
            if ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') || 
                (c == '_')) {
                
                l = get_config_keyword(kbuf, sizeof(kbuf), &p);
                p1 = p;
                while (*p == ' ' || *p == '\t')
                    p++;
                if (strfind(config_keywords, kbuf, 0)) {
                    set_color(p_start, p1 - p_start, QE_STYLE_KEYWORD);
                } else
                if (strfind(config_types, kbuf, 0)) {
                    set_color(p_start, p1 - p_start, QE_STYLE_TYPE);
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

static int config_mode_probe(ModeProbeData *p)
{
    const char *r;

    //currently, only use the file extension
    r = extension(p->filename);
    if (*r) {
        if (strfind("|yaml|yml|", r + 1, 1))
            return 100;
    }
    return 0;
}

static int config_mode_init(EditState *s, ModeSavedData *saved_data)
{
    int ret;
    ret = text_mode_init(s, saved_data);
    if (ret)
        return ret;
    set_colorize_func(s, config_colorize_line);
    return ret;
}

static CmdDef config_commands[] = {
    CMD_DEF_END,
};

int config_init(void)
{
    memcpy(&config_mode, &text_mode, sizeof(ModeDef));
    config_mode.name = "CONFIG";
    config_mode.mode_probe = config_mode_probe;
    config_mode.mode_init = config_mode_init;

    qe_register_mode(&config_mode);
    qe_register_cmd_table(config_commands, "CONFIG");

    return 0;
}

