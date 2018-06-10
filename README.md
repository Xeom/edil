# Edil

Edil is my personal project of a text editor. Work started on
Edil in 2015, however that project eventually turned info a
disaster. This is a second attempt, emphasising making everything
simple and lightweight.

![EDIL](/doc/img/title.png)

## Motivation

As a programmer, I spend a lot of my time using text editors.
My favourite has, for a long time, been Emacs. Despite being
a very powerful and excellent text editor in many ways, Emacs
is ancient, obese, and no one knows their way around its codebase.
Writing extensions or fixing anything is impossible without creating
a million bugs from the most terrifying corners of 1997.

Vim is an excellent alternative in many ways, and while far
from perfect, it definitely holds its own against Emacs.
Particularly in the speed and bugginess department. I've been a
fan of Emacs for too long however, so I could never
un-patriotically turn my back on it and edit my text with its
arch-rival.

Edil is my solution to this problem. Creating my own text editor
is a fun and useful project, and hopefully the end result will solve
a lot of my problems with the text editors that are around today.

## Using Edil

### Building

Included in the repository is a Makefile. This can be used to build
Edil and produce an executable at `./bin/edil`. Currently, Edil is
built to compile on Linux, and is tested and used by me on the
[simple terminal](https://st.suckless.org/).

```
 ~/edil-2.0 $ make all
...
Built bin/edil
 ~/edil-2.0 $ ./bin/edil -v
Edil v0.0.2 -- Compiled (2018-04-09 11:56 +0100)
 ~/edil-2.0 $
```

### Quick Start Guide

1. In order to open a file in Edil, use the key combination `Esc-n`,
   to run the `new` command. Then you can type the name of the file
   you want to open press enter to run this command. The file will
   open in a new buffer.

2. To switch between buffers, you can use the key combinations
   `Ctrl-n` and `Ctrl-v`. This allows you to edit multiple files at
   once.

3. To save an edited file, you can simply run the `save` command by
   using the key combination `Esc-s` and pressing enter. This will
   save the file in the current buffer.

4. To exit Edil, you can use the key combination `Ctrl-Esc-k`. This
   will close edil. If you don't wish to save what changes you have
   made, you can run the `discard` command by pressing `Ctrl-x` and
   typing `discard`, then pressing enter.

### Further documentation

More detailed information can be found in the `doc/` directory:

* [Command documentation](doc/commands.md) can be found in `doc/commands.md`.
* [Keybind documentation](doc/keys.md) can be found in `doc/keys.md`.
* [Basic editing guide](doc/editing.md) can be found in `doc/editing.md`.
* [Command-line options](doc/options.md) can be found in `doc/options.md`.
