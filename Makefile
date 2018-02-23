include conf.mk

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
  $(shell make -f deps.mk -j8 1>&2)
  include $(DFILES)
endif

clean_err:
	@rm -f errs.txt

clean_bin:
	@rm -f bin/*

clean_obj:
	@rm -rf obj/*

clean_dep:
	@rm -rf dep/*

$(OBJDIR)%.o: $(SRCDIR)%.c
	@printf "Building $@ ... "
	@mkdir -p $(@D)
	@gcc -c $(FLAGS) $< -o $@ 2>>errs.txt
	@printf "Done\n"

bin/edil: $(OFILES)
	@printf "Building $@ ... "
	@mkdir -p $(@D)
	@gcc $(FLAGS) $^ -o $@ 2>>errs.txt
	@printf "Done\n"

all: clean_err bin/edil
	@if [ -s errs.txt ]; then cat errs.txt | less -r; fi

clean: clean_err clean_bin clean_obj clean_dep

.PHONEY=all clean_err clean_bin clean_obj clean_dep clean
.DEFAULT_GOAL=all
