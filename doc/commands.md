Commands
========

To type a command in *Edil*, enter *command mode*, by pressing
`Ctrl-X`, then you can simply type a command and press enter to
run it.

Commands take arguments delimited by spaces after their name,
e.g. `load file1`, and for arguments with spaces inside them,
quotation marks can be used. e.g. `load "file 1"`.

Many commands are also bound to shortcut or run on certain key
combinations. A shortcut is where, when you press the key, Edil
enters bar mode and the command is inserted into the bar ready to
be run. This allows you to specify arguments for the command, or
prevents accidentally running the command when you did not mean
to.

The implementation for this system is in the `src/cmd.c` file, and
files containing implementations of commands themselves are stored
in the `src/cmd` directory.

List of commands
----------------
 * [__associate__](#associate-command) - _Associate a buffer with a file_
 * [__autoindent__](#autoindent-command) - _Automatically indent a line_
 * [__basebar__](#basebar-command) - _Set the window base-bar string_
 * [__buffer__](#buffer-command) - _Switch to a specified buffer_
 * [__bufinfo__](#bufinfo-command) - _Display information about the current buffer_
 * [__cd__](#cd-command) - _Change directory_
 * [__conffile__](#conffile-command) - _Load a config file_
 * [__copy__](#copy-command) - _Copy the current region_
 * [__cut__](#cut-command) - _Cut the current region_
 * [__decrindent__](#decrindent-command) - _Un-indent a line_
 * [__discard__](#discard-command) - _Discard the contents of a buffer_
 * [__eofnl__](#eofnl-command) - _Always end files in newlines_
 * [__goto__](#goto-command) - _Go to a specific line and column_
 * [__incrindent__](#incrindent-command) - _Indent a line_
 * [__indent__](#indent-command) - _Set the indentation depth of a line_
 * [__indentmode__](#indentmode-command) - _Set indent modes_
 * [__kill__](#kill-command) - _Kill the current buffer_
 * [__lineify__](#lineify-command) - _Select the current line_
 * [__load__](#load-command) - _Load a file to a buffer_
 * [__lvlwidth__](#lvlwidth-command) - _Set the indent level width_
 * [__new__](#new-command) - _Create a new buffer_
 * [__next__](#next-command) - _Go to the next buffer_
 * [__paste__](#paste-command) - _Paste a buffer_
 * [__prev__](#prev-command) - _Go to the previous buffer_
 * [__quit__](#quit-command) - _Quit Edil_
 * [__remap__](#remap-command) - _Remap a key_
 * [__save__](#save-command) - _Save a buffer to a file_
 * [__saveall__](#saveall-command) - _Save all buffers_
 * [__snap__](#snap-command) - _Snap the secondary cursor_
 * [__swap__](#swap-command) - _Swap the cursors_
 * [__tabwidth__](#tabwidth-command) - _Set the tab width_
 * [__translate__](#translate-command) - _Translate a keypress_
 * [__unmap__](#unmap-command) - _Unmap a key_

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
#### Basebar command
 - Set the window base-bar string

Sets the string displayed in the window bar. This by default shows
the name of the current buffer, the current cursor position and
the current mode. Escape sequences prefixed with '%' are used here
to set the content of the bar:

| Sequence | Produces |
| -------- | -------- |
| %L, %C | The line and column of the primary cursor.|
| %l, %c | The line and column of the secondary cursor.|
| %w, %h | The width and height of the window.|
| %x, %y | The x and y position of the window.|
| %b     | The number of lines in the current buffer.|
| %p     | The percentage of lines in the current buffer below the topof the page. |
| %n     | The name of the current buffer.|
| %m     | The name of the current mode.|
| %f     | The full filename of the current file.|
| %X     | Blank, this is useful for terminating colours.|
| %%     | A literal percentage sign.|

Colours can also be specified, in the format `%fg,bg,attrs`. The
background and attributes are optional however, making `%fg` and
`%fg,bg` valid sequences. All three options are specified by numbers
that correspond to the enums defined in [inc/col.h](/inc/col.h).

For the background and foreground, the following are the the colour
codes:

| Code | Colour |
| ---- | ------ |
| 0    | Black  |
| 1    | Red    |
| 2    | Green  |
| 3    | Yellow |
| 4    | Blue   |
| 5    | Magenta|
| 6    | Cyan   |
| 7    | White  |
| 16   | None   |

Adding 8 to one of the first 8 colour codes will produce a brighter
version of that colour. For example, Black (0) plus 8 produces
grey.

For the attributes, the following values may be summed:

| Code | Effect     |
| ---- | ------     |
| 1    | Bold       |
| 2    | Underlined |
| 4    | Inverted   |
| 8    | Blinking   |

The default basebar is ' **%n** **%8**│**%0**
**%L**·**%C** **%8**│**%0** **%m**'.

---
#### Buffer command
 - Switch to a specified buffer

This command switches the current window to a specified buffer.
It takes one argument, which is used to search for a matching
buffer. If no argument is given, the it goes back to the
buffer that was previously navigated away from using this
command.

If the command is given a number, then the buffer with that id is
chosen.

Then, the names of every buffer are checked. These are generally
basenames of the files associated with those buffers.

Finally, the full path of every buffer is checked.

The buffer that is switched to will be the first one after the
current buffer that matches.

---
#### Bufinfo command
 - Display information about the current buffer

If a file is associated with the buffer, the path of
that file is also returned, along with the buffer's
index number and the buffer's name.

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

Change Edil's current working directory to one specified as an
argument. Even if no argument is given, the command returns the
current working directory.

---
#### Conffile command
 - Load a config file

Configuration files are just lists of commands, one on each line,
that are run by Edil. The commands are run just as if they were
typed after pressing `Ctrl+X`. Blank lines, and those starting
with a `#` symbol are ignored.

This command can be passed multiple paths as arguments, and will
run the commands contained in each one. It requires at least one
argument.

Commands in `~/.edil`, `~/.edil.conf`, `~/.edil.remap`,
`~/.config/edil`, `~/.config/edil.conf`, and `~/config/edil.remap`
are run by default when Edil starts up.


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
 - Un-indent a line

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
#### Eofnl command
 - Always end files in newlines

Every file can have an eofnl flag set. If the flag is set, then
when it is saved, an extra newline is added to the end, if one is
not already there.

The command can take the following arguments:
 - `all` - Enables the flag on all newly opened files by default.
 - `!all` - Disables the flag by default.
 - `1` - Enables the flag on this specific file.
 - `0` - Disables the flag on this specific file.

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
    achieved using tabs as much as possible, and spaces as needed
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
`load file1` is equivalent to `associate file1` followed by
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
had been run. i.e. `new file1` is equivalent to `new` followed
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
 - Quit Edil

Exit Edil, if no buffers are modified. If `!` is given as
an argument, then Edil will exit even if buffers are
modified

---
#### Remap command
 - Remap a key

Remaps a key to a new binding for a specific mode. The
[unmap](#unmap-command) command removes one of these mappings.
The `remap` command takes three arguments, the first is the three
letter name of the mode being affected, e.g. `buf`, `bar`, or
`mov`.

The second argument is the key being rebound, as a hexadecimal
keycode. These keycodes can be found by pressing `Ctrl+K` in edil
and entering keycode mode. When in this mode, every keypress types
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

Performs the equivalent of the [save](#save-command) command to all
buffers, where they are modified, and associated with a file.

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
table. When an input key is received by the input system, it is
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
