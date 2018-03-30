# Edil

*Edil* is my personal project of a text editor, originally
created and turned into a disaster two years ago, this is
it's rebirth.

## Motivation

As a programmer, I spend a lot of my time using text editors.
My favourite has, for a long time, been *Emacs*. Despite being
a very powerful and excellent text editor in many ways, *Emacs*
is ancient, obese, and no one knows their way around its codebase.
Writing extensions or fixing anything is impossible without creating
a million bugs from the most terrifying corners of 1997.

*Vim* is an excellent alternative in many ways, and while far
from perfect, it definitely holds its own against *Emacs*. 
Particularly in the speed and bugginess department. I've been a
fan of *Emacs* for too long however, so I could never 
un-patriotically turn my back on it and edit my text with its 
arch-rival.

*Edil* is my solution to this problem. Creating my own text editor
is a fun and useful project, and hopefully the end result will solve
a lot of my problems with the text editors that are around today.

## Building

The Makefile included should build an executable as `bin/edil`.
*Edil* currently and is planned to only compile on Linux, and
is built with the [simple terminal](https://st.suckless.org/)
in mind.

## Using Edil

More detailed documentation can be found in the `docs/` directory:

* [Command documentation](doc/commands.md) can be found in `docs/commands.md`.
* [Keybind documentation](doc/keys.md) can be found in `docs/keys.md`.
