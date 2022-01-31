# Qi

![docs](doc/qi.svg)

Qi's goal is to be my perfect editor. It attempts to combine the things I like about _emacs_ with the speed and system integration of _vi_. Qi focuses on the following goals:

- Speed and large file editing
- Only terminal editor (no x11, etc)
- Emacs bindings and usage emulation (chords, minibuffer, modes, etc)
- Compiles to a single, self contained binary
- Cross platform (unix based system)
- Multi-language (human) support - with a focus on English and Chinese.
- Multi-language (computer) support
	- C
	- Markdown
	- HTML (XML)
	- Golang
	- YAML
	- Lua
- Default encoding UTF-8
- Hex editing

Qi was forked from QEmacs 3.2 with a lot of features stripped out.

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
make dist
```

If everything worked, you should have a `qi` binary in the `dist` project directory.

## Documentation / Manual

[Read the Manual](docs/manual.md)

## Licensing

QEmacs was released under the GNU Lesser General Public License v2 (read the accompanying LICENSE file) so Qi is as well.

- Qi is copyright 2013 [The Rohans][rohans]
- QEmacs is copyright 2002 Fabrice Bellard.

[qi]: http://en.wikipedia.org/wiki/Qi
[rohans]: http://therohans.com
