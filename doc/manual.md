# Qi Manual

## Introduction

Qi is a small UNIX editor. Some of it's features include:

- Full screen editor with an Emacs look and feel with some common Emacs features: multi-buffer, multi-window, command mode, minibuffer with completion and history.
- Can edit files of hundreds of Megabytes without being slow by using a highly optimized internal representation and by mmaping the file.
- Unicode support.
- C mode: coloring with immediate update. Emacs like auto-indent.
- Hexadecimal editing mode with insertion and block commands. Unicode hexa editing is also supported.
- Works on any VT100 terminals without termcap. UTF8 VT100 support included with double width glyphs.
- Small footprint. About 480K.

While reading this document you will see `C` and `M` referenced when talking about keybindings. These are almost always:
- `C` = Control key
- `M` = Alt key (or Esc key)

So, when you see `C-x` that means press the _control key_ while also pressing the _x key_.

These can, however, be mapped differently based on your system.

## Invocation

```
usage: qi [-h] [filename...]
```

| Flag | Does |
| -- | -- |
| --help,-h | show help and exit |
| --verson,-V | show version and exit |


Qi will start in _dired_ mode automatically so that you can browse your files easily (same as _C-x C-d_ key).

## Common editing commands

The best way to see all the most up to date keybinding is to type `M-x describe-bindings` while running the editor.

### Concepts

Qi store file content in _buffers_. Buffers can be seen as big arrays of bytes.

An _editing mode_ tells how to display the content of a buffer and how to interact with the user to modify its content.

Multiple _windows_ can be shown on the screen at the same time. Each window show the content of a buffer with an editing mode. It means that you can open several windows which show the same buffer in different modes (for example, in both text and hexadecimal).

Each key binding activates a _command_. You can directly execute a command by typing _M-x <command> RET_

Commands can take arguments. The key binding _C-u N_ where N is an optional number is used to give a numeric argument to the commands which can handle them. If the command cannot handle a numerical argument, it is simply repeated _N_ times.

For example, if you type `C-u 5 M-x next-line RET` the cursor will move down 5 lines. Don't worry, this is not the only way to move around.


### Help

You can press _C-h b_ to have the list of all the currently active bindings, including the ones of the current editing mode.

```
C-x ?, F1               : help-for-help
```

### Text and Movement keys (commands)

```
default                 : self-insert-command
C-p, up                 : previous-line
C-n, down               : next-line
C-b, left               : backward-char
C-f, right              : forward-char
M-b, C-left             : backward-word
M-f, C-right            : forward-word
M-v, prior              : scroll-down
C-v, next               : scroll-up
home, C-a               : beginning-of-line
end, C-e                : end-of-line
insert                  : overwrite-mode
C-d, delete             : delete-char
backspace               : backward-delete-char
M-<, C-home             : beginning-of-buffer
M->, C-end              : end-of-buffer
C-i                     : tabulate
C-q                     : quoted-insert
RET                     : newline
M-{                     : backward-paragraph
M-}                     : forward-paragraph
```

### Region handling

```
C-k                     : kill-line
C-space                 : set-mark-command
C-w                     : kill-region
M-w                     : copy-region
C-y                     : yank
M-y                     : yank-pop
C-x C-x                 : exchange-point-and-mark
```

### Buffer and file handling

```
C-x C-s                 : save-buffer
C-x C-w                 : write-file
C-x C-c                 : suspend-emacs
C-x C-f                 : find-file
C-x C-v                 : find-alternate-file
C-x b                   : switch-to-buffer
C-x k                   : kill-buffer
C-x i                   : insert-file
C-x C-q                 : vc-toggle-read-only
C-x C-b                 : list-buffers
```

### Search and replace

```
C-s                     : isearch-backward
C-r                     : isearch-forward
M-%                     : query-replace
```

### Command handling

```
M-x                     : execute-extended-command
C-u                     : universal-argument
C-g                     : abort
C-x u, C-_              : undo
C-x (                   : start-kbd-macro
C-x )                   : end-kbd-macro
C-x e                   : call-last-kbd-macro
```

### Window handling

```
C-x o                   : other-window
C-x 0                   : delete-window
C-x 1                   : delete-other-windows
C-x 2                   : split-window-vertically
C-x 3                   : split-window-horizontally
C-x f                   : toggle-full-screen
```

### Miscellaneous

```
C-l                     : refresh
M-g                     : goto-line
M-q                     : fill-paragraph
C-x RET l               : toggle-line-numbers
C-x RET t               : truncate-lines
C-x RET w               : word-wrap
C-x C-e                 : compile
C-x C-p                 : previous-error
C-x C-n                 : next-error
C-x C-d                 : dired
```

## Editing Modes

Qi has support for a few differnt file types by default. File support is handled with something called a _mode_. When you activate a mode, the syntax highlighting changes to support the mode, and some modes offer new commands for the given mode. The default mode is `text-mode`. 

Most modes operate on the file extension being loaded, but you can force a mode by typing `M-x <mode-name>`. The following are modes builtin to Qi by default.

### C Mode

This mode is currently activated by `M-x c-mode`. It is activated automatically when a C file is loaded.

### XML Mode

This mode is activated by _M-x xml-mode_. It is activated automatically when an XML file is loaded.

Currently, only specific XML colorization is done in this mode. Javascript (in SCRIPT tags) is colored as in C mode. CSS Style
sheets (in STYLE tags) are colorized with a specific color.

### Hexadecimal Mode

Qi has powerful hexadecimal editing modes: all common commands work in these modes, including the block commands. 

The hexadecimal mode, `M-x hex-mode`, shows both the hexadecimal and ascii (bytes) values. You can toggle between the hexa and ascii columns with `TAB`.

The ascii mode, `M-x ascii-mode`, only shows the ascii column.


The unihex mode, `M-x unihex-mode` shows both the unicode and glyph associated to each _character_ of the buffer by using the current
buffer charset (default utf-8).

You can change the line width in these modes with `C-left` and `C-right`.

### Dired Mode

You can activate it with `C-x C-d`. You can open the selected directory with `C-f` or `right`. `C-b` or `left` is used to go to the parent directory. The current selected is opened in the right window. You can use `C-x o` to edit the currently file you are visiting without leaving dired, or use `C-f` to open the file for editing.

### Bufed Mode

While you can eaily switch buffer using `C-x b`, Sometimes it is helpful to see a list of all the currently open buffers. You can use Bufed to do that. You can activate Bufed with _C-x C-b_. You can select with _RET_ or _right_ the current buffer.

## Developper's Guide

### Plugins

You can use one of the examples in `src/plugin/' to develop a Qi plugins (aka modules).

Plugins can add any dynamic resource qi supports (modes, keybindings, ...).


