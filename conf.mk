FILES=bar buf chr cmd col cur file indent inp namevec table out ring vec ui win bind \
      cmd/file cmd/nav cmd/indent cmd/buf cmd/region cmd/conf \
      bind/curbind bind/cmdbind bind/modebind bind/barbind \
      bind/barmap bind/bufmap bind/movmap bind/kcdmap

SRCDIR=src/
INCDIR=inc/
OBJDIR=obj/
BINDIR=bin/
DEPFILE=deps.d

INSTDIR=/usr/local/bin/

VERSION=0.1.0

WARNINGS=all no-unused-parameter no-switch extra missing-prototypes

STD=c11

DEFINES=
