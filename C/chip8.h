#ifndef CHIP8_H
#define CHIP8_H

typedef struct _chip8_t chip8_t;

typedef enum {
	RUNNING = 0,
	PAUSED,
	QUIT,
} chip8_state_t;

// CHIP8 functions
chip8_t *init_chip8(char *filename);
void cleanup_chip8(chip8_t *chip8);

void emulate_chip8(chip8_t *chip8);

void set_chip8_state(chip8_t *chip8, chip8_state_t state);
chip8_state_t get_chip8_state(chip8_t *chip8);

#endif