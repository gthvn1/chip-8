.PHONY: clean

CFLAGS=-Wall -Werror

chip-8: chip-8.c
	gcc -o $@ $< `sdl2-config --cflags --libs` $(CFLAGS)

clean:
	rm -f chip-8
