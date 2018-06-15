FILES=buf/buf buf/text buf/line \
      bar chr cmd col cur file indent inp namevec table out ring vec ui win bind circvec \
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
