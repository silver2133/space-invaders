#pragma once
#include "model.h"
#include "commands.h"

typedef struct View {
    int  (*init)(void);               
    void (*render)(const Game *g);
    Command (*poll_event)(void);
    void (*cleanup)(void);
} View;

extern View view_ncurses;
extern View view_sdl;
