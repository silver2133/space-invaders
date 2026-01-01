#include "view_ncurses.h"
#include <ncurses.h>

static void draw_borders(void) {
    for (int x = 0; x < GAME_W; x++) {
        mvaddch(1, x, '-');
        mvaddch(GAME_H - 1, x, '-');
    }
    for (int y = 1; y < GAME_H; y++) {
        mvaddch(y, 0, '|');
        mvaddch(y, GAME_W - 1, '|');
    }
}

int nc_init(void) {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    timeout(0);

    if (COLS < GAME_W || LINES < GAME_H) {
        endwin();
        return 1;
    }
    return 0;
}

static void render_shields(const Game *g) {
    for (int s = 0; s < MAX_SHIELDS; s++) {
        const Shield *sh = &g->shields[s];
        for (int y = 0; y < SHIELD_H; y++) {
            for (int x = 0; x < SHIELD_W; x++) {
                int hp = sh->hp[y][x];
                if (hp <= 0) continue;
                char c = (hp == 2) ? '#' : '+';
                int rx = sh->x + x;
                int ry = sh->y + y;
                if (rx > 0 && rx < GAME_W - 1 && ry > 1 && ry < GAME_H - 1)
                    mvaddch(ry, rx, c);
            }
        }
    }
}

void nc_render(const Game *g) {
    erase();

    mvprintw(0, 0, "Space Invaders (ncurses)  Score:%d  Lives:%d  Level:%d  [q quit, p pause, space shoot]",
             g->score, g->lives, g->level);

    draw_borders();

    // player
    mvaddch(GAME_H - 2, g->player_x, 'A');

    // enemies
    for (int i = 0; i < g->enemy_count; i++) {
        if (!g->enemies[i].alive) continue;
        mvaddch(g->enemies[i].y, g->enemies[i].x, 'W');
    }

    // shields
    render_shields(g);

    // projectiles
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!g->projectiles[i].active) continue;
        mvaddch(g->projectiles[i].y, g->projectiles[i].x,
                g->projectiles[i].from_player ? '|' : '!');
    }

    if (g->paused) {
        mvprintw(GAME_H / 2, (GAME_W / 2) - 6, "[ PAUSED ]");
    }
    if (g->game_over) {
        mvprintw(GAME_H / 2, (GAME_W / 2) - 6, "[ GAME OVER ]");
    }

    refresh();
}

Command nc_poll_event(void) {
    int ch = getch();
    switch (ch) {
        case 'q': return CMD_QUIT;
        case 'p': return CMD_PAUSE;
        case KEY_LEFT:  return CMD_MOVE_LEFT;
        case KEY_RIGHT: return CMD_MOVE_RIGHT;
        case ' ': return CMD_SHOOT;
        default: return CMD_NONE;
    }
}

void nc_cleanup(void) {
    endwin();
}

View view_ncurses = {
    .init = nc_init,
    .render = nc_render,
    .poll_event = nc_poll_event,
    .cleanup = nc_cleanup
};
