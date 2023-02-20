#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_log.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "graphics.h"

struct _sdl_t {
	uint8_t scale;
	SDL_Window *window;
	SDL_Renderer *renderer;
};


static bool create_window(sdl_t *sdl, const char *title)
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

static bool create_renderer(sdl_t *sdl)
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

/* Set default values */
sdl_t *init_sdl(const char *title)
{
	sdl_t *sdl = calloc(1, sizeof(sdl_t));
	if (sdl == NULL) {
		printf("Failed to allocate memory for sdl\n");
		return NULL;
	}

	sdl->scale = 10;
	SDL_Init(SDL_INIT_VIDEO); // init SDL2

	if (!create_window(sdl, title)) {
		goto free_sdl;
	}

	if (!create_renderer(sdl)) {
		SDL_DestroyWindow(sdl->window);
		goto free_sdl;
	}

	return sdl;

free_sdl:
	free(sdl);
	return NULL;

}

void cleanup_sdl(sdl_t *sdl)
{
	if (sdl) {
		SDL_DestroyRenderer(sdl->renderer);
		SDL_DestroyWindow(sdl->window);
		SDL_Quit();

		free(sdl);
		sdl = NULL;
	}
}

void draw_black_screen(sdl_t *sdl)
{
	SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, 255); // black
	SDL_RenderClear(sdl->renderer); // clear screen with our color.
	SDL_RenderPresent(sdl->renderer); // Show the modifications
}

void handle_sdl_input(chip8_t *chip8)
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_QUIT:
				set_chip8_state(chip8, QUIT);
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.scancode == SDL_SCANCODE_P) {
					if (get_chip8_state(chip8) == PAUSED) {
						SDL_Log("Game resumed");
						set_chip8_state(chip8, RUNNING);
					} else {
						SDL_Log("Game paused");
						set_chip8_state(chip8, PAUSED);
					}
				}
				// Our keyboard is an Azerty, so Q is A
				else if (event.key.keysym.scancode == SDL_SCANCODE_A) {
					set_chip8_state(chip8, QUIT);
				}

				break;
		}
	}
}

void update_sdl_window()
{
	// TODO
}
