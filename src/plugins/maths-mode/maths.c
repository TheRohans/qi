/*
 * Maths mode for Qi
 * Copyright (c) 2022 Rob Rohan
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

int maths_mode_init(EditState *s, ModeSavedData *saved_data)
{
    int ret;

    ret = text_mode_init(s, saved_data);
    if (ret)
        return ret;
    // set_colorize_func(s, c_colorize_line);
    
    return ret;
}

/* specific Maths commands */
static CmdDef maths_commands[] = {
    CMD1( KEY_META('s'), KEY_NONE, "math-sqrt", do_char,              L'√')
    CMD1( KEY_META('S'), KEY_NONE, "math-cube-root", do_char,         L'∛')
    CMD1( KEY_META('i'), KEY_NONE, "math-integral", do_char,          L'∫')
    CMD1( KEY_META('a'), KEY_NONE, "math-angle", do_char,             L'∠')
    CMD1( KEY_META('o'), KEY_NONE, "math-omega", do_char,             L'Ω')
    CMD1( KEY_META('p'), KEY_NONE, "math-pi", do_char,                L'π')
    CMD1( KEY_META('+'), KEY_NONE, "math-sigma", do_char,             L'∑')
    CMD1( KEY_META('8'), KEY_NONE, "math-infinity", do_char,          L'∞')
    CMD1( KEY_META('('), KEY_NONE, "math-epsilon-l", do_char,         L'ϵ')
    CMD1( KEY_META(')'), KEY_NONE, "math-epsilon-r", do_char,         L'϶')
    CMD1( KEY_META('t'), KEY_NONE, "math-theta", do_char,             L'ϴ')
    CMD1( KEY_META('T'), KEY_NONE, "math-therefore", do_char,         L'∴')
    CMD1( KEY_META('/'), KEY_NONE, "math-div", do_char,               L'÷')
    CMD1( KEY_NONE,      KEY_NONE, "math-cross", do_char,             L'⨯')
    CMD1( KEY_NONE,      KEY_NONE, "math-degree", do_char,            L'°')
    CMD1( KEY_META('*'), KEY_NONE, "math-dot", do_char,               L'∙')
    
    CMD1( KEY_META('1'), KEY_NONE, "math-sub-1", do_char,             L'₁')
    CMD1( KEY_META('2'), KEY_NONE, "math-sup-2", do_char,             L'²')
    CMD1( KEY_META('3'), KEY_NONE, "math-sup-3", do_char,             L'³')
    CMD1( KEY_META('4'), KEY_NONE, "math-sup-n", do_char,             L'ⁿ')
    
    CMD1( KEY_META('!'), KEY_NONE, "math-ceil-l", do_char,            L'⌈')
    CMD1( KEY_META('@'), KEY_NONE, "math-ceil-r", do_char,            L'⌉')
    CMD1( KEY_META('#'), KEY_NONE, "math-floor-l", do_char,           L'⌊')
    CMD1( KEY_META('$'), KEY_NONE, "math-floor-r", do_char,           L'⌋')
    
    CMD1( KEY_META('_'), KEY_NONE, "math-horz-bar", do_char,          L'―')
    
    // XXX: Doing this messes up the cursor position becuase we are not taking
    // combined chars into account when doing utf8 length
    CMD1( KEY_META('h'), KEY_NONE, "math-vector-arrow", do_char,      0x20D7)// x⃗
    // XXX: can not meta with arrow keys
    CMD1( KEY_META(KEY_UP), KEY_NONE, "math-up-arrow", do_char,       L'⭡')
    CMD1( KEY_META(KEY_DOWN), KEY_NONE, "math-down-arrow", do_char,   L'⭣')
    CMD1( KEY_META(KEY_LEFT), KEY_NONE, "math-left-arrow", do_char,   L'⭠')
    CMD1( KEY_META(KEY_RIGHT), KEY_NONE, "math-right-arrow", do_char, L'⭢')
    // Unused: ep\|jk;:'"cnm,.?1234567890^&-= 
    CMD_DEF_END,
};

static ModeDef maths_mode;

int maths_init(void)
{
    /* maths mode is almost like the text mode, so we copy and patch it */
    memcpy(&maths_mode, &text_mode, sizeof(ModeDef));
    
    maths_mode.name = "Maths";
    // maths_mode.mode_probe = maths_mode_probe;
    maths_mode.mode_init = maths_mode_init;

    qe_register_mode(&maths_mode);
    qe_register_cmd_table(maths_commands, "Maths");

    return 0;
}
