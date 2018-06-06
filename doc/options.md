Command-Line Options
====================

 * `--cmd`, `-c` - This option takes one argument, which is
   then run as a command once edil is started and initialized.
   e.g. `edil --cmd "tabwidth 8"` will start up edil with tabs
   of width eight instead of the default four.

 * `--help`, `-h` - This option simply displays a help message,
   detailing these options, then exits.

 * `--version`, `-v` - This option just prints out the current
   version of edil, and when it was compiled, then exits.

 * `--binds`, `-b` - This option prints out all of edils default
   keybindings, then exits.

 * `--filecmd`, `-f` - This option takes one argument, a filename,
   and then loads the commands stored in the specified file. This
   done using the `loadconf` command.

 * `--keycode`, `-kc` - This option takes one argument, a
    hexadecimal number. The name of the corresponding key is then
    printed to stdout and edil exits. This is the same as the hex
    numbers printed in kcd mode, accessed by Ctrl+K, or the
    inp_key enum within edil's code.

Usage
=====

To use edil, simply run the binary. Files to be edited can be
specified in arguments. These are run as if they were given to
the `new` command as arguments.

```
~/edil-2.0 $ make all
...
Built bin/edil
~/edil-2.0 $ edil -v
Edil v0.0.3-debug, -- Compiled (2018-06-02 17:27 +0100)
~/edil-2.0 $ edil --cmd "new file.txt" --cmd "new file2.txt"
... [runs edil and opens the two files]
~/edil-2.0 $ edil file.txt file2.txt
... [runs edil and opens the same files]
```
