.PHONY: clean

CFLAGS=-Wall -Werror -g -O0 -pedantic
BIN=chip8-emulator

$(BIN): main.o graphics.o chip8.o
	gcc -o $@ $? `sdl2-config --cflags --libs` $(CFLAGS)

%.o: %.c
	gcc -c $<

clean:
	rm -f $(BIN) *.o
