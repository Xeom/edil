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

Once Edil is running, a file can be opened in a new buffer using the
[new command](/doc/commands.md#new-command) in the form `new filename`.
To run a [command](/doc/commands.md), press `Ctrl+x` and type the command,
then press `enter`. To open a file in the current buffer, you can run the
command `load filename`. Each buffer in Edil can be associated with a file.
When the [load command](/doc/commands.md#load-command) is invoked with a
filename, the specified file is associated with the buffer.

A file can be associated with a buffer without loading it however,
using the command `associate filename`. The
[associate command](/doc/commands#associate-command) can be useful if, for
example, the buffer contains information already that you want to save to a
file, and so you don't want to override it by loading the contents of the file
to it. Once a file is associated with a buffer, its contents can be loaded by
running the load command with no argument.

Edil includes shortcuts to the load and new commands. In order to quickly
run the load command, press `Esc+a`, and to quickly run the new command, press
`Esc+n`.

All these methods of opening files achieve the same thing. In fact, running
`edil filename` is equivalent to the command `edil -c "new filename"`, which
is equivalent to running Edil and then running the command `new filename`.
This command is equivalent to running the commands `new`, to create a new
buffer, and then `load filename` to load the file into it. This is also
equivalent to running the commands `new`, `associate filename`, and then
`load`.

Editing Files
-------------

All the [standard keys](/doc/keys.md#buf-mode) for navigating files can be
used. The default mode of the editor is 'buffer mode', where the arrow keys
move the primary cursor, and typing an ordinary printable character will insert
this character at the position of the primary cursor, and the `enter` key will
insert a newline, just like most other editors.

The only character which cannot be inserted by simply typing it is the tab,
which can be manually inserted using the key combination `Esc+tab`. The `tab`
key will indent the current line by one level when pressed. The combination
`Shift+tab` will un-indent the current line.

Edil includes a [movement mode](/doc/keys.md#mov-mode), which may be useful if
you prefer a more vim-style of editing.

Edil does not have a region mode - the region is always present. It is the text
between the two cursors, the primary (white), and secondary (blue), including
the characters highlighted by both cursors. Edil includes copy, paste and cut
commands for this region, with shortcuts of `Ctrl+Y` and `Ctrl+P` for copy
and paste respectively.

Saving and Exiting
------------------

In order to save the contents of a buffer, you must run the
[save command](/doc/commands.md#save-command). When this command is run, the
contents of the current buffer are saved to the file it is associated with.
This command can be quickly accessed using the shortcut `Esc+s`.
Edil also includes the [saveall command](/doc/commands.md#saveall-command),
which saves the contents of every modified open buffer. The saveall command
has a shortcut of `Esc+Shift+s`.

To exit Edil, use the key combination `Esc+Ctrl+k` to run the `quit` command,
if you want to quit without saving, you need to run the command `quit !`.
