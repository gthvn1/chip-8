#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "chip8.h"

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

struct _chip8_t {
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
};

typedef uint16_t insn_t;

typedef struct {
	uint8_t value; // The 4 upper bits
	uint16_t nnn; // Address on 12-bit
	uint8_t nn;   // 8-bit constant
	uint8_t n;    // 4-bit constant
	uint8_t x;    // 4-bit register identifier
	uint8_t y;    // 4-bit register identifier
} opcode_t;

void set_chip8_state(chip8_t *chip8, chip8_state_t state)
{
	chip8->state = state;
}

chip8_state_t get_chip8_state(chip8_t *chip8)
{
	return chip8->state;
}

// CHIP8 functions
chip8_t *init_chip8(char *filename)
{
	chip8_t *chip8 = calloc(1, sizeof(chip8_t));
	if (chip8 == NULL) {
		printf("Failed to allocate memory for chip8\n");
		return NULL;
	}

	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		printf("Failed to open %s: %s\n", filename, strerror(errno));
		goto free_chip8;
	}

	fseek(f, 0, SEEK_END); // Go at the end of the file

	long fsize = ftell(f); // Get the size of the file
	if (fsize > (RAM_SIZE - ENTRY_POINT)) {
		printf("Code is too big (%ld > %d)\n", fsize, RAM_SIZE - ENTRY_POINT);
		goto free_chip8;
	}

	rewind(f); // Return to the beginning
	if (ftell(f) != 0) {
		printf("Failed to rewind %s: %s\n", filename, strerror(errno));
		goto free_chip8;
	}

	size_t iread = fread(&chip8->ram[ENTRY_POINT], 1, fsize, f);
	if (iread != fsize) {
		printf("Failed to load %s into RAM: %s\n", filename, strerror(errno));
		printf("Read %ld bytes instead of %ld\n", iread, fsize);
		goto free_chip8;
	}

	fclose(f);

	chip8->pc = ENTRY_POINT;
	chip8->code_size = (uint16_t)fsize;

	// For some reason it is popular to put fonts at 0x50 in RAM.
	// So let's do the same.
	memcpy(&chip8->ram[FONTS_OFFSET], FONTS, sizeof(FONTS));

	return chip8;

free_chip8:
	if (f) {
		fclose(f);
	}
	free(chip8);
	return NULL;
}

void cleanup_chip8(chip8_t *chip8)
{
	if (chip8) {
		free(chip8);
		chip8 = NULL;
	}
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
				printf("PC[0x%04x] | insn: 0x%04x: Clear screen not implemented\n",
					pc, insn);
			} else if (insn == 0x00EE) {
				printf("PC[0x%04x] | insn: 0x%04x: Return from subroutine not implemented\n",
					pc, insn);
			} else {
				printf("PC[0x%04x] | insn: 0x%04x: Opcode not implemented\n",
					pc, insn);
			}
			break;
		case 0x1:
			printf("PC[0x%04x] | insn: 0x%04x: Jump to 0x%04x not implemented\n",
				pc, insn, opcode.nnn);

			break;
		case 0x2:
			printf("PC[0x%04x] | insn: 0x%04x: Opcode not implemented\n", pc, insn);
			break;
		case 0x3:
			printf("PC[0x%04x] | insn: 0x%04x: Opcode not implemented\n", pc, insn);
			break;
		case 0x4:
			printf("PC[0x%04x] | insn: 0x%04x: Opcode not implemented\n", pc, insn);
			break;
		case 0x5:
			printf("PC[0x%04x] | insn: 0x%04x: Opcode not implemented\n", pc, insn);
			break;
		case 0x6:
			printf("PC[0x%04x] | insn: 0x%04x: Set V%x to 0x%04x Opcode not implemented\n",
				pc, insn, opcode.x, opcode.nn);
			break;
		case 0x7:
			printf("PC[0x%04x] | insn: 0x%04x: Adds 0x%04x to V%x not implemented\n",
				pc, insn, opcode.nn, opcode.x);
			break;
		case 0x8:
			printf("PC[0x%04x] | insn: 0x%04x: Opcode not implemented\n", pc, insn);
			break;
		case 0x9:
			printf("PC[0x%04x] | insn: 0x%04x: Opcode not implemented\n", pc, insn);
			break;
		case 0xA:
			printf("PC[0x%04x] | insn: 0x%04x: Set I to 0x%04x not implemented\n",
				pc, insn, opcode.nnn);
			break;
		case 0xB:
			printf("PC[0x%04x] | insn: 0x%04x: Opcode not implemented\n", pc, insn);
			break;
		case 0xC:
			printf("PC[0x%04x] | insn: 0x%04x: Opcode not implemented\n", pc, insn);
			break;
		case 0xD:
			printf("PC[0x%04x] | insn: 0x%04x: Draw at (V%x, V%x, height %u) not implemented\n",
				pc, insn, opcode.x, opcode.y, opcode.n);
			break;
		case 0xE:
			printf("PC[0x%04x] | insn: 0x%04x: Opcode not implemented\n", pc, insn);
			break;
		case 0xF:
			printf("PC[0x%04x] | insn: 0x%04x: Opcode not implemented\n", pc, insn);
			break;
		default:
			printf("PC[0x%04x] | insn: 0x%04x: Invalid Opcode\n", pc, insn);
			break;
	}
}
