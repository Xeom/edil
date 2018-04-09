include conf.mk

ifndef DEBUG
  DEBUG=yes
endif

ifeq ($(DEBUG), yes)
  FLAGS+=-g
  VERSION:=$(VERSION)-debug
else
  FLAGS+=-O3
endif

DFLAGS=$(addprefix -D, $(DEFINES)) \
       -DCOMPILETIME='$(shell date +"%Y-%m-%d %H:%M %z")' \
       -DVERSION='$(VERSION)'

WFLAGS=$(addprefix -W, $(WARNINGS))
FLAGS+=$(WFLAGS) --std=c99 -pedantic -pthread -I$(INCDIR) -fPIC -fdiagnostics-color=always $(DFLAGS)

HFILES=$(addprefix $(INCDIR), $(addsuffix .h, $(FILES)))
CFILES=$(addprefix $(SRCDIR), $(addsuffix .c, $(FILES)))
OFILES=$(addprefix $(OBJDIR), $(addsuffix .o, $(FILES)))
DFILES=$(addprefix $(DEPDIR), $(addsuffix .d, $(FILES)))

ERRPIPE=2>>errs.txt || (less -r errs.txt && /bin/false)

clean_err:
	@rm -f errs.txt

clean_bin:
	@rm -f bin/*

clean_obj:
	@rm -rf obj/*

clean_dep:
	@rm -rf dep/*

$(OBJDIR)%.o: $(SRCDIR)%.c conf.mk
	@mkdir -p $(@D)
	@gcc -c $(FLAGS) $< -o $@ $(ERRPIPE)
	@printf "Built $@\n"

$(DEPDIR)%.d: $(SRCDIR)%.c $(HFILES) conf.mk
	@mkdir -p $(@D)
	@printf $(OBJDIR) > $@
	@gcc -MM -MT $(@:$(DEPDIR)%.d=%.o) $(FLAGS) $< >> $@
	@printf "Created $@\n"

$(BINDIR)libedil.so: $(OFILES)
	@mkdir -p $(@D)
	@gcc $(FLAGS) -shared $^ -o $@ $(ERRPIPE)
	@printf "Linked $@\n"

$(BINDIR)libedil.a: $(OFILES)
	@mkdir -p $(@D)
	@ar rcs $@ $^ $(ERRPIPE)
	@printf "Linked $@\n"

$(BINDIR)edil: $(SRCDIR)edil.c $(BINDIR)libedil.a
	@mkdir -p $(@D)
	@gcc $(FLAGS) $^ -o $@ $(ERRPIPE)
	@printf "Built $@\n"

deps: $(DFILES)

all: deps $(BINDIR)libedil.a $(BINDIR)libedil.so $(BINDIR)edil
	@if [ -s errs.txt ]; then cat errs.txt | less -r; fi

clean: clean_err clean_bin clean_obj clean_dep

.PHONEY=deps all clean_err clean_bin clean_obj clean_dep clean install uninstall
.DEFAULT_GOAL=all

$(shell rm -f errs.txt)

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
  ifeq (,$(findstring deps,$(MAKECMDGOALS)))
    $(info Including depenendencies ...)
    include $(DFILES)
  endif
endif
