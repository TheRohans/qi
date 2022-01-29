# Qi

Welcome to Qi ([pronounced 'chi'][qi]).

Qi's goal is to be my perfect editor. It attempts to combine all the things I like about _emacs_ with the speed of _vi_. Qi focuses on the following goals:

- Speed and large file editing
- Only terminal editor (no x11, etc)
- Emacs bindings and usage emulation (chords, minibuffer, modes, etc)
- Compiles to a single, self contained binary
- Cross platform (unix based system)
- Multi-language (human) support - with a focus on English and Chinese.
- Multi-language (computer) stupport
- Default UTF-8

Qi's roots are QEmacs; Qi's base is a fork of QEmacs 3.2 with a lot of bits stripped out.

## Current Features

- Emacs keybindings
- Text editing mode
- Open and edit large files
- Basic C mode
- Basic Markdown mode
- Basic XML mode (and HTML)
- Hex editing mode
- UTF-8 display

## Compiling

- Launch the configure tool _./configure_. You can look at the
  possible options by typing _./configure --help_.

- Type _make qi_ to compile Qi and its built-in plugins

## Documentation

The original QEmacs documentation is in _doc/qe-doc.html_; however, not all QEmacs features are in Qi and some exist that are not in the documentation.

## Licensing

QEmacs was released under the GNU Lesser General Public License (read the accompagning COPYING file) so Qi is as well.

Qi is copyright 2013 [The Rohans, LLC.][rohans]

QEmacs is copyright 2002 Fabrice Bellard.

[qi]: http://en.wikipedia.org/wiki/Qi
[rohans]: http://therohans.com
