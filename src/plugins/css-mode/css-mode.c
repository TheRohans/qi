/*
 * Markdown mode for Qi
 * Copyright (c) 2013 Rob Rohan
 * Based on c-mode by Fabrice Bellard
 */
#include "qe.h"

#define MAX_BUF_SIZE    512

static ModeDef css_mode;

static const char css_keywords[] = 
"|inline|block|none|black|white|red|green|blue|yellow|px|em|left|right|center|print|screen|important|no-repeat|";

static const char css_types[] = 
"|azimuth|background-attachment|background-color|background-image|background-position|background-repeat|"
"background|border-collapse|border-color|border-spacing|border-style|border-top|border-right|border-bottom|"
"border-left|border-top-color|border-right-color|border-bottom-color|border-left-color|border-top-style|border-right-style|"
"border-bottom-style|border-left-style|border-top-width|border-right-width|border-bottom-width|border-left-width|border-width|"
"border|bottom|caption-side|clear|clip|color|content|counter-increment|counter-reset|cue-after|cue-before|cue|cursor|"
"direction|display|elevation|empty-cells|float|font-family|font-size|font-style|font-variant|font-weight|font|height|"
"left|letter-spacing|line-height|list-style-image|list-style-position|list-style-type|list-style|margin-right|margin-left|"
"margin-top|margin-bottom|margin|max-height|max-width|min-height|min-width|orphans|outline-color|"
"outline-style|outline-width|outline|overflow|padding-top|padding-right|padding-bottom|padding-left|padding|"
"page-break-after|page-break-before|page-break-inside|pause-after|pause-before|pause|pitch-range|pitch|play-during|"
"position|quotes|richness|right|speak-header|speak-numeral|speak-punctuation|speak|speech-rate|stress|table-layout|"
"text-align|text-decoration|text-indent|text-transform|top|unicode-bidi|vertical-align|visibility|voice-family|volume|"
"white-space|widows|width|word-spacing|z-index|";

static int get_css_keyword(char *buf, int buf_size, unsigned int **pp)
{
    unsigned int *p, c;
    char *q;

    p = *pp;
    c = *p;
    q = buf;
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') || 
        (c == '_') ||
        (c == '-')) {
        do {
            if ((q - buf) < buf_size - 1)
                *q++ = c;
            p++;
            c = *p;
        } while ((c >= 'a' && c <= 'z') ||
                 (c >= 'A' && c <= 'Z') ||
                 (c == '_') ||
                 (c == '-') ||
                 (c >= '0' && c <= '9'));
    }
    *q = '\0';
    *pp = p;
    return q - buf;
}

enum {
    CSS_COMMENT = 1,
    CSS_STRING,
};

// TODO: add state handling to allow colorization of elements longer
// than one line (eg, multi-line functions and strings)
void css_colorize_line(unsigned int *buf, int len, 
                                int *colorize_state_ptr, int state_only)
{
    int c, state;
    unsigned int *p, *p_start, *p1;
    char kbuf[32];
    
    state = *colorize_state_ptr;
    p = buf;
    p_start = p;

    // if already in a state, go directly in the code parsing it
    switch (state) {
    case CSS_COMMENT:
        goto parse_comment;
    case CSS_STRING:
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
        case '/':
            p++;
            if (*p == '*') {
                //normal comment
                p++;
                state = CSS_COMMENT;
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
            }
            break;

        case '#':
            p++;
            while (*p != '\n') {
                p++;
                if(*p == ' ' || *p == ';' || *p == '}' || *p == '[') break;
            }
            set_color(p_start, p - p_start, QE_STYLE_TYPE);
            break;

        case '.':
            p++;
            if( (*p >= '0' && *p <= '9') ) {
                break;
            }
            //first character is not a number
            while (*p != '\n') {
                p++;
                if(*p == ' ' || *p == ';' || *p == '}' || *p == '[') break;                
            }
            set_color(p_start, p - p_start, QE_STYLE_VARIABLE);                
            break;

        case '(':
            while (*p != '\n') {
                p++;
                if(*p == ')') {
                    p++;
                    break;
                }
                
            }
            set_color(p_start, p - p_start, QE_STYLE_STRING);
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
                
                get_css_keyword(kbuf, sizeof(kbuf), &p);
                p1 = p;
                while (*p == ' ' || *p == '\t')
                    p++;
                if (strfind(css_keywords, kbuf, 0)) {
                    set_color(p_start, p1 - p_start, QE_STYLE_KEYWORD);
                } else
                if (strfind(css_types, kbuf, 0)) {
                    // css type 
                    // if not cast, assume type declaration
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

static int css_mode_probe(ModeProbeData *p)
{
    const char *r;

    //currently, only use the file extension
    r = extension(p->filename);
    if (*r) {
        if (strfind("|css|", r + 1, 1))
            return 100;
    }
    return 0;
}

static int css_mode_init(EditState *s, ModeSavedData *saved_data)
{
    int ret;
    ret = text_mode_init(s, saved_data);
    if (ret)
        return ret;
    set_colorize_func(s, css_colorize_line);
    return ret;
}

// specific Markdown commands
static CmdDef css_commands[] = {
    CMD_DEF_END,
};

int css_init(void)
{
    memcpy(&css_mode, &text_mode, sizeof(ModeDef));
    css_mode.name = "CSS";
    css_mode.mode_probe = css_mode_probe;
    css_mode.mode_init = css_mode_init;

    qe_register_mode(&css_mode);
    qe_register_cmd_table(css_commands, "CSS");

    return 0;
}

