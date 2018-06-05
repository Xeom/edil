#!/bin/bash

echo Keybindings
echo ===========
echo
echo "The following are edil's default keybindings, generated by the"
echo "command \`edil --binds.\` The script [doc/genkeys.sh](/doc/genkeys.sh)"
echo "has generated this file."
echo

bin/edil --binds | sed -r 's/^\| ([^ ]+) ([^ ]+)( *)    \| ([^ ]+)( *)  \|/| `\1` `\2`\3| `\4`\5|/'
