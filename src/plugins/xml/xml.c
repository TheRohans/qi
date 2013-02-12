/*
 * XML text mode for QEmacs.
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

/* colorization states */
enum {
    XML_TAG = 1,
    XML_COMMENT, 
};

void xml_colorize_line(unsigned int *buf, int len, 
                       int *colorize_state_ptr, int state_only)
{
    int c, state;
    unsigned int *p, *p_start, *p1;
    
    state = *colorize_state_ptr;
    p = buf;
    p_start = p;

    //if already in a state, go directly in the code parsing it
    switch (state) {
    case XML_COMMENT:
        goto parse_comment;
    case XML_TAG:
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
                state = XML_COMMENT;
                //wait until end of comment
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
            }
            parse_tag:
                while (*p != '\n') {
                    if (*p == '>') {
                        p++;
						state = 0;
						break;
                    } else {
                        p++;
                    }
                }
                set_color(p_start, p - p_start, QE_STYLE_TAG);
        } else {
            //text
            p++;
        }
    }
 the_end:
    *colorize_state_ptr = state;
}

int xml_mode_probe(ModeProbeData *p)
{
    /* const char *p;

    p = (const char *)p1->buf;
    while (css_is_space(*p))
        p++;
    if (*p != '<')
        return 0;
    p++;
    if (*p != '!' && *p != '?' && *p != '/' && 
        !isalpha(*p))
        return 0;
    return 90; //leave some room for more specific XML parser 
	*/
	
    const char *r;

    //currently, only use the file extension
    r = extension(p->filename);
    if (*r) {
        if (strfind("|xml|project|classpath|config|lzx|plist|xsd|xmi|wsdl|rss|resx|rng|ixml|wsdd|gladep|glade|wddx|xsl|xslt|xdf|mxml|", r + 1, 1))
            return 100;
    }
    return 0;
	
}

int xml_mode_init(EditState *s, ModeSavedData *saved_data)
{
    int ret;
    ret = text_mode_init(s, saved_data);
    if (ret)
        return ret;
    set_colorize_func(s, xml_colorize_line);
    return 0;
}

static ModeDef xml_mode;

int xml_init(void)
{
    /* c mode is almost like the text mode, so we copy and patch it */
    memcpy(&xml_mode, &text_mode, sizeof(ModeDef));
    xml_mode.name = "xml";
    xml_mode.mode_probe = xml_mode_probe;
    xml_mode.mode_init = xml_mode_init;

    qe_register_mode(&xml_mode);

    return 0;
}

