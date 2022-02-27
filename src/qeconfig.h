/**
 * default qi configuration 
 */
CmdDef basic_commands[] = {
    CMDV( KEY_DEFAULT, KEY_NONE, "self-insert-command", do_char, ' ', "*v")

	CMD_( KEY_CTRL('o'), KEY_NONE, "open-line", do_open_line, "*")
    CMD1( KEY_CTRL('p'), KEY_UP, "previous-line", do_up_down, -1 )
    CMD1( KEY_CTRL('n'), KEY_DOWN, "next-line", do_up_down, 1 )
    CMD1( KEY_CTRL('b'), KEY_LEFT, "backward-char", do_left_right, -1 )
    CMD1( KEY_CTRL('f'), KEY_RIGHT, "forward-char", do_left_right, 1 )
    CMD1( KEY_META('b'), KEY_CTRL_LEFT, "backward-word", do_word_right, -1 )
    CMD1( KEY_META('f'), KEY_CTRL_RIGHT, "forward-word", do_word_right, 1 )
    CMD1( KEY_META('v'), KEY_PAGEUP, "scroll-down", do_scroll_up_down, -2 )
    CMD1( KEY_CTRL('v'), KEY_PAGEDOWN, "scroll-up", do_scroll_up_down, 2 )
    CMD1( KEY_META('z'), KEY_NONE, "scroll-down-one", do_scroll_up_down, -1 )
    CMD1( KEY_CTRL('z'), KEY_NONE, "scroll-up-one", do_scroll_up_down, 1 )
    CMD0( KEY_HOME, KEY_CTRL('a'), "beginning-of-line", do_bol)
    CMD0( KEY_END, KEY_CTRL('e'), "end-of-line", do_eol)
	
    CMD0( KEY_INSERT, KEY_NONE, "overwrite-mode", do_insert)
    // deletion commands are allowed in read only buffers,
    // they will merely copy the data to the kill ring
    CMD0( KEY_CTRL('d'), KEY_DELETE, "delete-char", do_delete_char)
    CMD0( 127, KEY_NONE, "backward-delete-char", do_backspace)
    CMD1( KEY_META(KEY_DEL) , KEY_META(KEY_BS), "backward-delete-word", do_delete_word, -1)
    CMD1( KEY_META('d') , KEY_NONE, "delete-word", do_delete_word, 1)
	
    CMD1( KEY_CTRL('k'), KEY_NONE, "kill-line", do_kill_region, 2 )
    CMD0( KEY_CTRL('@'), KEY_NONE, "set-mark-command", do_set_mark )
	CMD0( KEY_CTRL('l'), KEY_NONE, "center-line", do_center_cursor )
    CMD1( KEY_CTRL('w'), KEY_NONE, "kill-region", do_kill_region, 1 )
    CMD1( KEY_META('w'), KEY_NONE, "copy-region", do_kill_region, 0 )
	
    CMD0( KEY_META('<'), KEY_CTRL_HOME, "beginning-of-buffer", do_bof )
    CMD0( KEY_META('>'), KEY_CTRL_END, "end-of-buffer", do_eof )
	
    CMD_( KEY_META('x'), KEY_NONE, "execute-extended-command", do_execute_command, "s{Command: }[command]|command|i")
	
    CMD0( KEY_CTRL('u'), KEY_NONE, "universal-argument", do_universal_argument)
	
    CMD_( KEY_CTRL('y'), KEY_NONE, "yank", do_yank, "*")
    CMD_( KEY_META('y'), KEY_NONE, "yank-pop", do_yank_pop, "*")

    ////////////////////////////////////////    
    // Maths
    CMD1( KEY_META('s'), KEY_NONE, "math-sqrt", do_char,              0x221a)// √
    CMD1( KEY_META('S'), KEY_NONE, "math-cube-root", do_char,         0x221b)// ∛
    CMD1( KEY_META('i'), KEY_NONE, "math-integral", do_char,          0x222b)// ∫
    CMD1( KEY_META('a'), KEY_NONE, "math-angle", do_char,             0x2220)// ∠
    CMD1( KEY_META('o'), KEY_NONE, "math-omega", do_char,             0x2126)// Ω
    CMD1( KEY_META('p'), KEY_NONE, "math-pi", do_char,                0x03c0)// π
    CMD1( KEY_META('+'), KEY_NONE, "math-sigma", do_char,             0x2211)// ∑
    CMD1( KEY_META('8'), KEY_NONE, "math-infinity", do_char,          0x221e)// ∞
    CMD1( KEY_META('('), KEY_NONE, "math-epsilon-l", do_char,         0x03f5)// ϵ
    CMD1( KEY_META(')'), KEY_NONE, "math-epsilon-r", do_char,         0x03f6)// ϶
    CMD1( KEY_META('t'), KEY_NONE, "math-theta", do_char,             0x03f4)// ϴ
    CMD1( KEY_META('T'), KEY_NONE, "math-therefore", do_char,         0x2234)// ∴
    CMD1( KEY_META('/'), KEY_NONE, "math-div", do_char,               0x00F7)// ÷
    CMD1( KEY_NONE,      KEY_NONE, "math-mul", do_char,               0x00D7)// ⨯
    CMD1( KEY_NONE,      KEY_NONE, "math-degree", do_char,            0x00B0)// °
    CMD1( KEY_META('*'), KEY_NONE, "math-dot", do_char,               0x2219)// ∙
    
    CMD1( KEY_META('1'), KEY_NONE, "math-sub-1", do_char,             0x2081)// ₁ 
    CMD1( KEY_META('2'), KEY_NONE, "math-sup-2", do_char,             0x00B2)// ² 
    CMD1( KEY_META('3'), KEY_NONE, "math-sup-3", do_char,             0x00B3)// ³ 
    CMD1( KEY_META('4'), KEY_NONE, "math-sup-n", do_char,             0x207f)// ⁿ
    
    CMD1( KEY_META('!'), KEY_NONE, "math-ceil-l", do_char,            0x2308)// ⌈
    CMD1( KEY_META('@'), KEY_NONE, "math-ceil-r", do_char,            0x2309)// ⌉
    CMD1( KEY_META('#'), KEY_NONE, "math-floor-l", do_char,           0x230a)// ⌊
    CMD1( KEY_META('$'), KEY_NONE, "math-floor-r", do_char,           0x230b)// ⌋
    
    CMD1( KEY_META('_'), KEY_NONE, "math-horz-bar", do_char,          0x2015)// ―
    
    // XXX: Doing this messes up the cursor position becuase we are not taking
    // combined char into account when doing length
    CMD1( KEY_META('h'), KEY_NONE, "math-vector-arrow", do_char,      0x20D7)// x⃗
    // XXX: can not meta with arrow keys
    CMD1( KEY_META(KEY_UP), KEY_NONE, "math-up-arrow", do_char,       0x2b61)// ⭡
    CMD1( KEY_META(KEY_DOWN), KEY_NONE, "math-down-arrow", do_char,   0x2b63)// ⭣
    CMD1( KEY_META(KEY_LEFT), KEY_NONE, "math-left-arrow", do_char,   0x2b60)// ⭠
    CMD1( KEY_META(KEY_RIGHT), KEY_NONE, "math-right-arrow", do_char, 0x2b62)// ⭢
    // Unused: ep\|jk;:'"cnm,.?1234567890^&-= 
    ////////////////////////////////////////
    
	// do_tab will not change read only buffer
    CMD0( KEY_CTRL('i'), KEY_NONE, "tabulate", do_tab)
    
	CMD1( KEY_CTRLX(KEY_CTRL('s')), KEY_NONE, "save-buffer", do_save, 0 )
    CMD1( KEY_CTRLX(KEY_CTRL('w')), KEY_NONE, "write-file", do_save, 1 )
    CMD0( KEY_CTRLX(KEY_CTRL('c')), KEY_NONE, "exit-qi", do_quit )
	
    CMD_( KEY_CTRLX(KEY_CTRL('f')), KEY_NONE, "find-file", do_load, "s{Find file: }[file]|file|")
    CMD_( KEY_CTRLX(KEY_CTRL('v')), KEY_NONE, "find-alternate-file", do_find_alternate_file, "s{Find alternate file: }[file]|file|")
	
    CMD_( KEY_CTRLX('b'), KEY_NONE, "switch-to-buffer", do_switch_to_buffer, "s{Switch to buffer: }[buffer]|buffer|")
    CMD_( KEY_CTRLX('k'), KEY_NONE, "kill-buffer", do_kill_buffer, "s{Kill buffer: }[buffer]|buffer|")
    CMD_( KEY_CTRLX('i'), KEY_NONE, "insert-file", do_insert_file, "*s{Insert file: }[file]|file|")
	
    CMD0( KEY_CTRL('g'), KEY_CTRLX(KEY_CTRL('g')), "abort", do_break)
    
	CMDV( KEY_NONE, KEY_NONE, "search-forward", do_search_string, 1, "s{Search forward: }|search|v")
    CMDV( KEY_NONE, KEY_NONE, "search-backward", do_search_string, -1, "s{Search backward: }|search|v")
	
    CMD1( KEY_CTRL('s'), KEY_NONE, "isearch-forward", do_isearch, 1 )
    CMD1( KEY_CTRL('r'), KEY_NONE, "isearch-backward", do_isearch, -1 )
	
	CMD_( KEY_META('%'), KEY_NONE, "query-replace", do_query_replace, "*s{Query replace: }|search|s{With: }|replace|")
    CMD_( KEY_META('r'), KEY_NONE, "replace-string", do_replace_string, "*s{Replace String: }|search|s{With: }|replace|")
	
    CMD0( KEY_CTRLX('u'), KEY_CTRL('_'), "undo", do_undo)
    
	CMD_( KEY_RET, KEY_NONE, "newline", do_return, "*")
    CMD0( KEY_CTRL('t'), KEY_NONE, "refresh", do_refresh_complete)
    
	// CG: should take a string if no numeric argument given
    CMD_( KEY_META('g'), KEY_NONE, "goto-line", do_goto_line, "i{Goto line: }")
    CMD_( KEY_NONE, KEY_NONE, "goto-char", do_goto_char, "i{Goto char: }")
    
	CMD0( KEY_CTRLX(KEY_CTRL('q')), KEY_NONE, "vc-toggle-read-only", do_toggle_read_only)
    
	CMD0( KEY_META('~'), KEY_NONE, "not-modified", do_not_modified)
    CMD_( KEY_META('q'), KEY_NONE, "fill-paragraph", do_fill_paragraph, "*")
    
	CMD0( KEY_META('{'), KEY_NONE, "backward-paragraph", do_backward_paragraph)
    CMD0( KEY_META('}'), KEY_NONE, "forward-paragraph", do_forward_paragraph)
	
    CMD0( KEY_CTRLX(KEY_CTRL('x')), KEY_NONE, "exchange-point-and-mark", do_exchange_point_and_mark)
	
    CMDV( KEY_META('l'), KEY_NONE, "downcase-word", do_changecase_word, 0, "*v")
    CMDV( KEY_META('u'), KEY_NONE, "upcase-word", do_changecase_word, 1, "*v")
	
	// These are too dangerous for me :-/
	// need to highlight selected region first
    // CMDV( KEY_CTRLX(KEY_CTRL('l')), KEY_NONE, "downcase-region", do_changecase_region, 0, "*v")
    // CMDV( KEY_CTRLX(KEY_CTRL('u')), KEY_NONE, "upcase-region", do_changecase_region, 1, "*v")

    // keyboard macros
    // CMD_( KEY_NONE, KEY_NONE, "global-set-key", 
	//	 do_global_set_key, "s{Set key globally: }[key]s{command: }[command]|command|")

	// external commands
    CMD0( KEY_CTRLX('.'), KEY_NONE, "spelling", do_spell_check)
	
    // window handling
    CMD0( KEY_CTRLX('o'), KEY_NONE, "other-window", do_other_window)
    CMD0( KEY_CTRLX('n'), KEY_NONE, "next-window", do_other_window)
    CMD0( KEY_CTRLX('p'), KEY_NONE, "previous-window", do_previous_window)
#ifndef CONFIG_TINY
    CMD1( KEY_CTRL('x'), KEY_UP, "find-window-up", do_find_window, KEY_UP)
    CMD1( KEY_CTRL('x'), KEY_DOWN, "find-window-down", do_find_window, KEY_DOWN)
    CMD1( KEY_CTRL('x'), KEY_LEFT, "find-window-left", do_find_window, KEY_LEFT)
    CMD1( KEY_CTRL('x'), KEY_RIGHT, "find-window-right", do_find_window, KEY_RIGHT)
#endif
    CMD1( KEY_CTRLX('0'), KEY_NONE, "delete-window", do_delete_window, 0)
    CMD0( KEY_CTRLX('1'), KEY_NONE, "delete-other-windows", do_delete_other_windows)
    CMD1( KEY_CTRLX('2'), KEY_NONE, "split-window-vertically", do_split_window, 0)
    CMD1( KEY_CTRLX('3'), KEY_NONE, "split-window-horizontally", do_split_window, 1)
    
    // help
	CMD0( KEY_CTRLX('?'), KEY_F1, "help-for-help", do_help_for_help)
	CMD0( KEY_NONE, KEY_NONE, "describe-bindings", do_describe_bindings)
	CMD0( KEY_NONE, KEY_NONE, "describe-key-briefly", do_describe_key_briefly)
    CMD0( KEY_CTRL('h'), KEY_NONE, "backward-delete-char", do_backspace)

    // styles & display
    // CMD0( KEY_CTRLX('f'), KEY_NONE, "toggle-full-screen", do_toggle_full_screen)
    CMD0( KEY_NONE, KEY_NONE, "toggle-mode-line", do_toggle_mode_line)
	CMD0( KEY_NONE, KEY_NONE, "revert-buffer", do_revert_buffer)

    // config files
    CMD_( KEY_NONE, KEY_NONE, "load-file-from-path", do_load_file_from_path, "s{Load file from path: }|file|")
    CMD_( KEY_NONE, KEY_NONE, "parse-config-file",  parse_config, "s{Configuration file: }[file]|file|")
	
    CMD_( KEY_NONE, KEY_NONE, "load-qirc", do_load_qirc, "s{path: }[file]|file|")
    CMD_( KEY_NONE, KEY_NONE, "set-visited-file-name", do_set_visited_file_name, "s{Set visited file name: }[file]|file|s{Rename file? }")
    
    // non standard mappings
    CMD0( KEY_CTRLXRET('l'), KEY_NONE, "toggle-line-numbers", do_toggle_line_numbers)
    CMD0( KEY_CTRLXRET('t'), KEY_NONE, "toggle-truncate-lines", do_toggle_truncate_lines)
    CMD0( KEY_CTRLXRET('w'), KEY_NONE, "word-wrap", do_word_wrap)
    //CMD_( KEY_NONE, KEY_NONE, "set-emulation", do_set_emulation, "s{Emulation mode: }")
    CMD_( KEY_NONE, KEY_NONE, "cd", do_cd, "s{Change default directory: }[file]|file|")
    CMD_( KEY_NONE, KEY_NONE, "set-mode", do_cmd_set_mode, "s{Set mode: }[mode]")
    CMD0( KEY_CTRLX('h'), KEY_NONE, "mark-whole-buffer", do_mark_whole_buffer)
    CMD0( KEY_CTRLX('l'), KEY_NONE, "count-lines", do_count_lines)
    CMD0( KEY_CTRLX('='), KEY_NONE, "what-cursor-position", do_what_cursor_position)
    
    // tab & indent
    CMD_( KEY_NONE, KEY_NONE, "set-tab-width", do_set_tab_width, "i{Tab width: }")
    CMD_( KEY_NONE, KEY_NONE, "set-indent-width", do_set_indent_width, "i{Indent width: }")
    CMD_( KEY_NONE, KEY_NONE, "set-indent-tabs-mode", do_set_indent_tabs_mode, "i{Indent tabs mode (0 or 1): }")
    CMD_DEF_END,
};

CmdDef minibuffer_commands[] = {
    CMD1( KEY_RET, KEY_NONE, "minibuffer-exit", do_minibuffer_exit, 0)
    CMD1( KEY_CTRL('g'), KEY_NONE, "minibuffer-abort", do_minibuffer_exit, 1)
    CMD0( KEY_CTRL('i'), KEY_NONE, "minibuffer-complete", do_completion)
    CMD0( ' ', KEY_NONE, "minibuffer-complete-space", do_completion_space)
    CMD1( KEY_CTRL('p'), KEY_UP, "previous-history-element", do_history, -1 )
    CMD1( KEY_CTRL('n'), KEY_DOWN, "next-history-element", do_history, 1 )
    CMD_DEF_END,
};

CmdDef less_commands[] = {
    CMD0( 'q', KEY_CTRL('g'), "less-exit", do_less_quit)
    CMD1( '/', KEY_NONE, "less-isearch", do_isearch, 1)
    CMD_DEF_END,
};


QEStyleDef qe_styles[QE_STYLE_NB] = {

#define STYLE_DEF(constant, name, fg_color, bg_color, font_style, font_size, text_style) \
{ name, fg_color, bg_color, font_style, font_size, text_style },
#include "qestyles.h"
#undef STYLE_DEF

};
