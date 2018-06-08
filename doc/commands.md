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
current buffer. This means that the [save](#save-command), and
[load](#load-command) will work for the associated file.

The contents of the buffer are unaffected by this command, so it
is, for example, possible to copy a file:
```
new "file1"
associate "file2"
save
```
This will copy file1 to file2.

Regardless of whether any argument is given, the file associated
with the current buffer is returned.

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
 * `assoc` - The buffer is associated with a pipe or file.
 * `cr` - The buffer uses '
' style newlines.
 * `pipe` - The buffer is associated with a pipe.
 * `ro` - The buffer is read-only.
 * `mod` - The buffer has been modified.
 * `nofile` - No file can be associated with this buffer.
 * `nokill` - This buffer cannot be killed.

---
#### Cd command 
 - Change directory

Change edil's current working directory to one specified as an
argument. Even if no argument is given, the command returns the
current working directory.

---
#### Conffile command 
 - Load a config file

Configuration files are just lists of commands, one on each line,
that are run by edil. The commands are run just as if they were
typed after pressing `Ctrl+X`. Blank lines, and those starting
with a `#` symbol are ignored.

This command can be passed multiple paths as arguments, and will
run the commands contained in each one. It requires at least one
argument.

Commands in `~/.edil`, `~/.edil.conf`, `~/.config/edil`, and
`~/.config/edil.conf` are run by default when edil starts up.


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
a buffer. It deletes the contents of the current buffer and removes
its modified flag. The buffer can then be killed without edil
complaining.

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
If `!` is given as an argument, the buffers will be
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

This command will load the contents of an associated file to the
current buffer, or reload them if they are already loaded.

The command takes one argument optionally, which is the path of
a file to associate with the buffer before loading, just as if the
[associate](#associate-command) command had been run. i.e.
`load file1` is equivialent to `associate file1` followed by
`load`.

If the file specified to the command does not exist, it is created,
and if it is already open, the command simply switches to the
buffer where it is open instead of opening it twice.

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

Create a new buffer and switch to it. Optionally, open a new
file in the new buffer.

The argument is a filename, which is associated with the buffer
and loaded to it, just as if the [load](#load-command) command
had been run. i.e. `new file1` is equivialent to `new` followed
by `load file1`.

If the file does not exist, it is created, and if it is already
open, the command simply switches to the buffer where it is open
instead of creating a new one.


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

Exit edil, if no buffers are modified. If `!` is given as
an argument, then edil will exit even if buffers are
modified

---
#### Remap command 
 - Remap a key

Remaps a key to a new binding for a specific mode. The
[unmap](#unmap-command) removes one of these mappings. This command
takes three arguments, the first is the the letter name of the mode
being affected, e.g. `buf`, `bar`, or `mov`.

The second is the key being rebound, as a hexadecimal keycode.
These keycodes can be found by pressing `Ctrl+K` in edil and
entering keycode mode. When in this mode, every keypress types
the relevant key name and code as a hexadecimal number. Internally
these values are stored in the `inp_key` enum, defined in
the [inp header](/inc/inp.h).

The third argument is the bind to remap. e.g. `cur_mv_l`,
`cmd_goto`, or `mode_mov`. The bindings currently used can be
viewed the [doc/keys.md](/doc/keys.md) file, or can be viewed by
running the command `edil --binds`. They are the third column.

Once a binding is remapped, pressing the key associated with it,
while in the correct mode, will run the associated bind. For
example:
```
remap mov 068 cur_mv_l
remap mov 06a cur_mv_d
remap mov 06b cur_mv_u
remap mov 06c cur_mv_r
```
will remap the `h j k l` keys to move the cursor in movement mode,
vim style!

---
#### Save command 
 - Save a buffer to a file

Save the contents of the current buffer to the file associated
with it.

This command takes no arguments

---
#### Saveall command 
 - Save all buffers

Performs the equivialent of the [save](#save-command) command to allbuffers, where they are modified, and associated with a file.

This command takes no arguments and returns the number of files
that have been saved.

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

This command adds a pair of `inp_key`s to the `inp_keytranslate`
table. When an input key is recieved by the input system, it is
passed through this table, which for example, turns `Ctrl+I` to
`inp_key_tab`. Making changes to this table can be useful if your
terminal maps different keys to different codes.

The first argument is the keycode being mapped from, and the second
is the keycode being mapped to. Both are hexadecimal numbers.
These keycodes can be found by pressing `Ctrl+K` in edil and
entering keycode mode. When in this mode, every keypress types
the relevant key name and code as a hexadecimal number. Internally
these values are stored in the `inp_key` enum, defined in
the [inp header](/inc/inp.h).


---
#### Unmap command 
 - Unmap a key

This command removes the binding from a key in a specific mode.
It is the opposite of the [remap](#remap-command) command. It takes
only two arguments. The first is the three letter name of the mode
that a mapping is being removed from, e.g. `kcd`, or `buf`.

The last argument is the key being unbound, as a hexadecimal
keycode. These keycodes can be found by pressing `Ctrl+K` in edil
and entering keycode mode. When in this mode, every keypress types
the relevant key name and code as a hexadecimal number. Internally
these values are stored in the `inp_key` enum, defined in
the [inp header](/inc/inp.h).

Unbinding a key causes nothing to happen when it is pressed while
edil is in the relevant mode. For example,
```
unmap mov 64b
```
will unmap the shortcut of the `quit` command from `Ctrl+Esc+K`.

---
