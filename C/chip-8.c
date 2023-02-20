#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

const uint32_t CHIP8_WINDOW_WIDTH = 64;
const uint32_t CHIP8_WINDOW_LENGHT = 32;

typedef enum {
	CREATE_WINDOW_ERROR = 1,
	CREATE_RENDERER_ERROR,
	ROM_FILENAME_IS_MISSING,
	INIT_CHIP8_ERROR,
} chip8_error;

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
void init_sdl(sdl_t *sdl)
{
	sdl->scale = 10;
	SDL_Init(SDL_INIT_VIDEO); // init SDL2
}

void cleanup_sdl(sdl_t *sdl)
{
	SDL_DestroyWindow(sdl->window);
	SDL_DestroyRenderer(sdl->renderer);
	SDL_Quit();
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

bool init_chip8(chip8_t *chip8, char *filename)
{
	// TODO:
	// - Open the file for reading
	// - Load the content of the file in RAM at 0x200
	// - Set the PC to 0x200
	// - Set the SP ???
	// - Set the 2 delay timers ???
	return true;
}

void help(char *progname)
{
	printf("USAGE: %s <rom>\n", progname);
}

int main(int argc, char **argv)
{
	const char *title = "Chip-8 emulator";
	chip8_t chip8 = {0};
	sdl_t sdl = {0};

	init_sdl(&sdl); // Set default value and init SDL2

	if (!create_window(&sdl, title)) {
		return CREATE_WINDOW_ERROR;
	}

	if (!create_renderer(&sdl)) {
		SDL_DestroyWindow(sdl.window);
		return CREATE_RENDERER_ERROR;
	}

	SDL_SetRenderDrawColor(sdl.renderer, 255, 0, 0, 255); // Red (will be black soon)
	SDL_RenderClear(sdl.renderer); // clear screen with our color.
	SDL_RenderPresent(sdl.renderer); // Show the modifications

	// check if there is at least one argument and init chip8
	if (argc < 2) {
		cleanup_sdl(&sdl);
		help(argv[0]);
		return ROM_FILENAME_IS_MISSING;
	}
	if (!init_chip8(&chip8, argv[1])) {
		cleanup_sdl(&sdl);
		return INIT_CHIP8_ERROR;
	}

	// Before closing wait 3 Seconds...
	// The event loop will soon replace this delay
	SDL_Delay(3000);

	cleanup_sdl(&sdl);
	return 0;
}
