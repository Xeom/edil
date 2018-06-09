keys.md: ../bin/edil
	./genkeys.sh > $@

commands.md: ../bin/edil
	../bin/edil -cl > $@

all: keys.md commands.md

.DEFAULT_GOAL=all