include conf.mk

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
  $(shell make -f deps.mk -j8 1>&2)
  include $(DFILES)
endif

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
	@printf "Building $@ ... "
	@mkdir -p $(@D)
	gcc -c $(FLAGS) $< -o $@ $(ERRPIPE)
	@printf "Done\n"

$(BINDIR)libedil.so: $(OFILES)
	@printf "Linking $@ ... "
	@mkdir -p $(@D)
	@gcc $(FLAGS) -g -shared $^ -o $@ $(ERRPIPE)
	@printf "Done\n"

$(BINDIR)libedil.a: $(OFILES)
	@printf "Linking $@ ... "
	@mkdir -p $(@D)
	@ar rcs $@ $^ $(ERRPIPE)
	@printf "Done\n"

$(BINDIR)edil: $(SRCDIR)edil.c $(BINDIR)libedil.a
	@printf "Building $@ ... "
	@mkdir -p $(@D)
	@gcc $(FLAGS) $^ -o $@ $(ERRPIPE)
	@printf "Done\n"

all: clean_err $(BINDIR)libedil.a $(BINDIR)libedil.so $(BINDIR)edil
	@if [ -s errs.txt ]; then cat errs.txt | less -r; fi

clean: clean_err clean_bin clean_obj clean_dep

.PHONEY=all clean_err clean_bin clean_obj clean_dep clean
.DEFAULT_GOAL=all
