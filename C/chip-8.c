#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include <errno.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

const uint32_t CHIP8_WINDOW_WIDTH = 64;
const uint32_t CHIP8_WINDOW_LENGHT = 32;

const uint16_t FONTS_OFFSET = 0x50; // offset in RAM where fonts are loaded
const uint16_t ENTRY_POINT = 0x200;

// Copied from https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#font
const uint8_t FONTS[] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

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
	uint8_t  delay_timer; // decremented by one 60 times per second (60Hz)
	uint8_t  sound_timer;
	uint8_t  vn[16]; // General purpose registers
	uint8_t  sp; // stack pointer
	uint16_t stack[32]; // Let's use a stack outside RAM
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
	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		printf("Failed to open %s: %s\n", filename, strerror(errno));
		return false;
	}

	fseek(f, 0, SEEK_END); // Go at the end of the file
	long fsize = ftell(f); // Get the size of the file
	rewind(f); // Return to the beginning
	if (ftell(f) != 0) {
		printf("Failed to rewind %s: %s\n", filename, strerror(errno));
		fclose(f);
		return false;
	}

	size_t iread = fread(&chip8->ram[ENTRY_POINT], 1, fsize, f);
	if (iread != fsize) {
		printf("Failed to load %s into RAM: %s\n", filename, strerror(errno));
		printf("Read %ld bytes instead of %ld\n", iread, fsize);
		fclose(f);
		return false;
	}

	fclose(f);

	chip8->pc = ENTRY_POINT;

	// For some reason it is popular to put fonts at 0x50 in RAM.
	// So let's do the same.
	memcpy(&chip8->ram[FONTS_OFFSET], FONTS, sizeof(FONTS));

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
