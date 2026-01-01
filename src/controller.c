#include "controller.h"

void controller_handle(Game *g, Command cmd) {
    switch (cmd) {
        case CMD_MOVE_LEFT:  game_move_player(g, -1); break;
        case CMD_MOVE_RIGHT: game_move_player(g, +1); break;
        case CMD_SHOOT:      game_player_shoot(g); break;
        case CMD_PAUSE:      game_toggle_pause(g); break;
        default: break;
    }
}
