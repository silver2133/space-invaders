#pragma once
#include "view.h"

int sdl_init(void);
void sdl_render(const Game *g);
Command sdl_poll_event(void);
void sdl_cleanup(void);
