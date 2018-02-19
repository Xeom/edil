FILES=cli text vec cmd util inp bar files

HFILES=$(addprefix inc/, $(addsuffix .h, $(FILES)))
CFILES=$(addprefix src/, $(addsuffix .c, $(FILES)))
OFILES=$(addprefix obj/, $(addsuffix .o, $(FILES)))

WARN=-Wall -Wno-unused-parameter -Wno-switch -Wextra
FLAG=--std=c99 -pedantic -g -pthread -Iinc

obj/%.o: src/%.c $(HFILES)
	mkdir -p $(@D)
	gcc $(FLAG) $(WARN) -c -g -fPIC $< -o $@

edil: $(OFILES)
	gcc $(FLAG) $(WARN) $^ -o $@
