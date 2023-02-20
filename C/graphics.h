#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>

#include "chip8.h"

#define CHIP8_WINDOW_WIDTH  64
#define CHIP8_WINDOW_LENGHT 32

typedef struct _sdl_t sdl_t;

// SDL functions
sdl_t *init_sdl(const char *title);
void cleanup_sdl(sdl_t *sdl);

void handle_sdl_input(chip8_t *chip8);
void draw_black_screen(sdl_t *sdl);
void update_sdl_window();

#endif