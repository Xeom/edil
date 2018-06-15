FILES=buf/buf buf/text buf/line buf/chr buf/col buf/cur \
      bar cmd file indent inp out ring ui win bind \
      container/vec container/namevec container/circvec container/table \
      cmd/file cmd/nav cmd/indent cmd/buf cmd/region cmd/conf \
      bind/curbind bind/cmdbind bind/modebind bind/barbind \
      bind/barmap bind/bufmap bind/movmap bind/kcdmap bind/lngmap

HEADERS=types


SRCDIR=src/
INCDIR=inc/
OBJDIR=obj/
BINDIR=bin/
DEPFILE=deps.d

INSTDIR=/usr/local/bin/

VERSION=0.2.0

WARNINGS=all extra no-unused-parameter no-switch missing-prototypes

STD=c11

DEFINES=
