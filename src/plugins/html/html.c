/*
 * HTML text mode for Qi
 * Copyright (c) 2013 The Rohans, LLC
 * Copyright (c) 2002 Fabrice Bellard.
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
// Requires ts for javascript coloring, and css for css coloring
#include "plugincore.h"

/* colorization states */
enum {
    HTML_TAG = 1,
    HTML_COMMENT, 
    HTML_TAG_SCRIPT,
    HTML_TAG_STYLE,
    HTML_STYLE,         // mode for inside a style, colored with css mode 
    HTML_SCRIPT = 0x10, // mode for inside a script, colored with ts mode 
};

void html_colorize_line(unsigned int *buf, int len, 
                       int *colorize_state_ptr, int state_only)
{
    int c, state;
    unsigned int *p, *p_start, *p1;
    
    state = *colorize_state_ptr;
    p = buf;
    p_start = p;

    // if already in a state, go directly in the code parsing it
    if (state & HTML_SCRIPT)
        goto parse_script;
	
    switch (state) {
    case HTML_COMMENT:
        goto parse_comment;
    case HTML_TAG:
    case HTML_TAG_SCRIPT:
        goto parse_tag;
    case HTML_STYLE:
        goto parse_style;
    default:
        break;
    }

    for (;;) {
        p_start = p;
        c = *p;
        if (c == '\n') {
            goto the_end;
        } else if (c == '<' && state == 0) {
            p++;
            if (p[0] == '!' && p[1] == '-' && p[2] == '-') {
                p += 3;
                state = HTML_COMMENT;
                // wait until end of comment
parse_comment:
                while (*p != '\n') {
                    if (p[0] == '-' && p[1] == '-' && p[2] == '>') {
                        p += 3;
                        state = 0;
                        break;
                    } else {
                        p++;
                    }
                }
                set_color(p_start, p - p_start, QE_STYLE_COMMENT);
            } else {
                //we are in a tag
                if (ustristart(p, "SCRIPT", (const unsigned int **)&p)) {
                    state = HTML_TAG_SCRIPT;
                } else if (ustristart(p, "STYLE", (const unsigned int **)&p)) {
                    state = HTML_TAG_STYLE;
				} else {
                    state = HTML_TAG;
                }
parse_tag:
                //set_color(p_start, p - p_start, QE_STYLE_TAG);
                while (*p != '\n') {
                    //if we hit the end of the tag (php ends at a space)
                    if (*p == '>') {
                        p++;
                        if (state == HTML_TAG_SCRIPT)
                            state = HTML_SCRIPT;
                        else if (state == HTML_TAG_STYLE)
                            state = HTML_STYLE;
                        else
                            state = 0;
                        break;
                    } else {
                        //state = HTML_TAG;
                        p++;
                    }
                }
                set_color(p_start, p - p_start, QE_STYLE_TAG);
                
                if (state == HTML_SCRIPT) {
                    // javascript coloring
                    p_start = p;
parse_script:
                    for (;;) {
                        if (*p == '\n') {
                            state &= ~HTML_SCRIPT;
                            ts_colorize_line(p_start, p - p_start, &state, state_only);
                            state |= HTML_SCRIPT;
                            break;
                        } else if (ustristart(p, "</SCRIPT", (const unsigned int **)&p1)) {
                            while (*p1 != '\n' && *p1 != '>') 
                                p1++;
                            if (*p1 == '>')
                                p1++;
                            state &= ~HTML_SCRIPT;
                            ts_colorize_line(p_start, p - p_start, &state, state_only);
                            state |= HTML_SCRIPT;
                            set_color(p, p1 - p, QE_STYLE_TAG);
                            p = p1;
                            state = 0;
                            break;
                        } else {
                            p++;
                        }
                    }
				} else if (state == HTML_STYLE) {
                    //stylesheet coloring
                    p_start = p;
parse_style:
                    for (;;) {
                        if (*p == '\n') {
                            css_colorize_line(p_start, p - p_start, &state, state_only);
                            break;
                        } else if (ustristart(p, "</STYLE", (const unsigned int **)&p1)) {
                            while (*p1 != '\n' && *p1 != '>') 
                                p1++;
                            if (*p1 == '>')
                                p1++;
                            set_color(p_start, p - p_start, QE_STYLE_CSS);
                            set_color(p, p1 - p, QE_STYLE_TAG);
                            p = p1;
                            state = 0;
                            break;
                        } else {
                            p++;
                        }
                    }
                }
            }
        } else {
            //text
            p++;
        }
    }
the_end:
    *colorize_state_ptr = state;
}

int html_mode_probe(ModeProbeData *p)
{
    const char *r;
    r = extension(p->filename);
    if (*r) {
        if (strfind("|html|xhtml|htm|", r + 1, 1))
            return 100;
    }
    return 0;
}

int html_mode_init(EditState *s, ModeSavedData *saved_data)
{
    int ret;
    ret = text_mode_init(s, saved_data);
    if (ret)
        return ret;
    set_colorize_func(s, html_colorize_line);
    return 0;
}

//specific PHP commands
static CmdDef html_commands[] = {
    // CMD_( KEY_CTRL('i'), KEY_NONE, "html-indent-command", do_c_indent, "*")
    // CMD_( KEY_NONE, KEY_NONE, "html-indent-region", do_c_indent_region, "*")
	// CMDV( ':', KEY_NONE, "html-electric-colon", do_c_electric, ':', "*v")
	// CMDV( '{', KEY_NONE, "html-electric-obrace", do_c_electric, '{', "*v")
	// CMDV( '}', KEY_NONE, "html-electric-cbrace", do_c_electric, '}', "*v")
	
	CMDV( KEY_RET, KEY_NONE, "html-electric-newline", do_c_electric, '\n', "*v")
    CMD_DEF_END,
};


static ModeDef html_mode;

int html_init(void)
{
    memcpy(&html_mode, &text_mode, sizeof(ModeDef));
    html_mode.name = "HTML";
    html_mode.mode_probe = html_mode_probe;
    html_mode.mode_init = html_mode_init;

    qe_register_mode(&html_mode);
    qe_register_cmd_table(html_commands, "HTML");
    return 0;
}
