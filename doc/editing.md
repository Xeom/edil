Basic Editing
=============

This file explains the basics of how to open, edit, and save files. It is as
basic as possible so that you can get started with using Edil quickly, but
covers everything in more detail than the README file.

Opening Files
-------------

There are several ways to open files using Edil. The simplest way is to specify
them as arguments when starting Edil. For example, running the command
`edil file1 file2` will run Edil and automatically open both files.

Once Edil is running, a file can be opened in a new buffer using the command
`new filename`. To run this command, press `Ctrl+x` and type the command, then
press `Enter`. To open a file in the current buffer, you can run the command
`load filename`. Each buffer in Edil can be associated with a file. When the
load command is invoked with a filename, the specified file is associated with
the buffer. A file can be associated with a buffer without loading it however,
using the command `associate filename`. This can be useful if, for example,
the buffer contains information already that you want to save to a file, and
so cannot override it by loading the contents of the file to it. Once a file
is associated with a buffer, its contents can be loaded by running the `load`
command with no argument.

All these methods of opening files achive the same thing. In fact, running
`edil filename` is equivient to the command `edil -c "new filename"`, which
is equivilent to running Edil and then running the command `new filename`.
This command is equivilent to running the commands `new`, to create a new
buffer, and then `load filename` to load the file into it. This is also
equivilent to running the commands `new`, `associate filename`, and then
`load`.

The `new` command can be easily accessed using the shortcut `Esc+n`. The
desired filename can be entered and then the full command run.

Editing Files
-------------

All the standard keys for navigating files can be used. The arrow keys move
the primary cursor, along with `PgUp`, `PgDn`, `Home` and `End`. Typing an
ordinary printable character will insert this character at the position of the
primary cursor, and the return key will insert a newline at this position. The
`Backspace` and `Del` keys work as expected also.

The only character which cannot be inserted by simply typing it is the tab,
which can be manually inserted using the key combination `Mod+Tab`.

Saving and Exiting
------------------

In order to save the contents of a buffer, you must run the `save` command.
When this command is run, the contents of the current buffer are saved to the
file it is associated with. This command can be quickly accessed using the
shortcut `Esc+s`.

To exit Edil, use the key combination `Esc+Ctrl+k`. This will exit the program
fully.