# edil

*edil* is my personal project of a text editor, originally
created and turned into a disaster two years ago, this is
it's rebirth.

## Motivation

As a programmer, I use text editors often. My favourite has,
for a long time, been *emacs*. However, *emacs* is ancient, obese,
and no one knows their way around its codebase. Writing extensions
is difficult to do without creating a million bugs from the
most terrifying corners of 1997.

*vim* is an excellent alternative in many ways, and while far
from perfect, it definitely holds its own against *emacs*, 
particularly in the speed and bugginess department. I've been a
fan of *emacs* for too long however, to un-patriotically turn my
back on it and edit my text with its arch-rival.

*edil* was my solution to this problem. Creating my own text editor
will be a fun and useful project.

## Building

The Makefile included should build an executable as `bin/edil`.
*edil* currently and is planned to only compile on Linux, and
is built with the [simple terminal](https://st.suckless.org/)
in mind.

## Using edil

### Modes

Each mode can be entered into by pressing control plus a key.

* `Ctrl+A` - **buffer mode**: This mode allows the user to edit text in
                              the currently selected buffer.
* `Ctrl+X` - **bar mode**: This mode allows the user to type and run
                           commands for the currently selected window.

### Commands

*edil* has various commands that can be accessed in bar mode.

* `load [file]`: Associate a file with the currently selected buffer,
                 and load that file's contents to the buffer. If no
                 file is specified, and one is already associated, reload
                 that file.
* `dump`:        Dump the contents of a file into the file associated with
                 it.
* `cd [path]`:   Move the current working directory of the application to
                 the location specified by `[path]`. If no path is specified,
                 then do not change the working directory. Then return the
                 current working directory.
