# Qi

Welcome to Qi ([pronounced 'chi'][qi]).

Qi's goal is to be my perfect editor. It attempts to combine all the things I like about _emacs_ with the speed of _vi_. Qi focuses on the following goals:

- Speed and large file editing
- Only terminal editor (no x11, etc)
- Emacs bindings and usage emulation (chords, minibuffer, modes, etc)
- Compiles to a single, self contained binary
- Cross platform (unix based system)
- Multi-language (human) support - with a focus on English and Chinese.
- Multi-language (computer) support
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

## Building

1. Run the configure tool:

```shell
./configure
```

You can see the possible options by typing 

```shell
./configure --help
```

2. Run the make file to build the core editor and plugins:

```shell
make qi
```

If everything worked, you should have a `qi` binary in the main project directory.


## Documentation

The original QEmacs documentation is in _doc/qe-doc.html_; however, not all QEmacs features are in Qi, and some exist that are not in the current documentation. However most of the documentation is still relevant.

## Licensing

QEmacs was released under the GNU Lesser General Public License v2 (read the accompanying LICENSE file) so Qi is as well.

- Qi is copyright 2013 [The Rohans, LLC.][rohans]
- QEmacs is copyright 2002 Fabrice Bellard.

[qi]: http://en.wikipedia.org/wiki/Qi
[rohans]: http://therohans.com
