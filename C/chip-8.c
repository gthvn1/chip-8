#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef enum {
	CREATE_WINDOW_ERROR = 1,
	CREATE_RENDERER_ERROR,
} chip8_error;

const uint32_t CHIP8_WINDOW_WIDTH = 64;
const uint32_t CHIP8_WINDOW_LENGHT = 32;

typedef struct {
	uint8_t  ram[4096]; // 4 KB RAM
	uint16_t pc; // Program Counter
	uint16_t i;  // Index register (point at location in memory)
	uint8_t  delay_timer;
	uint8_t  sound_timer;
	uint8_t  vn[16]; // General purpose registers
	uint8_t  sp; // Stack Pointer
} chip8_t;

typedef struct {
	uint8_t scale;
	SDL_Window *window;
	SDL_Renderer *renderer;
} sdl_t;

/* Set default values */
void new_sdl(sdl_t *sdl)
{
	sdl->scale = 10;
}

bool create_window(sdl_t *sdl, const char *title)
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

bool create_renderer(sdl_t *sdl)
{
	assert(sdl->window != NULL);
	sdl->renderer = SDL_CreateRenderer(
			sdl->window,
			-1,
			SDL_RENDERER_ACCELERATED);
	if (sdl->renderer == NULL) {
		printf("Failed to create renderer: %s\n", SDL_GetError());
		return false;
	}

	return true;
}

int main(int argc, char **argv)
{
	const char *title = "Chip-8 emulator";
	sdl_t sdl = {0};

	new_sdl(&sdl);

	puts("Hello, World!");
	if (!create_window(&sdl, title)) {
		return CREATE_WINDOW_ERROR;
	}

	if (!create_renderer(&sdl)) {
		SDL_DestroyWindow(sdl.window);
		return CREATE_RENDERER_ERROR;
	}

	// Next step: SDL_SetRenderDrawColor !!!

	SDL_DestroyWindow(sdl.window);
	SDL_DestroyRenderer(sdl.renderer);
	return 0;
}
