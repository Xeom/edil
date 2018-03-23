include common.mk

deps_print:
	@printf "Building deps ... "

deps: deps_print $(DFILES)
	@printf "Done\n"

$(DEPDIR)%.d: $(SRCDIR)%.c $(HFILES) conf.mk
	@mkdir -p $(@D)
	@printf $(OBJDIR) > $@
	@gcc -MM $(FLAGS) $< >> $@

.PHONEY=deps
.DEFAULT_GOAL=deps
