# Space Invaders – C (ncurses & SDL3)

Projet Space Invaders développé en C, proposant deux modes d’affichage :
- "ncurses" : mode terminal (stable, recommandé)
- "SDL3" : mode graphique (fenêtre)

Le projet est organisé selon une architecture Model / View / Controller (MVC) afin de séparer la logique du jeu, l’affichage et les entrées utilisateur.

---

## Structure du projet

space-invaders/
├── include/ # Fichiers headers (.h)
│ ├── commands.h
│ ├── controller.h
│ ├── model.h
│ ├── view.h
│ ├── view_ncurses.h
│ └── view_sdl.h
│
├── src/ # Fichiers sources (.c)
│ ├── controller.c
│ ├── model.c
│ ├── main_ncurses.c
│ ├── main_sdl.c
│ ├── view_ncurses.c
│ └── view_sdl.c
│
├── vendors/
│ └── SDL/ # SDL3 (vendorisée)
│
├── Makefile
├── .gitignore
└── README.md


---

## Prérequis

### Système
- Linux
- ou WSL2 (Ubuntu 22.04 recommandé)

### Outils nécessaires
```bash
sudo apt update
sudo apt install -y build-essential pkg-config ncurses-dev

Installation locale de SDL3 (si nécessaire)
cd vendors/SDL
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build


Vérification :

pkg-config --modversion sdl3

Compilation:
  make clean


### Lancer le jeu
Mode ncurses:
Fonctionne partout, y compris sous WSL sans configuration graphique.
  make run-ncurses

### Mode SDL3 (graphique)
Sous Linux natif:
make run-sdl

Sous WSL (recommandé)

Sous WSL, l’accélération OpenGL nous a posé problème.
nous avons dû utiliser le renderer software :

SDL_VIDEODRIVER=x11 SDL_RENDER_DRIVER=software make run-sdl

Ou directement :

SDL_VIDEODRIVER=x11 SDL_RENDER_DRIVER=software ./invaders_sdl
