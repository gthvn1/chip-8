#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
	uint8_t scale;
	SDL_Window *window;
} sdl_t;

const uint32_t CHIP8_WINDOW_WIDTH = 64;
const uint32_t CHIP8_WINDOW_LENGHT = 32;

/* Set default values */
void init_sdl(sdl_t *sdl)
{
	sdl->scale = 10;
}

bool init_window(sdl_t *sdl, const char *title)
{
	sdl->window = SDL_CreateWindow(
			title,
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			CHIP8_WINDOW_WIDTH * sdl->scale,
			CHIP8_WINDOW_LENGHT * sdl->scale,
			0);
	if (sdl->window == NULL) {
		printf("Failed to init window: %s\n", SDL_GetError());
		return false;
	}

	return true;
}

int main(int argc, char **argv)
{
	const char *title = "Chip-8 emulator";
	sdl_t sdl = {0};

	init_sdl(&sdl);

	puts("Hello, World!");
	if (!init_window(&sdl, title)) {
		return 1;
	}

	return 0;
}
