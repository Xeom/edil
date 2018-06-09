keys.md: ../bin/edil
	./genkeys.sh > $@

commands.md: ../bin/edil
	../bin/edil -cl > $@

all: keys.md commands.md

clean:
	@rm keys.md
	@echo Removed keys.md
	@rm commands.md
	@echo Removed commands.md

.DEFAULT_GOAL=all
.PHONEY=all clean
