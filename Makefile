FILES=buf chr col cur out vec

HFILES=$(addprefix inc2/, $(addsuffix .h, $(FILES)))
CFILES=$(addprefix src2/, $(addsuffix .c, $(FILES)))
OFILES=$(addprefix obj/, $(addsuffix .o, $(FILES)))

WARN=-Wall -Wno-unused-parameter -Wno-switch -Wextra
FLAG=--std=c99 -pedantic -g -pthread -Iinc2 -fdiagnostics-color=always

obj/%.o: src2/%.c $(HFILES)
	mkdir -p $(@D)
	gcc $(FLAG) $(WARN) -c -g -fPIC $< -o $@

edil: $(OFILES)
	gcc $(FLAG) $(WARN) $^ -o $@
