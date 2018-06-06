#!/usr/bin/python3

import sys
from subprocess import check_output

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
            return "Could not remap"

        else:
            return ord(remaps[c]) | mods

namecache = dict();

def getname(code):
    if code in namecache:
        return namecache[code]

    cmd = "../bin/edil -kc {:x}".format(code)
    output = check_output(cmd, shell=True).decode("utf-8")
    name   = output.strip().split()[1]

    namecache[code] = name

    return name

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

    if (conv == code): continue

    if (conv == None): continue

    if (conv == "Could not remap"):
        print("# !!! We could not automatically find the appropriate key\n"
              "#     to map {0} to.\n"
              "#     Originally, {0} was mapped to {1}\n".format(getname(code), bind))
        continue

    c = chr(conv & 0xff)

    if (conv & 0x400 and c not in ctrlable):
        err("{} is getting mapped to invalid key {}".format(bind, getname(code)))

        print("# !!! The following is an invalid keybinding to '{0}'. \n"
              "#     Please remap this manually. The original keybinding\n"
              "#     was '{1}'\n"
              "# unmap {2} {3:x}\n"
              "# unmap {2} {4:x} {5}\n" \
              .format(getname(conv), getname(code), mode, code, conv, bind))

    else:
        print("# Remap '{0}' to '{1}'.\n"
              "unmap {2} {3:x}\n"
              "remap {2} {4:x} {5}\n" \
              .format(getname(code), getname(conv), mode, code, conv, bind))
