#!/bin/bash

layout=$(setxkbmap -query | grep -oP "(?<=layout:).*$")
variant=$(setxkbmap -query | grep -oP "(?<=variant:).*$")

if [[ -z $variant ]]; then
    reset_cmd="setxkbmap -layout $layout"
else
    reset_cmd="setxkbmap -layout $layout -variant $variant"
fi

function gen()
{
    test=$(basename $1 .keys)

    if [[ $test == qwerty ]]; then
        setxkbmap -layout $layout
    else
        setxkbmap -layout $layout -variant $test
    fi
    echo Generating $1

    ./gen-xcodes > $1
    $reset_cmd
}

for f in $@; do
    gen $f
done