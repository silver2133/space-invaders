#pragma once
#include "view.h"

int nc_init(void);
void nc_render(const Game *g);
Command nc_poll_event(void);
void nc_cleanup(void);
