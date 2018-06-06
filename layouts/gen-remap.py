#!/usr/bin/python3

import sys

if len(sys.argv) != 3:
    print("gen-remap.py [from] [to]")
    exit()

try:
    fromfile = open(sys.argv[1], "r")
    tofile   = open(sys.argv[2], "r")

except IOError as e:
    print(e)
    exit()

fromcodes = dict(l.strip().split()[::-1] for l in fromfile.readlines())
tocodes   = dict(l.strip().split()[::-1] for l in tofile.readlines())

remaps = {key : tocodes.get(code, None) for code, key in fromcodes.items()}

def convert(code):
    mods =     code & 0xff00
    c    = chr(code & 0x00ff)
    if c in remaps:
        remap = remaps[c]

        if remap == None:
            print("Could not remap '{}'".format(c))
            return None

        else:
            return ord(remaps[c]) | mods

for line in sys.stdin.readlines():
    if not line.startswith(" "):
        continue

    mode, code, key, bind, desc = line.split(maxsplit=4)

    code = int(code, 16)
    conv = convert(code)

    if (conv != None):
        print("remap {} {:x} {}".format(mode, conv, bind))

exit(0)
