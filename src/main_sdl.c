#include "view.h"
#include "controller.h"
#include <stdio.h>
#include <time.h>

static void sleep_seconds(double sec) {
    if (sec <= 0) return;
    struct timespec ts;
    ts.tv_sec = (time_t)sec;
    ts.tv_nsec = (long)((sec - (double)ts.tv_sec) * 1000000000.0);
    nanosleep(&ts, NULL);
}

static double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

int main(void) {
    View *view = &view_sdl;

    Game g;
    game_init(&g);

    if (view->init() != 0) {
        fprintf(stderr, "View init failed.\n");
        return 1;
    }

    const double dt = 1.0 / 60.0;
    double acc = 0.0;
    double prev = now_seconds();

    while (!g.game_over) {
        Command cmd = view->poll_event();
        if (cmd == CMD_QUIT) break;
        controller_handle(&g, cmd);

        double t = now_seconds();
        double frame = t - prev;
        if (frame > 0.25) frame = 0.25;
        prev = t;
        acc += frame;

        while (acc >= dt) {
            game_update(&g, (float)dt);
            acc -= dt;
        }

        view->render(&g);
        sleep_seconds(0.001);
    }

    view->cleanup();
    return 0;
}
