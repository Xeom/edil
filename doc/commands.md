Commands
========

associate command 
-----------------
 - Associate a buffer with a file

If an argument is given, then that file is associated with the
current buffer. Regardless of whether any argument is given, the
file associated with the current buffer is returned. The contents
of the buffer are not affected, so the associate command can be
to copy a file, for example.

autoindent command 
------------------
 - Automatically indent a line

Set the indent of the current line to the indentation of the
previous line.

bufinfo command 
---------------
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

cd command 
----------
 - Change directory

Change edil's current working directory to one specified as an
argument. Even if no argument is given, the command returns the
current working directory.

conffile command 
----------------
 - Load a config file


copy command 
------------
 - Copy the current region

Copy the current region between the primary and secondary cursors
the contents of the region is placed into the clipboard buffer,
which is created if it does not exist already.

cut command 
-----------
 - Cut the current region

This command runs the copy command, and then deletes the region.

decrindent command 
------------------
 - Undent a line

Decrease the indentation of the current line to the previous
indentation level as specified by lvlwidth.

discard command 
---------------
 - Discard the contents of a buffer

This command is useful when you don't want to save the contents of
a buffer. It deletes the contents of the buffer and removes its
modified flag.

goto command 
------------
 - Go to a specific line and column

 * If given, the first argument specifies the line number to go to.
 * If given, the second argument specifies the column number to go
   to.
Once the command is run, cursor's column and line number are
returned and printed. Both column and line numbers for this
command are indexed starting at one.

incrindent command 
------------------
 - Indent a line

Increase the indentation of the current line to the next
indentation level as specified by lvlwidth.

indent command 
--------------
 - Set the indentation depth of a line

If a number is given as an argument, then indent the current line
such that it begins with that number of columns of whitespace.

Even if an argument is not given, the command returns the current
indentation depth of the current line.

indentmode command 
------------------
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
kill command 
------------
 - Kill the current buffer

By default, this command kills the current buffer. If other
buffers are given as arguments however, it will kill them
instead. Buffers are not killed if they are modified.
If '!' is given as an argument, the buffers will be
force-killed, even if they are modified.

lineify command 
---------------
 - Select the current line

Move the primary cursor to the beginning of the current line, and
the secondary cursor to the end. This places the whole line in the
region, and allows for it to easily be cut or copied etc.

load command 
------------
 - Load a file to a buffer

(Re)load the file associated with the current buffer. If an
argument is given, that file is associated with the buffer before
it is loaded.

lvlwidth command 
----------------
 - Set the indent level width

The level width is the depth of an indent level, i.e. how far a
line is indented when the tab key is pressed. This is not
always the width of one tab.

The command takes a new width as a single argument, but even if
this is not given, the command prints out the current value.

new command 
-----------
 - Create a new buffer

Create a new buffer and switch to it. If a filename is given as an
argument, then this filename is associated with the buffer and
loaded. If the file is already open, then the command simply
switches to the buffer where it is open.

next command 
------------
 - Go to the next buffer

Go to the buffer with an index one higher than the current
buffer, or if there is no such buffer, loop around to the
first buffer.

paste command 
-------------
 - Paste a buffer

Insert the contents of another buffer at the location of the cursor
By default, the clipboard buffer is inserted, but if an argument is
given, it can specify a different buffer index.
prev command 
------------
 - Go to the previous buffer

Go to the buffer with an index one lower than the current
buffer, or if there is no such buffer, loop around to the
final buffer.

quit command 
------------
 - Quit edil

Exit edil, if no buffers are modified. If '!' is given as
an argument, then edil will exit even if buffers are
modified

remap command 
-------------
 - Remap a key


save command 
------------
 - Save a buffer to a file

Save the contents of the current buffer to the file associated
with it.

saveall command 
---------------
 - Save all buffers

Performs the equivialent of the same command to all buffers, where
appropriate.

snap command 
------------
 - Snap the secondary cursor

Move the secondary cursor to the position of the primary cursor.

swap command 
------------
 - Swap the cursors

Move the primary cursor to the secondary cursor, and vice versa.
The primary and secondary cursors swap places.

tabwidth command 
----------------
 - Set the tab width

The tab width is the width that tabs are displayed as.

The command takes a new width as a single argument, but even if
this is not given, the command prints out the current value.

translate command 
-----------------
 - Translate a keypress


unmap command 
-------------
 - Unmap a key


