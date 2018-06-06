VARIANTS=dvorak qwerty dvp dvorak-l dvorak-r colemak

KEYFILES=$(addprefix keys/, $(addsuffix .keys, $(VARIANTS)))

keys/%.keys: gen-xcodes
	mkdir -p keys
	./gen-xcodes.sh $@

gen-xcodes: gen-xcodes.c
	gcc $< -o $@ -lX11 -lxcb -lX11-xcb

all: $(KEYFILES)
