include conf.mk

ifndef DEBUG
  DEBUG=yes
endif

ifeq ($(DEBUG), yes)
  FLAGS += -g
  VERSION:=$(VERSION)-debug
else
  FLAGS += -O3
endif

DFLAGS=$(addprefix -D, $(DEFINES)) \
       -DCOMPILETIME='$(shell date +"%Y-%m-%d %H:%M %z")' \
       -DVERSION='$(VERSION)'

WFLAGS=$(addprefix -W, $(WARNINGS))
FLAGS+= $(WFLAGS) --std=c99 -pedantic -pthread -I$(INCDIR) -fPIC -fdiagnostics-color=always $(DFLAGS)

HFILES=$(addprefix $(INCDIR), $(addsuffix .h, $(FILES)))
CFILES=$(addprefix $(SRCDIR), $(addsuffix .c, $(FILES)))
OFILES=$(addprefix $(OBJDIR), $(addsuffix .o, $(FILES)))
DFILES=$(addprefix $(DEPDIR), $(addsuffix .d, $(FILES)))
