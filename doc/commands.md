Commands
========

To type a command in *Edil*, enter *command mode*, by pressing
`Ctrl-X`, then you can simply type a command and press enter to
run it.

Commands take arguments delimited by spaces after their name,
e.g. `load file1`, and for arguments with spaces inside them,
quotation marks can be used. e.g. `load "file 1"`.

Many commands are also bound to shortcut or run on certain key
combinations. A shortcut is where, when you press the key, edil
enters bar mode and the command is inserted into the bar ready to
be run. This allows you to specify arguments for the command, or
prevents accidentally running the command when you did not mean
to.

The implementation for this system is in the `src/cmd.c` file, and
files containing implementations of commands themselves are stored
in the `src/cmd` directory.

List of commands
----------------
 * [associate](#associate-command) - _Associate a buffer with a file_
 * [autoindent](#autoindent-command) - _Automatically indent a line_
 * [bufinfo](#bufinfo-command) - _Display information about the current buffer_
 * [cd](#cd-command) - _Change directory_
 * [conffile](#conffile-command) - _Load a config file_
 * [copy](#copy-command) - _Copy the current region_
 * [cut](#cut-command) - _Cut the current region_
 * [decrindent](#decrindent-command) - _Undent a line_
 * [discard](#discard-command) - _Discard the contents of a buffer_
 * [goto](#goto-command) - _Go to a specific line and column_
 * [incrindent](#incrindent-command) - _Indent a line_
 * [indent](#indent-command) - _Set the indentation depth of a line_
 * [indentmode](#indentmode-command) - _Set indent modes_
 * [kill](#kill-command) - _Kill the current buffer_
 * [lineify](#lineify-command) - _Select the current line_
 * [load](#load-command) - _Load a file to a buffer_
 * [lvlwidth](#lvlwidth-command) - _Set the indent level width_
 * [new](#new-command) - _Create a new buffer_
 * [next](#next-command) - _Go to the next buffer_
 * [paste](#paste-command) - _Paste a buffer_
 * [prev](#prev-command) - _Go to the previous buffer_
 * [quit](#quit-command) - _Quit edil_
 * [remap](#remap-command) - _Remap a key_
 * [save](#save-command) - _Save a buffer to a file_
 * [saveall](#saveall-command) - _Save all buffers_
 * [snap](#snap-command) - _Snap the secondary cursor_
 * [swap](#swap-command) - _Swap the cursors_
 * [tabwidth](#tabwidth-command) - _Set the tab width_
 * [translate](#translate-command) - _Translate a keypress_
 * [unmap](#unmap-command) - _Unmap a key_

Full documentation
------------------

#### Associate command 
 - Associate a buffer with a file

If an argument is given, then that file is associated with the
current buffer. Regardless of whether any argument is given, the
file associated with the current buffer is returned. The contents
of the buffer are not affected, so the associate command can be
to copy a file, for example.

---
#### Autoindent command 
 - Automatically indent a line

Set the indent of the current line to the indentation of the
previous line.

---
#### Bufinfo command 
 - Display information about the current buffer

If a file is associated with the buffer, the path of
that file is also returned, along with the buffer's
index number.

Prints flags associated with the buffer:
 * assoc - The buffer is associated with a pipe or file.
 * cr - The buffer uses '
' style newlines.
 * pipe - The buffer is associated with a pipe.
 * ro - The buffer is read-only.
 * mod - The buffer has been modified.
 * nofile - No file can be associated with this buffer.
 * nokill - This buffer cannot be killed.

---
#### Cd command 
 - Change directory

Change edil's current working directory to one specified as an
argument. Even if no argument is given, the command returns the
current working directory.

---
#### Conffile command 
 - Load a config file


---
#### Copy command 
 - Copy the current region

Copy the current region between the primary and secondary cursors
the contents of the region is placed into the clipboard buffer,
which is created if it does not exist already.

---
#### Cut command 
 - Cut the current region

This command runs the copy command, and then deletes the region.

---
#### Decrindent command 
 - Undent a line

Decrease the indentation of the current line to the previous
indentation level as specified by lvlwidth.

---
#### Discard command 
 - Discard the contents of a buffer

This command is useful when you don't want to save the contents of
a buffer. It deletes the contents of the buffer and removes its
modified flag.

---
#### Goto command 
 - Go to a specific line and column

 * If given, the first argument specifies the line number to go to.
 * If given, the second argument specifies the column number to go
   to.
Once the command is run, cursor's column and line number are
returned and printed. Both column and line numbers for this
command are indexed starting at one.

---
#### Incrindent command 
 - Indent a line

Increase the indentation of the current line to the next
indentation level as specified by lvlwidth.

---
#### Indent command 
 - Set the indentation depth of a line

If a number is given as an argument, then indent the current line
such that it begins with that number of columns of whitespace.

Even if an argument is not given, the command returns the current
indentation depth of the current line.

---
#### Indentmode command 
 - Set indent modes

Various indent modes can be set:
 * `spacify` - By default, indentation to a specific depth is
    achived using tabs as much as possible, and spaces as needed
    after the tabs. If spacify mode is enabled, then only spaces
    are used.
 * `auto` - If this mode is active, when the enter key is pressed,
    the new line is automatically indented to the same indentation
    level as the previous line.
 * `trim` - When the enter key is pressed, the trailing whitespace
    on the current line is trimmed.
 * `skipblanks` - Blank lines are not considered when calculating
    the automatic indent, and so the automatic level is the same as
    the previous non-blank line. A blank line is one consisting only
    of whitespace.

To set a mode, give it as an argument, and to unset a mode, give it
as an argument prefixed with an '!'.
---
#### Kill command 
 - Kill the current buffer

By default, this command kills the current buffer. If other
buffers are given as arguments however, it will kill them
instead. Buffers are not killed if they are modified.
If '!' is given as an argument, the buffers will be
force-killed, even if they are modified.

---
#### Lineify command 
 - Select the current line

Move the primary cursor to the beginning of the current line, and
the secondary cursor to the end. This places the whole line in the
region, and allows for it to easily be cut or copied etc.

---
#### Load command 
 - Load a file to a buffer

(Re)load the file associated with the current buffer. If an
argument is given, that file is associated with the buffer before
it is loaded.

---
#### Lvlwidth command 
 - Set the indent level width

The level width is the depth of an indent level, i.e. how far a
line is indented when the tab key is pressed. This is not
always the width of one tab.

The command takes a new width as a single argument, but even if
this is not given, the command prints out the current value.

---
#### New command 
 - Create a new buffer

Create a new buffer and switch to it. If a filename is given as an
argument, then this filename is associated with the buffer and
loaded. If the file is already open, then the command simply
switches to the buffer where it is open.

---
#### Next command 
 - Go to the next buffer

Go to the buffer with an index one higher than the current
buffer, or if there is no such buffer, loop around to the
first buffer.

---
#### Paste command 
 - Paste a buffer

Insert the contents of another buffer at the location of the cursor
By default, the clipboard buffer is inserted, but if an argument is
given, it can specify a different buffer index.
---
#### Prev command 
 - Go to the previous buffer

Go to the buffer with an index one lower than the current
buffer, or if there is no such buffer, loop around to the
final buffer.

---
#### Quit command 
 - Quit edil

Exit edil, if no buffers are modified. If '!' is given as
an argument, then edil will exit even if buffers are
modified

---
#### Remap command 
 - Remap a key


---
#### Save command 
 - Save a buffer to a file

Save the contents of the current buffer to the file associated
with it.

---
#### Saveall command 
 - Save all buffers

Performs the equivialent of the same command to all buffers, where
appropriate.

---
#### Snap command 
 - Snap the secondary cursor

Move the secondary cursor to the position of the primary cursor.

---
#### Swap command 
 - Swap the cursors

Move the primary cursor to the secondary cursor, and vice versa.
The primary and secondary cursors swap places.

---
#### Tabwidth command 
 - Set the tab width

The tab width is the width that tabs are displayed as.

The command takes a new width as a single argument, but even if
this is not given, the command prints out the current value.

---
#### Translate command 
 - Translate a keypress


---
#### Unmap command 
 - Unmap a key


---
