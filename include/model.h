#pragma once
#include <stdint.h>

#define GAME_W  80
#define GAME_H  24

#define MAX_ENEMIES      60
#define MAX_PROJECTILES  128
#define MAX_SHIELDS      4
#define SHIELD_W         8
#define SHIELD_H         3

typedef struct {
    int x, y;
    int alive;
} Enemy;

typedef struct {
    int x, y;
    int active;
    int dy;          // -1 player, +1 enemy
    int from_player; // 1 player, 0 enemy
} Projectile;

typedef struct {
    int x, y;
    int hp[SHIELD_H][SHIELD_W]; // 0..2 (2 = intact)
} Shield;

typedef struct {
    // player
    int player_x;
    int lives;

    // progression
    int score;
    int level;

    // state flags
    int paused;
    int game_over;

    // enemies
    Enemy enemies[MAX_ENEMIES];
    int enemy_count;
    int enemy_dir;           // -1 / +1
    float enemy_step_timer;  // accum
    float enemy_step_delay;  // seconds between steps

    // shields
    Shield shields[MAX_SHIELDS];

    // projectiles
    Projectile projectiles[MAX_PROJECTILES];

    // rng
    uint32_t rng;
} Game;

void game_init(Game *g);
void game_reset_level(Game *g);

void game_toggle_pause(Game *g);
void game_move_player(Game *g, int dx);
void game_player_shoot(Game *g);

void game_update(Game *g, float dt);
