FILES=buf chr cmd col con cur inp out tab vec win \
      cmd/file

SRCDIR=src/
INCDIR=inc/
OBJDIR=obj/
BINDIR=bin/
DEPDIR=dep/

WARNINGS=all no-unused-parameter no-switch extra missing-prototypes
DEFINES=

DFLAGS=$(addprefix -D, $(DEFINES))
WFLAGS=$(addprefix -W, $(WARNINGS))
FLAGS= $(WFLAGS) --std=c99 -pedantic -g -pthread -I$(INCDIR) -fPIC -fdiagnostics-color=always $(DFLAGS)

HFILES=$(addprefix $(INCDIR), $(addsuffix .h, $(FILES)))
CFILES=$(addprefix $(SRCDIR), $(addsuffix .c, $(FILES)))
OFILES=$(addprefix $(OBJDIR), $(addsuffix .o, $(FILES)))
DFILES=$(addprefix $(DEPDIR), $(addsuffix .d, $(FILES)))
