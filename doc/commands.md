Commands
========

To type a command in *Edil*, enter *command mode*, by pressing `Ctrl-X`,
then you can simply type a command and press enter to run it.

The implementation for this system is in the `src/cmd.c` file, and files
containing implementations of commands themselves are stored in the
`src/cmd` directory.

Commands for Files
------------------

Every buffer can be associated with a file, the contents of that file can
then be loaded to that buffer, and the contents of the buffer saved to
that file.

* `load [file]` - If a file is specifed, associate that file with the
    current buffer, then load the contents of the associated file to the 
    buffer.

* `associate [file]` - If a file is specified, associate that file with
    the current buffer. Display the name of the currently associated file.

* `save` - Save the contents of the current buffer to its associated buffer.

* `cd [path]` - If a path is specified, move the current working directory
    of the application to that path. Display the name of the current working
    directory.

* `discard` - Delete and discard the contents of the current buffer.

Commands for Buffers
--------------------

* `new [file]` - Open a new buffer. If a file is specified, run
    `load [file]` on that new buffer.

* `next`, `prev` - Switch the current window to the next and previous buffers

Commands for Navigation
-----------------------

* `goto [line] [col]` - Move the primary cursor to the specified line and column,
    if specified. Display the current primary cursor position.

* `swap` - Swap the positions of the primary and secondary cursors.

Commands for Indentation
------------------------

* `tabwidth [val]` - Set the width tabs are displayed as if a value is specified.
    Display the current setting.

* `lvlwidth [val]` - Set the width of one "level" of indentation if a value is
    specified, in number of characters. This differs from `tabwidth` as, for
    example, `tabwidth` could be 4, and `lvlwidth` could be 8. If this is the
    case, pressing tab would indent the current line by two tabs.

* `indentmode [mode], ![mode]` - Enable or disable indentation modes. Modes given
    as arguments are disabled if prefixed with `!`, and enabled otherwise.
    The modes that can be enabled and disabled are listed below.
  * `spacify` - By default, indentation to a specific depth is achived using
      tabs as much as possible, and spaces as needed after the tabs. If `spacify`
      mode is enabled, then only spaces are used.
  * `auto` - If this mode is active, when the enter key is pressed, the new line
      is automatically indented to the same indentation level as the previous
      line.

* `indent [val]` - Indent the current line a certain distance, if one is specified.
    Then display the depth the line is indented to.

* `incrindent`, `decrindent` - Increment  or decrement the indentation level of 
    the current line.

* `autoindent` - Automatically indent the current line. 
