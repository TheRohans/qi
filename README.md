# Qi

[![.github/workflows/c-cpp.yml](https://github.com/TheRohans/qi/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/TheRohans/qi/actions/workflows/c-cpp.yml)

```
          _ 
         (_)
   __ _   _ 
  / _` | | |
 | (_| | | |
  \__, | |_|
     | |    
     |_|    

```

Qi's goal is to be my perfect editor. It attempts to combine the things I
like about _emacs_ with the speed and system integration of _vi_. Qi focuses
on the following goals:

- Small foot print (~350K full feature, ~100K tiny build)
- Speed 
- Large file editing
- Terminal editor (no x11, etc)
- Emacs bindings and usage emulation (chords, minibuffer, modes, etc)
- Compiles to a single, self contained binary
- Cross platform (posix based systems)
- Multi-language (human) support - with a focus on English, French and Chinese.
- Multi-language (computer) support
	- C
	- Golang
	- Makefile
	- Markdown
	- HTML (XML)
	- Typescript
	- Python
	- YAML / Terraform
	- Lua
- UTF-8 encoding
- Hex editing

Qi was originally forked from QEmacs 3.2. Qi has been heavily modified from
QEmacs, and it is not backwards compatible with QEmacs.

## Demo

[![Qi Demo Video](https://img.youtube.com/vi/Gap4lMlfPBM/0.jpg)](https://www.youtube.com/watch?v=Gap4lMlfPBM)

## Documentation / Manual

[Read the Manual](doc/manual.md)

## Building

Qi should build on any POSIX system. It is currently being run daily on:
- Ubuntu 20.04
- M1 Macs
- Raspberry Pi 1
- FreeBSD 13 (Beta: only working from within Xorg)

### Quick Start

```shell
./configure
make dist
sudo make install
```

If you leave off the `make install` step, you should have a `qi` binary in
the `dist` directory.

### Slower Start

You can see the possible options by typing 

```shell
./configure --help
```

If you `--enable-tiny` you will get a very small build with almost all of
qi's extra features not included. For example, there will be no syntax
highlighting, no file browser, and no beta features. Depending on your
system, the size of the editor should be down in the 100K range.

`--prefix` can be used to install qi somewhere that doesn't require admin
access, and `--cc` can be used to specify a different compiler.

Do you feel lucky? Well, `--enable-beta` might be for you. This will turn on
features that are currently under development which could be cool or could
crash everything.

Example of custom build:

```shell
./configure --cc=clang --prefix=~/ --enable-beta
make dist
make install
```

### Quick Start (Developer)

Using `--enable-debug` will build qi with debugging symbols enabled, and it
also activates a macro that does some logging (see [the manual](doc/manual.md#developers-guide) 
for more information).

## Licensing

QEmacs was released under the GNU Lesser General Public License v2 (read the
accompanying LICENSE file) so Qi is as well.

- Qi is Copyright 2013 [The Rohans][rohans]
- QEmacs is Copyright 2002 Fabrice Bellard.

[qi]: http://en.wikipedia.org/wiki/Qi
[rohans]: http://therohans.com
