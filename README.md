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

### Modes

Each mode can be entered into by pressing control plus a key.

* `Ctrl+A` - **buffer mode**: This mode allows the user to edit text in
                              the currently selected buffer.
* `Ctrl+X` - **bar mode**: This mode allows the user to type and run
                           commands for the currently selected window.
* `Ctrl+K` - **key mode**: This mode types ascii representations of user
                           keystrokes into the current buffer. e.g. 
                           `(031 1)`, `(106 del)`, `(657 Ctrl+Esc+W)`


### Commands

*Edil* has various commands that can be accessed in bar mode.

#### Files

* `load [file]` - Associate a file with the currently selected buffer,
                  and load that file's contents to the buffer. If no
                  file is specified, and one is already associated, reload
                  that file.

* `save` - Save the contents of the current buffer to the file associated with it.

* `cd [path]` - Move the current working directory of the application to
                the location specified by `[path]`. If no path is specified,
                then do not change the working directory. Then return the
                current working directory.

* `discard` - Discard and delete the contents of the current buffer.

* `associate [file]` - Associate the current buffer with a specific file. Then
                       return the current file associated with the current buffer.


#### Buffers

* `new [file]` - Open a new buffer. If a file is specified, that file is loaded
                 to the new buffer, equivialent to `new` followed by `load [file]`.

* `next` - Go to the next buffer.

* `prev` - Go to the previous buffer.

#### Navigation

* `goto [line] [col]` - Goto the specified line and column, if given. Then return
                        the new location of the cursor.

* `swap` - Swap the position of the primary and secondary cursors.

#### Indentation

* `tabwidth [number]` - Set the width tabs are displayed as if a number is given.
                        Then return the current value.
                       
