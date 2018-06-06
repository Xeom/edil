#!/usr/bin/python3

import sys

ctrlable = "@ABCDEFGKLNOPQRSTUVWXYZ]^_"

def err(s):
    print(s, file=sys.stderr)

def convert(code):
    mods =     code & 0xff00
    c    = chr(code & 0x00ff)
    if c in remaps:
        remap = remaps[c]

        if remap == None:
            err("Could not remap '{}'".format(c))
            return None

        else:
            return ord(remaps[c]) | mods

if len(sys.argv) != 3:
    err("gen-remap.py [from] [to]")
    exit()

try:
    fromfile = open(sys.argv[1], "r")
    tofile   = open(sys.argv[2], "r")

except IOError as e:
    err(e)
    exit()

fromcodes = dict(l.strip().split()[::-1] for l in fromfile.readlines())
tocodes   = dict(l.strip().split()[::-1] for l in tofile.readlines())

remaps = {key : tocodes.get(code, None) for code, key in fromcodes.items()}

for line in sys.stdin.readlines():
    if not line.startswith(" "):
        continue

    mode, code, key, bind, desc = line.split(maxsplit=4)

    code = int(code, 16)
    conv = convert(code)

    if (conv == None): continue

    c    = chr(conv & 0xff)

    if (conv & 0x400 and c not in ctrlable):
        err("{} is getting mapped to a key involving Ctrl+{}".format(bind, c))
        print("\n# The following is an invalid keybinding. Please remap this manually.")
        print("# the original keybinding on qwerty was {:x}".format(code))
        print("# unmap {} {:x}".format(mode, code))
        print("# remap {} {:x} {}\n".format(mode, conv, bind))

    else:
        print("unmap {} {:x}".format(mode, code))
        print("remap {} {:x} {}".format(mode, conv, bind))

exit(0)
