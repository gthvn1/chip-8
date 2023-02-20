#include <SDL2/SDL_timer.h>

#include "graphics.h"
#include "chip8.h"

typedef enum {
	OK = 0,
	INIT_SDL_ERROR,
	INIT_CHIP8_ERROR,
	ROM_FILENAME_IS_MISSING,
} error_t;

void help(char *progname)
{
	printf("USAGE: %s <rom>\n", progname);
}

int main(int argc, char **argv)
{
	const char *title = "Chip-8 emulator";
	chip8_t *chip8;
	sdl_t *sdl;

	sdl = init_sdl(title);
	if (sdl == NULL)
		return INIT_SDL_ERROR;

	//clean_screen(&sdl);

	// check if there is at least one argument and init chip8
	if (argc < 2) {
		cleanup_sdl(sdl);
		help(argv[0]);
		return ROM_FILENAME_IS_MISSING;
	}

	chip8 = init_chip8(argv[1]);
	if (chip8 == NULL) {
		cleanup_sdl(sdl);
		return INIT_CHIP8_ERROR;
	}

	draw_black_screen(sdl);

	// Main loop
	while (get_chip8_state(chip8) != QUIT) {
		handle_sdl_input(chip8);

		if (get_chip8_state(chip8) == PAUSED)
			continue;

		emulate_chip8(chip8);
		SDL_Delay(17);
		update_sdl_window(); // Not yet implemented...
	}

	cleanup_chip8(chip8);
	cleanup_sdl(sdl);
	return OK;
}
