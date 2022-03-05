# Qi Manual

## Introduction

Qi is a small UNIX editor. Some of it's features include:

- Editor with an Emacs look and feel with some common Emacs
features: multi-buffer, multi-window, command mode, minibuffer with
completion and history. 
- Can efficiently edit files of hundreds of Megabytes.
- Unicode support.
- Hexadecimal editing mode with insertion and block commands. 
- Works on any VT100 terminals without termcap. UTF8 VT100 support included.
- Support for C/C++, Go, Terraform, Python, and others.
- Small footprint. About 480K.

While reading this document you will see `C` and `M` referenced when talking
about keybindings. These are almost always:
- `C` = Control key 
- `M` = Alt key (or Esc key)

So, when you see `C-x` that means press the _control key_ while also
pressing the _x key_.

These can, however, be mapped differently based on your system.

## Invocation

```
usage: qi [-h] [filename...]
```

| Flag | Does |
| -- | -- |
| --help,-h | show help and exit |
| --version,-V | show version and exit |


Qi will start in _dired_ mode automatically so that you can browse your
files easily (same as _C-x C-d_ key).

## Common editing commands

The best way to see all the most up to date keybinding is to type `M-x
describe-bindings` while running the editor.

### Concepts

Qi store file content in _buffers_. Buffers can be seen as big arrays of bytes.

An _editing mode_ tells how to display the content of a buffer and how to
interact with the user to modify its content.

Multiple _windows_ can be shown on the screen at the same time. Each window
show the content of a buffer with an editing mode. It means that you can
open several windows which show the same buffer in different modes (for
example, in both text and hexadecimal).

Each key binding activates a _command_. You can directly execute a command
by typing _M-x <command> RET_

Commands can take arguments. The key binding _C-u N_ where N is an optional
number is used to give a numeric argument to the commands which can handle
them. If the command cannot handle a numerical argument, it is simply
repeated _N_ times.

For example, if you type `C-u 5 M-x next-line RET` the cursor will move down
5 lines. Don't worry, this is not the only way to move around.

### Help

You can press _C-h b_ to have the list of all the currently active bindings,
including the ones of the current editing mode.

```
C-x ?, F1               : help-for-help
```

### Text and Movement keys (commands)

```
C-p, up                 : previous-line
C-n, down               : next-line
C-b, left               : backward-char
C-f, right              : forward-char
C-a, home               : beginning-of-line
C-e, end                : end-of-line
C-d, delete             : delete-char
backspace               : backward-delete-char
C-x u, C-_              : undo
M-g                     : goto-line

M-b	                    : backward-word
M-f                     : forward-word
C-v, pgdn               : scroll-down
M-v, pgup               : scroll-up
insert                  : overwrite-mode
M-<, C-home             : beginning-of-buffer
M->, C-end              : end-of-buffer
C-i                     : tabulate
RET                     : newline
M-{                     : backward-paragraph
M-}                     : forward-paragraph
C-l                     : center-line
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
   C-c                      : toggle case sensitivity
   C-w                      : toggle word find
M-%                     : query-replace
M-r                     : replace-string
```

### Command handling

```
C-g                     : abort
M-x                     : execute-extended-command
```

### Window handling

```
C-x o                   : other-window
C-x 0                   : delete-window
C-x 1                   : delete-other-windows
C-x 2                   : split-window-vertically
C-x 3                   : split-window-horizontally
C-x up                  : find-window-up
C-x down                : find-window-down
C-x left                : find-window-left
C-x right               : find-window-right
C-x n                   : next-window
C-x p                   : previous-window
```

### Miscellaneous

```
C-t                     : refresh (display, force redraw)
M-q                     : fill-paragraph (force to 76 chars)
C-x RET l               : toggle-line-numbers
C-x RET t               : truncate-lines
C-x RET w               : word-wrap
```

### File browser (Dir'ed)

```
C-x C-d                 : dired (Show directory browser)
C-f                          : open file
C-p                          : parent directory
```

### Buffer browser (Buf'ed)

```
C-x C-b                 : Show buffer browser
k                             : kill selected buffer
%                             : toggle readonly
r                             : refresh view
```


## Startup Config

(**Subject to change**) You can add a file named `.qirc` to your home
directory where you can do some basic editor startup configuration. For
example:

```
/* comment */
toggle-line-numbers()
set-indent-width(2)
```

## Editing Modes

Qi has support for a few different file types by default. File support is
handled with something called a _mode_. When you activate a mode, the syntax
highlighting can change, some new commands could be activated, and some other
extra tools might be available. Some of the commands and tools will be covered
here, but it's best to [look through the code](https://github.com/TheRohans/qi/tree/main/src/plugins) 
as the modes are constantly being updated.

The default mode is `text-mode`.

Most modes are activated when the the file is loaded (often based of the
extension or file name), but you can force a mode by typing 
`M-x <mode-name>`. The following are modes builtin to Qi by default.

### Text Mode

Text mode is the default editing mode. If you have `aspell` installed, you
can run the current through the spell checker with `C-x .`.

### C Mode

This mode is currently activated by `M-x c-mode`. It is activated
automatically when a C file is loaded.

### Config Mode

There is a generic mode for configuration files that does some basic things
like highlight tab characters. This mode will activate when you open an
`.ini` or `.yaml` type file. It will also activate one some specific files
like `Dockerfile` or `Makefile`. You can turn this on anytime with 
`M-x config-mode`

### XML Mode

This mode is activated by `M-x xml-mode`. It is activated automatically when
an XML file is loaded.

Currently, only specific XML colourisation is done in this mode. 

### HTML Mode

HTML mode is similar to XML Mode, but Javascript (in `script` tags) is
coloured by the Typescript mode. Likewise, code in `style` tags is coloured by
`css-mode`.

### Typescript Mode

Typescript mode handles both Javascript and Typescript modes - 
`M-x typescript-mode`.

### Markdown Mode

You can activate markdown mode with `M-x markdown-mode`, or by opening a
`.markdown` or `.md` file.

### Python Mode

Python mode can be activated by: `M-x python-mode`.

### Maths Mode

Maths mode is useful if you want to type formulas in your markdown files or
in comments. It creates keyboard shortcuts that can be used to type utf-8
friendly maths symbols. Here is an example:

```
        ∞  ______
f₁(x) = ∑ ∛ x²+y³ ∙ n÷π
       n=1
```

```
           ⌈1 2 3⌉   ⌈x₁⌉
 [x y z] ⨯ |4 5 6| = |y₁|
           ⌊7 8 9⌋   ⌊z₁⌋
```

You can enable this mode by `M-x maths-mode`. It will not change your syntax
highlighting, only add some of those keybindings. There are many bindings
and they are in flux. It is best to [look at the code](https://github.com/TheRohans/qi/blob/b2b7780284b6c68c2ef6829014426ccb5e5e9abd/src/plugins/maths-mode/maths.c#L35)
for the most up to date information, but almost all the bindings are based
around `M-` keys (`M-p` for π for example).

### Hexadecimal Mode

Qi has powerful hexadecimal editing modes: all common commands work in these
modes, including the block commands.

The hexadecimal mode, `M-x hex-mode`, shows both the hexadecimal and ASCII
(bytes) values. You can toggle between the hexa and ASCII columns with
`TAB`.

The ASCII mode, `M-x ascii-mode`, only shows the ASCII column.

The unihex mode, `M-x unihex-mode` shows both the Unicode and glyph
associated to each character of the buffer by using the current buffer
charset (utf-8).

You can change the line width in these modes with `C-left` and `C-right`.

### Dired Mode

Dired mode is used to browse directories. You can activate it with 
`C-x C-d`. You can open the selected directory or file with `C-f` or `right`. 
`C-b` or `left` is used to go to the parent directory. 

The current selected is opened in the window to the right. You can use `C-x o` to
edit the currently file you are visiting without closing dired, or use `C-f`
to open the file for editing.

### Bufed Mode

While you can easily switch buffer using `C-x b`, Sometimes it is helpful to
see a list of all the currently open buffers. You can use Bufed to do that.
You can activate Bufed with _C-x C-b_. You can select with _RET_ or _right_
the current buffer.

## Analysing Code

(**Subject to change, and beta**) We are currently playing with adding LSP
(Language Server Protocol) support to Qi, but that wont be added for quite a
while. However, currently Qi does support running code formatters against
the currently open file.

You will need to install the formatter yourself (using `apt-get` or
`homebrew` or what have you), and make sure that the applications are on
your `PATH` variable, but once you do you can activate the formatters with
`C-x y`. See below for a list of supported formatters.

### Supported Formatters

- Golang: [`gofmt`](https://pkg.go.dev/cmd/gofmt)
- C/C++: [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html)
- Python: [`black`](https://pypi.org/project/black/)

### LSP (Language Server)

(**Under development**)

- `gopls`
- `clangd`

---

## Developer's Guide

### Plugins

You can use one of the examples in `src/plugin/` to develop a Qi plugins
(aka modules).

Plugins can add any dynamic resource Qi supports (modes, keybindings, ...).

### Debugging

When you configure the application build with `--enable-debug`, on top of
adding debug symbols that can be used with `gdb`, there is a preprocessor
macro added to the code `LOG`. `LOG` will output debug information to
`stderr`.

This can be unexpected because by default the messages will get written to
`stdout`. They will start showing up randomly in the text editor UI. You can 
fix this by running debug builds using the command:

```
./qi 2>err.txt
```

If you then open the err.txt file within qi, you can monitor the messages
written by `LOG` (by doing `M-x revert-buffer` from within the err.txt
pane) or just `tail` the file.


### Helpful Functions

```
put_error(EditorState *s, const char *fmt, ...)
put_status(EditorState *s, const char *fmt, ...)
void switch_to_buffer(EditState *s, EditBuffer *b)
void show_popup(EditBuffer *b) 
EditState *insert_window_left(EditBuffer *b, int width, int flags)
eb_new(<buffer_name>, BF_SAVELOG)
LOG("%s", "write to the debug buffer")
```

