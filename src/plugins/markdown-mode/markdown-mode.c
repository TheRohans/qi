/*
 * Markdown mode for Qi
 * Copyright (c) 2013 Rob Rohan
 * Based on c-mode by Fabrice Bellard
 */
#include "qe.h"

#define MAX_BUF_SIZE    512

static ModeDef markdown_mode;

static void markdown_colorize_line(unsigned int *buf, int len, 
                                int *colorize_state_ptr, int state_only)
{
    int c, state;
    unsigned int *p, *p_start;

    state = *colorize_state_ptr;
    p = buf;
    p_start = p;

    for (;;) {
        p_start = p;
        c = *p;
        switch (c) {
        case '\n':
            goto the_end;
        case '=':
            p++;
            short eqdash = 0;
            while (*p != '\n') {
                p++;
                eqdash++;
                if(*p != '=') break;
            }
            if(eqdash > 1) {
                set_color(p_start, p - p_start, QE_STYLE_COMMENT);
            }
            break;
        case '#':
            p++;
            while (*p != '\n') {
                p++;
            }
            set_color(p_start, p - p_start, QE_STYLE_STRING);
            break;
        case '-':
            p++;
            short dash = 0;
            while (*p != '\n') {
                p++;
                dash++;
                if(*p != '-') break;
            }
            if(dash >= 3) {
                set_color(p_start, p - p_start, QE_STYLE_HIGHLIGHT);
            }
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
        case '_':
            //p++;
            while (*p != '\n') {
                p++;
                if(*p == '_') { 
                    p++;
                    break;
                }
            }
            set_color(p_start, p - p_start, QE_STYLE_KEYWORD);
            break;
        case '*':
            while (*p != '\n') {
                p++;
                if( (*p == ' ' || *p == '\n') && *(p-1) == '*') {
                    break;
                }
                if(*p == '*') { 
                    p++;
                    break;
                }
            }
            set_color(p_start, p - p_start, QE_STYLE_KEYWORD);
            break;
        default:
            p++;
            break;
        }
    }
 the_end:
    *colorize_state_ptr = state;
}

static int markdown_mode_probe(ModeProbeData *p)
{
    const char *r;

    //currently, only use the file extension
    r = extension(p->filename);
    if (*r) {
        if (strfind("|md|markdown|mdown|mkd|mdtext|mdtxt|mkdn|txt|text|", r + 1, 1))
            return 100;
    }
    return 0;
}

static int markdown_mode_init(EditState *s, ModeSavedData *saved_data)
{
    int ret;
    ret = text_mode_init(s, saved_data);
    if (ret)
        return ret;
    set_colorize_func(s, markdown_colorize_line);
    return ret;
}

// specific Markdown commands
static CmdDef markdown_commands[] = {
    CMD_DEF_END,
};

int markdown_init(void)
{
    // Markdown mode is almost like the text mode, so we copy and patch it
    memcpy(&markdown_mode, &text_mode, sizeof(ModeDef));
    markdown_mode.name = "Markdown";
    markdown_mode.mode_probe = markdown_mode_probe;
    markdown_mode.mode_init = markdown_mode_init;

    qe_register_mode(&markdown_mode);
    qe_register_cmd_table(markdown_commands, "Markdown");
    //register_completion("markdown", markdown_completion);

    return 0;
}

