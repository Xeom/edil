Keybindings
===========

The following are edil's keybindings for buffer mode.

Basic editing
-------------

| Key             | Binding                                           |
| ---             |   ---                                             |
| `Arrow keys`    | Move primary cursor.                              |
| `Printable key` | Type character.                                   |
| `Return`        | Insert newline.                                   |
| `Home`, `End`   | Move to the end and start of the current line.    |
| `PgUp`, `PgDn`  | Move to the top and bottom of the current screen. |
| `Delete`        | Delete a character.                               |
| `Backspace`     | Move backwards, then delete a character.          |

Changing modes
--------------

| Key             | Binding                                           |
| ---             |   ---                                             |
| `Ctrl+Esc+k`    | Close the application.                            |
| `Ctrl+k`        | Enter key mode.                                   |
| `Ctrl+x`        | Enter command mode.                               |
| `Ctrl+a`        | Enter buffer mode.                                |

Indentation
-----------

| Key             | Binding                                           |
| ---             |   ---                                             |
| `Tab`           | Increment the indentation of the current line.    |
| `Esc+Tab`       | Insert a tab.                                     |
| `Shift+Tab`     | Decrement the indentation of the current line.    |

Shortcuts
---------

| Key             | Binding                                           |
| ---             |   ---                                             |
| `Esc+n`         | The `new` command, with a blank argument.         |
| `Esc+a`         | The `associate` command, with a blank argument.   |
| `Esc+s`         | The `save` command, but does not run it.          |
| `Esc+g`         | The `goto` command, with blank arguments.         |
| `Ctrl+c`        | Run the command `swap`.                           |
| `Ctrl+z`        | Run the command `snap`.                           |
| `Ctrl+n`        | Run the command `next`.                           |
| `Ctrl+v`        | Run the command `prev`.                           |

Selections
----------

Selections are the text between the primary and secondary cursors, including
the cursors themselves.

| Key             | Binding                                           |
| ---             |   ---                                             |
| `Esc+Arrow Key` | Move selection in chosen direction.               |
| `Ctrl+l`        | Select the current line.                          |
| `Ctrl+y`        | Copy the current selection to clipboard.          |
| `Ctrl+p`        | Paste the current clipboard.                      |
