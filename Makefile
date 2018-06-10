include conf.mk

ifndef DEBUG
  DEBUG=yes
endif

ifeq ($(DEBUG), yes)
  FLAGS+=-g
  VERSION:=$(VERSION)-debug
else
  WARNINGS+=error
  FLAGS+=-O3
endif

DFLAGS=$(addprefix -D, $(DEFINES)) \
       -DCOMPILETIME='$(shell date +"%Y-%m-%d %H:%M %z")' \
       -DVERSION='$(VERSION)'

WFLAGS=$(addprefix -W, $(WARNINGS))
FLAGS+=$(WFLAGS) --std=$(STD) -pedantic -pthread -I$(INCDIR) -fPIC -fdiagnostics-color=always  $(DFLAGS)

HFILES=$(addprefix $(INCDIR), $(addsuffix .h, $(FILES)))
CFILES=$(addprefix $(SRCDIR), $(addsuffix .c, $(FILES)))
OFILES=$(addprefix $(OBJDIR), $(addsuffix .o, $(FILES)))
DFILES=$(addprefix $(DEPDIR), $(addsuffix .d, $(FILES)))

ifeq ($(DEBUG), yes)
  ERRPIPE=2>&1 | tee -a errs.txt || (less -R errs.txt && /bin/false)
else
  ERRPIPE=
endif

$(DEPDIR)%.d: $(SRCDIR)%.c $(HFILES) conf.mk
	@mkdir -p $(@D)
	@printf $(OBJDIR) > $@
	@$(CC) -MM -MG -MT $(@:$(DEPDIR)%.d=%.o) $(FLAGS) $< >> $@
	@printf "Created $@\n"

$(OBJDIR)%.o: $(SRCDIR)%.c conf.mk
	@mkdir -p $(@D)
	@$(CC) -c $(FLAGS) $< -o $@ $(ERRPIPE)
	@printf "Built $@\n"

$(BINDIR)edil: $(OFILES) $(SRCDIR)edil.c
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) $^ -o $@ $(ERRPIPE)
	@printf "Built $@\n"

all: deps $(BINDIR)edil
	@if [ -s errs.txt ]; then cat errs.txt | less -R; rm errs.txt; fi

install: all
	@cp $(BINDIR)edil $(INSTDIR)edil
	@printf "Installed edil in $(INSTDIR)edil\n"

uninstall:
	@rm -f $(INSTDIR)/edil
	@printf "Uninstalled edil\n"

clean_err:
	@rm -f errs.txt

clean_bin:
	@rm -rf $(BINDIR)
	@printf "Removed $(BINDIR)\n"

clean_obj:
	@rm -rf $(OBJDIR)
	@printf "Removed $(OBJDIR)\n"

clean_dep:
	@rm -rf $(DEPDIR)
	@printf "Removed $(DEPDIR)\n"

clean_docs:
	@make -C doc clean
	@make -C layouts clean

docs: all
	@make -C doc all
	@make -C layouts all

deps: $(DFILES)
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
