#!/bin/bash

echo Keybindings
echo ===========
echo
echo "The following are edil's default keybindings, generated by the"
echo "command \`edil --binds.\`"

bin/edil --binds | sed 's/^ | \([0-9a-f]\{3\}\) \([^ ]*\)/ | `\2`  /g'
