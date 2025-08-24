#pragma once

#include <stdbool.h>

typedef enum {
    gaming = 0,
    pause = 1,
    menu = 2,
} Screen;

typedef struct {
    Screen curr_screen;
    bool running;
} GameContext;

