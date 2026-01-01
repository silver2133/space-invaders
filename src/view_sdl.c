#include "view_sdl.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>

static SDL_Window   *win = NULL;
static SDL_Renderer *ren = NULL;

static const int SCALE = 12;
static const int WIN_W = GAME_W * SCALE;
static const int WIN_H = GAME_H * SCALE;

static void log_sdl_error(const char *where)
{
    const char *err = SDL_GetError();
    if (err && *err) fprintf(stderr, "[SDL] %s: %s\n", where, err);
    else             fprintf(stderr, "[SDL] %s: (no error string)\n", where);
}

int sdl_init(void)
{
    fprintf(stderr, "DISPLAY=%s\n", getenv("DISPLAY") ? getenv("DISPLAY") : "(null)");
    fprintf(stderr, "WAYLAND_DISPLAY=%s\n", getenv("WAYLAND_DISPLAY") ? getenv("WAYLAND_DISPLAY") : "(null)");
    fprintf(stderr, "SDL_VIDEODRIVER=%s\n", getenv("SDL_VIDEODRIVER") ? getenv("SDL_VIDEODRIVER") : "(null)");

    const int force_sw = (getenv("SDL_FORCE_SOFTWARE") != NULL);

    // SDL3: SDL_Init returns bool (true on success)
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        log_sdl_error("SDL_Init(SDL_INIT_VIDEO) failed");
        return 1;
    }

    win = SDL_CreateWindow("Space Invaders (SDL3)", WIN_W, WIN_H, SDL_WINDOW_RESIZABLE);
    if (!win) {
        log_sdl_error("SDL_CreateWindow failed");
        SDL_Quit();
        return 2;
    }

    if (!force_sw) {
        ren = SDL_CreateRenderer(win, NULL);
        if (!ren) {
            log_sdl_error("SDL_CreateRenderer(NULL) failed (trying software fallback)");
        }
    }

    if (!ren) {
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
        ren = SDL_CreateRenderer(win, "software");
        if (!ren) {
            log_sdl_error("SDL_CreateRenderer(\"software\") failed");
            SDL_DestroyWindow(win);
            win = NULL;
            SDL_Quit();
            return 3;
        }
    }

    return 0;
}

static void draw_rect(int x, int y, int w, int h)
{
    SDL_FRect r = { (float)x, (float)y, (float)w, (float)h };
    SDL_RenderFillRect(ren, &r);
}


static void render_shields(const Game *g)
{
    for (int s = 0; s < MAX_SHIELDS; s++) {
        const Shield *sh = &g->shields[s];
        for (int y = 0; y < SHIELD_H; y++) {
            for (int x = 0; x < SHIELD_W; x++) {
                if (sh->hp[y][x] <= 0) continue;
                draw_rect((sh->x + x) * SCALE, (sh->y + y) * SCALE, SCALE, SCALE);
            }
        }
    }
}

void sdl_render(const Game *g)
{
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);

    SDL_SetRenderDrawColor(ren, 40, 40, 40, 255);
    draw_rect(0, 0, WIN_W, SCALE);

    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    draw_rect(g->player_x * SCALE, (GAME_H - 2) * SCALE, SCALE, SCALE);

    SDL_SetRenderDrawColor(ren, 200, 200, 200, 255);
    for (int i = 0; i < g->enemy_count; i++) {
        if (!g->enemies[i].alive) continue;
        draw_rect(g->enemies[i].x * SCALE, g->enemies[i].y * SCALE, SCALE, SCALE);
    }

    SDL_SetRenderDrawColor(ren, 120, 200, 120, 255);
    render_shields(g);

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!g->projectiles[i].active) continue;

        if (g->projectiles[i].from_player) SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        else SDL_SetRenderDrawColor(ren, 255, 120, 120, 255);

        draw_rect(g->projectiles[i].x * SCALE + SCALE/3,
                  g->projectiles[i].y * SCALE,
                  SCALE/3, SCALE);
    }

    SDL_RenderPresent(ren);
}

Command sdl_poll_event(void)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) return CMD_QUIT;

        if (e.type == SDL_EVENT_KEY_DOWN) {
            SDL_Scancode sc = e.key.scancode;
            switch (sc) {
                case SDL_SCANCODE_Q:     return CMD_QUIT;
                case SDL_SCANCODE_P:     return CMD_PAUSE;
                case SDL_SCANCODE_LEFT:  return CMD_MOVE_LEFT;
                case SDL_SCANCODE_RIGHT: return CMD_MOVE_RIGHT;
                case SDL_SCANCODE_SPACE: return CMD_SHOOT;
                default: break;
            }
        }
    }
    return CMD_NONE;
}

void sdl_cleanup(void)
{
    if (ren) SDL_DestroyRenderer(ren);
    if (win) SDL_DestroyWindow(win);
    ren = NULL;
    win = NULL;
    SDL_Quit();
}

View view_sdl = {
    .init       = sdl_init,
    .render     = sdl_render,
    .poll_event = sdl_poll_event,
    .cleanup    = sdl_cleanup
};
