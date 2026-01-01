CC      := gcc
CFLAGS  := -Wall -Wextra -std=c99 -Iinclude -D_POSIX_C_SOURCE=200809L

SRC_COMMON := src/model.c src/controller.c
SRC_NC     := src/main_ncurses.c src/view_ncurses.c
SRC_SDL    := src/main_sdl.c src/view_sdl.c

NCURSES_LIBS := -lncurses

SDL_CFLAGS := $(shell pkg-config --cflags sdl3 2>/dev/null)
SDL_LIBS   := $(shell pkg-config --libs sdl3 2>/dev/null)

ifeq ($(SDL_LIBS),)
SDL_CFLAGS := -I/usr/local/include
SDL_LIBS   := -L/usr/local/lib -Wl,-rpath,/usr/local/lib -lSDL3
endif

BIN_NC  := invaders_ncurses
BIN_SDL := invaders_sdl

.PHONY: all ncurses sdl run-ncurses run-sdl clean valgrind

all: ncurses sdl

ncurses:
	$(CC) $(CFLAGS) $(SRC_COMMON) $(SRC_NC) $(NCURSES_LIBS) -o $(BIN_NC)

sdl:
	$(CC) $(CFLAGS) $(SRC_COMMON) $(SRC_SDL) $(SDL_CFLAGS) $(SDL_LIBS) -o $(BIN_SDL)

run-ncurses: ncurses
	./$(BIN_NC)

run-sdl: sdl
	SDL_VIDEODRIVER=x11 ./$(BIN_SDL)

valgrind: ncurses
	valgrind --leak-check=full --show-leak-kinds=all ./$(BIN_NC)

clean:
	rm -f $(BIN_NC) $(BIN_SDL)
