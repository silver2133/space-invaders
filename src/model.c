#include "model.h"
#include <string.h>

// --- tiny RNG (xorshift32) ---
static uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}
static int rnd_range(Game *g, int lo, int hi_inclusive) {
    uint32_t r = xorshift32(&g->rng);
    int span = (hi_inclusive - lo + 1);
    return lo + (int)(r % (uint32_t)span);
}

static void clear_projectiles(Game *g) {
    for (int i = 0; i < MAX_PROJECTILES; i++) g->projectiles[i].active = 0;
}

static void init_shields(Game *g) {
    // 4 shields placed above player area
    int base_y = GAME_H - 6;
    int spacing = GAME_W / (MAX_SHIELDS + 1);
    for (int s = 0; s < MAX_SHIELDS; s++) {
        Shield *sh = &g->shields[s];
        sh->x = spacing * (s + 1) - SHIELD_W / 2;
        sh->y = base_y;
        for (int y = 0; y < SHIELD_H; y++)
            for (int x = 0; x < SHIELD_W; x++)
                sh->hp[y][x] = 2;
    }
}

void game_reset_level(Game *g) {
    // enemies grid
    g->enemy_count = 0;
    int rows = 5;
    int cols = 10;
    int start_x = 10;
    int start_y = 2;
    int dx = 5;
    int dy = 2;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (g->enemy_count >= MAX_ENEMIES) break;
            Enemy *e = &g->enemies[g->enemy_count++];
            e->x = start_x + c * dx;
            e->y = start_y + r * dy;
            e->alive = 1;
        }
    }

    g->enemy_dir = 1;
    g->enemy_step_timer = 0.0f;
    // base speed by level (lower delay = faster)
    g->enemy_step_delay = 0.55f - 0.05f * (float)(g->level - 1);
    if (g->enemy_step_delay < 0.12f) g->enemy_step_delay = 0.12f;

    init_shields(g);
    clear_projectiles(g);
}

void game_init(Game *g) {
    memset(g, 0, sizeof(*g));
    g->player_x = GAME_W / 2;
    g->lives = 3;
    g->score = 0;
    g->level = 1;
    g->paused = 0;
    g->game_over = 0;
    g->rng = 0xC0FFEEu; // deterministic seed (you can change)

    game_reset_level(g);
}

void game_toggle_pause(Game *g) {
    if (g->game_over) return;
    g->paused = !g->paused;
}

void game_move_player(Game *g, int dx) {
    if (g->paused || g->game_over) return;
    g->player_x += dx;
    if (g->player_x < 1) g->player_x = 1;
    if (g->player_x > GAME_W - 2) g->player_x = GAME_W - 2;
}

void game_player_shoot(Game *g) {
    if (g->paused || g->game_over) return;

    // limit: one player projectile on screen (classic-ish)
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (g->projectiles[i].active && g->projectiles[i].from_player)
            return;
    }
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!g->projectiles[i].active) {
            g->projectiles[i].active = 1;
            g->projectiles[i].x = g->player_x;
            g->projectiles[i].y = GAME_H - 2;
            g->projectiles[i].dy = -1;
            g->projectiles[i].from_player = 1;
            return;
        }
    }
}

static void enemy_shoot(Game *g) {
    // probability depending on enemies remaining and level
    // attempt 1 shot per update tick (not per frame), gated by randomness
    int alive = 0;
    for (int i = 0; i < g->enemy_count; i++) alive += g->enemies[i].alive;
    if (alive <= 0) return;

    // chance grows with level, and with fewer enemies remaining
    int chance = 800 - g->level * 40 - (60 - alive) * 6; // lower = more shots
    if (chance < 120) chance = 120;

    if (rnd_range(g, 0, chance) != 0) return;

    // pick a random alive enemy
    int target = rnd_range(g, 0, g->enemy_count - 1);
    for (int k = 0; k < g->enemy_count; k++) {
        int idx = (target + k) % g->enemy_count;
        if (g->enemies[idx].alive) {
            // spawn projectile downward
            for (int p = 0; p < MAX_PROJECTILES; p++) {
                if (!g->projectiles[p].active) {
                    g->projectiles[p].active = 1;
                    g->projectiles[p].x = g->enemies[idx].x;
                    g->projectiles[p].y = g->enemies[idx].y + 1;
                    g->projectiles[p].dy = +1;
                    g->projectiles[p].from_player = 0;
                    return;
                }
            }
            return;
        }
    }
}

static int shield_hit(Game *g, int x, int y) {
    for (int s = 0; s < MAX_SHIELDS; s++) {
        Shield *sh = &g->shields[s];
        int lx = x - sh->x;
        int ly = y - sh->y;
        if (lx >= 0 && lx < SHIELD_W && ly >= 0 && ly < SHIELD_H) {
            if (sh->hp[ly][lx] > 0) {
                sh->hp[ly][lx]--;
                return 1;
            }
        }
    }
    return 0;
}

static void step_enemies(Game *g) {
    int leftmost = 9999, rightmost = -9999, bottom = -9999;
    int alive = 0;

    for (int i = 0; i < g->enemy_count; i++) {
        if (!g->enemies[i].alive) continue;
        alive++;
        if (g->enemies[i].x < leftmost) leftmost = g->enemies[i].x;
        if (g->enemies[i].x > rightmost) rightmost = g->enemies[i].x;
        if (g->enemies[i].y > bottom) bottom = g->enemies[i].y;
    }

    if (alive == 0) return;

    // accelerate as enemies die (classic feel)
    float accel = 1.0f - (float)alive / (float)g->enemy_count; // 0..~1
    float delay = g->enemy_step_delay * (1.0f - 0.55f * accel);
    if (delay < 0.06f) delay = 0.06f;

    g->enemy_step_timer += 0.0f; // timer is managed in game_update

    // check bounds after horizontal move
    int next_left = leftmost + g->enemy_dir;
    int next_right = rightmost + g->enemy_dir;

    if (next_left <= 1 || next_right >= GAME_W - 2) {
        // go down and reverse
        g->enemy_dir *= -1;
        for (int i = 0; i < g->enemy_count; i++)
            if (g->enemies[i].alive) g->enemies[i].y += 1;
    } else {
        for (int i = 0; i < g->enemy_count; i++)
            if (g->enemies[i].alive) g->enemies[i].x += g->enemy_dir;
    }

    // if enemies reach player line -> game over
    if (bottom >= GAME_H - 4) {
        g->game_over = 1;
    }

    (void)delay;
}

void game_update(Game *g, float dt) {
    if (g->paused || g->game_over) return;

    // enemies stepping timer
    // recompute actual delay with acceleration
    int alive = 0;
    for (int i = 0; i < g->enemy_count; i++) alive += g->enemies[i].alive;
    if (alive > 0) {
        float accel = 1.0f - (float)alive / (float)g->enemy_count;
        float delay = g->enemy_step_delay * (1.0f - 0.55f * accel);
        if (delay < 0.06f) delay = 0.06f;

        g->enemy_step_timer += dt;
        while (g->enemy_step_timer >= delay) {
            g->enemy_step_timer -= delay;
            step_enemies(g);
        }
    }

    enemy_shoot(g);

    // move projectiles
    for (int p = 0; p < MAX_PROJECTILES; p++) {
        Projectile *pr = &g->projectiles[p];
        if (!pr->active) continue;

        pr->y += pr->dy;

        // out of bounds
        if (pr->y <= 0 || pr->y >= GAME_H - 1) {
            pr->active = 0;
            continue;
        }

        // shield collision
        if (shield_hit(g, pr->x, pr->y)) {
            pr->active = 0;
            continue;
        }

        // player projectile hits enemy
        if (pr->from_player) {
            for (int i = 0; i < g->enemy_count; i++) {
                Enemy *e = &g->enemies[i];
                if (e->alive && e->x == pr->x && e->y == pr->y) {
                    e->alive = 0;
                    pr->active = 0;
                    g->score += 10;
                    break;
                }
            }
        } else {
            // enemy projectile hits player
            if (pr->y == GAME_H - 2 && pr->x == g->player_x) {
                pr->active = 0;
                g->lives--;
                if (g->lives <= 0) g->game_over = 1;
            }
        }
    }

    // level cleared?
    alive = 0;
    for (int i = 0; i < g->enemy_count; i++) alive += g->enemies[i].alive;
    if (alive == 0) {
        g->level++;
        game_reset_level(g);
    }
}

