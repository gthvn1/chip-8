#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include <errno.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

#define CHIP8_WINDOW_WIDTH  64
#define CHIP8_WINDOW_LENGHT 32

#define RAM_SIZE     0x1000 // 4 KB
#define STACK_SIZE   32
#define FONTS_OFFSET 0x50 // offset in RAM where fonts are loaded
#define ENTRY_POINT  0x200

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
	OK = 0,
	INIT_SDL_ERROR,
	INIT_CHIP8_ERROR,
	ROM_FILENAME_IS_MISSING,
} error_t;

typedef enum {
	RUNNING = 0,
	PAUSED,
	QUIT,
} chip8_state_t;

typedef uint16_t insn_t;

typedef struct {
	uint8_t value; // The 4 upper bits
	uint16_t nnn; // Address on 12-bit
	uint8_t nn;   // 8-bit constant
	uint8_t n;    // 4-bit constant
	uint8_t x;    // 4-bit register identifier
	uint8_t y;    // 4-bit register identifier
} opcode_t;

typedef struct {
	uint8_t  ram[RAM_SIZE]; // 4 KB RAM
	uint16_t pc; // Program Counter
	uint16_t i;  // Index register (point at location in memory)
	uint8_t  delay_timer; // decremented by one 60 times per second (60Hz)
	uint8_t  sound_timer;
	uint8_t  vn[16]; // General purpose registers
	uint8_t  sp; // stack pointer
	uint16_t stack[STACK_SIZE]; // Let's use a stack outside RAM
	chip8_state_t state;
	uint16_t code_size; // Used to check that PC is valid
} chip8_t;

typedef struct {
	uint8_t scale;
	SDL_Window *window;
	SDL_Renderer *renderer;
} sdl_t;

// SDL functions
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

/* Set default values */
bool init_sdl(sdl_t *sdl, const char *title)
{
	sdl->scale = 10;
	SDL_Init(SDL_INIT_VIDEO); // init SDL2

	if (!create_window(sdl, title)) {
		return false;
	}

	if (!create_renderer(sdl)) {
		SDL_DestroyWindow(sdl->window);
		return false;
	}

	return true;
}

void cleanup_sdl(sdl_t *sdl)
{
	SDL_DestroyRenderer(sdl->renderer);
	SDL_DestroyWindow(sdl->window);
	SDL_Quit();
}

void handle_sdl_input(chip8_t *chip8)
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_QUIT:
				chip8->state = QUIT;
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.scancode == SDL_SCANCODE_P) {
					if (chip8->state == PAUSED) {
						SDL_Log("Game resumed");
						chip8->state = RUNNING;
					} else {
						SDL_Log("Game paused");
						chip8->state = PAUSED;
					}
				}
				// Our keyboard is an Azerty, so Q is A
				else if (event.key.keysym.scancode == SDL_SCANCODE_A) {
					chip8->state = QUIT;
				}

				break;
		}
	}
}

void update_sdl_window()
{
	// TODO
}

// CHIP8 functions
bool init_chip8(chip8_t *chip8, char *filename)
{
	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		printf("Failed to open %s: %s\n", filename, strerror(errno));
		return false;
	}

	fseek(f, 0, SEEK_END); // Go at the end of the file
	//
	long fsize = ftell(f); // Get the size of the file
	if (fsize > (RAM_SIZE - ENTRY_POINT)) {
		printf("Code is too big (%ld > %d)\n", fsize, RAM_SIZE - ENTRY_POINT);
		fclose(f);
		return false;
	}

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
	chip8->code_size = (uint16_t)fsize;

	// For some reason it is popular to put fonts at 0x50 in RAM.
	// So let's do the same.
	memcpy(&chip8->ram[FONTS_OFFSET], FONTS, sizeof(FONTS));

	return true;
}

void emulate_chip8(chip8_t *chip8)
{
	insn_t insn;
	opcode_t opcode;
	uint16_t pc; // Keep current pc for logging

	// Fetch
	pc = chip8->pc;
	insn = chip8->ram[pc] << 8 | chip8->ram[pc + 1];

	// TODO: it is not really clean. But currently the execute part
	//       just display the opcode and add the PC. So when we reach
	//       the end of the code we just stop displaying info.
	if (pc + 2 > (ENTRY_POINT + chip8->code_size)) {
		return;
	}

	chip8->pc += 2;

	// Decode
	opcode.value = (insn >> 12) & 0xF;
	opcode.nnn   = insn & 0x0FFF;
	opcode.nn    = insn & 0x00FF;
	opcode.n     = insn & 0x000F;
	opcode.x     = (insn >> 8) & 0xF;
	opcode.y     = (insn >> 4) & 0xF;

	// Execute: opcode fields interpretation depends of the value
	switch (opcode.value) {
		case 0x0:
			if (insn == 0x00E0) {
				SDL_Log("PC[0x%04x] | insn: 0x%04x: Clear screen not implemented",
					pc, insn);
			} else if (insn == 0x00EE) {
				SDL_Log("PC[0x%04x] | insn: 0x%04x: Return from subroutine not implemented",
					pc, insn);
			} else {
				SDL_Log("PC[0x%04x] | insn: 0x%04x: Opcode not implemented",
					pc, insn);
			}
			break;
		case 0x1:
			SDL_Log("PC[0x%04x] | insn: 0x%04x: Jump to 0x%04x not implemented",
				pc, insn, opcode.nnn);

			break;
		case 0x2:
			SDL_Log("PC[0x%04x] | insn: 0x%04x: Opcode not implemented", pc, insn);
			break;
		case 0x3:
			SDL_Log("PC[0x%04x] | insn: 0x%04x: Opcode not implemented", pc, insn);
			break;
		case 0x4:
			SDL_Log("PC[0x%04x] | insn: 0x%04x: Opcode not implemented", pc, insn);
			break;
		case 0x5:
			SDL_Log("PC[0x%04x] | insn: 0x%04x: Opcode not implemented", pc, insn);
			break;
		case 0x6:
			SDL_Log("PC[0x%04x] | insn: 0x%04x: Set V%x to 0x%04x Opcode not implemented\n",
				pc, insn, opcode.x, opcode.nn);
			break;
		case 0x7:
			SDL_Log("PC[0x%04x] | insn: 0x%04x: Adds 0x%04x to V%x not implemented\n",
				pc, insn, opcode.nn, opcode.x);
			break;
		case 0x8:
			SDL_Log("PC[0x%04x] | insn: 0x%04x: Opcode not implemented", pc, insn);
			break;
		case 0x9:
			SDL_Log("PC[0x%04x] | insn: 0x%04x: Opcode not implemented", pc, insn);
			break;
		case 0xA:
			SDL_Log("PC[0x%04x] | insn: 0x%04x: Set I to 0x%04x not implemented\n",
				pc, insn, opcode.nnn);
			break;
		case 0xB:
			SDL_Log("PC[0x%04x] | insn: 0x%04x: Opcode not implemented", pc, insn);
			break;
		case 0xC:
			SDL_Log("PC[0x%04x] | insn: 0x%04x: Opcode not implemented", pc, insn);
			break;
		case 0xD:
			SDL_Log("PC[0x%04x] | insn: 0x%04x: Draw at (V%x, V%x, height %u) not implemented\n",
				pc, insn, opcode.x, opcode.y, opcode.n);
			break;
		case 0xE:
			SDL_Log("PC[0x%04x] | insn: 0x%04x: Opcode not implemented", pc, insn);
			break;
		case 0xF:
			SDL_Log("PC[0x%04x] | insn: 0x%04x: Opcode not implemented", pc, insn);
			break;
		default:
			SDL_Log("PC[0x%04x] | insn: 0x%04x: Invalid Opcode", pc, insn);
			break;
	}
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

	if (!init_sdl(&sdl, title))
		return INIT_SDL_ERROR;

	SDL_SetRenderDrawColor(sdl.renderer, 0, 0, 0, 255); // black
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

	// Main loop
	while (chip8.state != QUIT) {
		// TODO:
		//   - Handle SDL input
		//   - Emulate CHIP8 insns
		//   - Add delay to simulate 60Hz
		//     => 1/60 = 0.01667s ~ 16.67ms of delay
		//   - Update SDL window
		handle_sdl_input(&chip8);

		if (chip8.state == PAUSED)
			continue;

		emulate_chip8(&chip8);
		SDL_Delay(17);
		update_sdl_window();
	}

	cleanup_sdl(&sdl);
	return 0;
}
