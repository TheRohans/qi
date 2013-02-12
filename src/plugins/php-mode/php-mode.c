/*
 * PHP mode for Qi
 * Copyright (c) 2013, The Rohans, LLC
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
#include "plugincore.h"

static const char php_keywords[] = 
"__halt_compiler|abstract|and|as|break|callable|case|catch|class|clone|const|continue|declare|default|die|do|echo|else|elseif|empty|enddeclare|endfor|endforeach|endif|endswitch|endwhile|eval|exit|extends|final|for|foreach|function|global|goto|if|implements|include|include_once|instanceof|insteadof|interface|isset|list|namespace|new|or|print|private|protected|public|require|require_once|return|static|switch|throw|trait|try|use|var|while|xor|";

//NOTE: 'var' is added for javascript
static const char php_types[] = 
"|int|integer|bool|boolean|float|double|real|string|array|object|unset|NULL|";

static int get_php_keyword(char *buf, int buf_size, unsigned int **pp)
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
    PHP_COMMENT = 1,
    PHP_STRING,
    PHP_STRING_Q,
	PHP_VARIABLE,
};

void php_colorize_line(unsigned int *buf, int len, 
                     int *colorize_state_ptr, int state_only)
{
    int c, state, l, type_decl;
    unsigned int *p, *p_start, *p1;
    char kbuf[32];

    state = *colorize_state_ptr;
    p = buf;
    p_start = p;
    type_decl = 0;
    c = 0;      // turn off stupid egcs-2.91.66 warning

    //if already in a state, go directly in the code parsing it
    switch (state) {
    case PHP_COMMENT:
        goto parse_comment;
    case PHP_STRING:
    case PHP_STRING_Q:
        goto parse_string;
	//case PHP_VARIABLE:
	//	goto parse_variable;
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
                // normal comment 
                p++;
                state = PHP_COMMENT;
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
                // line comment
                while (*p != '\n') 
                    p++;
                set_color(p_start, p - p_start, QE_STYLE_COMMENT);
            }
            break;
		//variables
		case '$':
			p++;
			while (*p != '\n') {
				//if (*p == '-' || *p == ' ' || *p == '\t' || *p == '=') {
				if ((*p >= 'a' && *p <= 'z') ||
				   (*p >= 'A' && *p <= 'Z') || 
				   (*p == '_')) {
					p++;
				} else {
					break;
				}
			}
			set_color(p_start, p - p_start, QE_STYLE_VARIABLE);
			//set_color(p_start, p - p_start, QE_STYLE_KEYWORD);
			break;
        case '\'':
            state = PHP_STRING_Q;
            goto string;
        case '\"':
            /* strings/chars */
            state = PHP_STRING;
        string:
            p++;
        parse_string:
            while (*p != '\n') {
                if (*p == '\\') {
                    p++;
                    if (*p == '\n')
                        break;
                    p++;
                } else if ((*p == '\'' && state == PHP_STRING_Q) ||
                           (*p == '\"' && state == PHP_STRING)) {
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
            // exit type declaration
            type_decl = 0;
            break;
        default:
            if ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') || 
                (c == '_')) {
                
                l = get_php_keyword(kbuf, sizeof(kbuf), &p);
                p1 = p;
                while (*p == ' ' || *p == '\t')
                    p++;
                if (strfind(php_keywords, kbuf, 0)) {
                    set_color(p_start, p1 - p_start, QE_STYLE_KEYWORD);
                } else
                if (strfind(php_types, kbuf, 0)) {
                    //php type
                    //if not cast, assume type declaration 
                    if (*p != ')') {
                        type_decl = 1;
                    }
                    set_color(p_start, p1 - p_start, QE_STYLE_TYPE);
                } else {
                    // assume typedef if starting at first column 
                    if (p_start == buf)
                        type_decl = 1;

                    if (type_decl) {
                        if (*p == '(') {
                            // function definition case
                            set_color(p_start, p1 - p_start, QE_STYLE_FUNCTION);
                            type_decl = 1;
                        } else if (p_start == buf) {
                            // assume type if first column
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

//static int php_mode_probe(ModeProbeData *p)
//{
//    const char *r;
//
//    //currently, only use the file extension
//    r = extension(p->filename);
//    if (*r) {
//        if (strfind("|c|e|h|js|cs|jav|java|cxx|cpp|groovy|", r + 1, 1))
//            return 100;
//    }
//    return 0;
//}

int php_mode_init(EditState *s, ModeSavedData *saved_data)
{
    int ret;

    ret = text_mode_init(s, saved_data);
    if (ret)
        return ret;
    set_colorize_func(s, php_colorize_line);
    return ret;
}

//specific PHP commands
static CmdDef php_commands[] = {
    CMD_( KEY_CTRL('i'), KEY_NONE, "php-indent-command", do_c_indent, "*")
    CMD_( KEY_NONE, KEY_NONE, "php-indent-region", do_c_indent_region, "*")
	CMDV( ':', KEY_NONE, "php-electric-colon", do_c_electric, ':', "*v")
	CMDV( '{', KEY_NONE, "php-electric-obrace", do_c_electric, '{', "*v")
	CMDV( '}', KEY_NONE, "php-electric-cbrace", do_c_electric, '}', "*v")
	CMDV( KEY_RET, KEY_NONE, "php-electric-newline", do_c_electric, '\n', "*v")
    CMD_DEF_END,
};

static ModeDef php_mode;

int php_init(void)
{
    /* php mode is almost like the text mode, so we copy and patch it */
    memcpy(&php_mode, &text_mode, sizeof(ModeDef));
    php_mode.name = "PHP";
    //php_mode.mode_probe = php_mode_probe;
    php_mode.mode_init = php_mode_init;

    qe_register_mode(&php_mode);
    qe_register_cmd_table(php_commands, "PHP");

    return 0;
}

