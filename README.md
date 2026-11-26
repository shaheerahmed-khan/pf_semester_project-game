# VICE: Project Doom

A 2D side-scrolling action platformer for the CS-1002 Programming Fundamentals
project. You play as Detective Quinn Hart in a dystopian city overrun by
criminals and mutants.

Built by: Shaheer · Tehreem · Maryam

## Current build

- One playable level loaded from `levels/level1.txt`
- Right/Space platforming with gravity, tile collision, pits, and respawn
- Seven enemy types: crawler, jumper, flying insect, leopard, green shooter,
  tribal warrior, and leaping fish
- Enemy contact/bullet damage, temporary invulnerability, and health reset
- Flip-screen-style layout with a 1136x896 window

Weapons, pickups, scoring, additional levels, and music playback are planned
but are not part of the current build.

## Build and run

Requires C++17 and SFML 2.6 (`libsfml-dev` on Ubuntu/Debian).

```bash
sudo apt-get install g++ libsfml-dev
make
./vice-project-doom
```

The Makefile compiles the whole game from one translation unit:

```bash
g++ -std=c++17 -Wall -Wextra main.cpp -o vice-project-doom \
    -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
```

`make clean` removes the executable.

Run from the repository root so paths such as `assets/backgrounds/level1.png`
and `levels/level1.txt` resolve.

## Controls

| Action | Key |
|---|---|
| Move right | `Right` |
| Jump | `Space` |
| Close window | Window close button |

There is deliberately no left movement: the allowed SFML input list names only
`Keyboard::Right`. `Space` is the agreed jump binding. The old debug keys
`1`–`7` and the `Esc` shortcut were removed.

## Project layout

```text
pf_semester_project-game/
├── assets/                 Runtime art, tiles, and audio
│   ├── backgrounds/
│   ├── sprites/            player, enemies, bosses, fx, items, ui
│   ├── tiles/
│   └── music/              music and sfx
├── levels/                 ASCII level maps
├── main.cpp                Entire game: level, physics, player, enemies, main
├── Makefile
└── .gitignore
```

`main.cpp` is the only translation unit; there are no project headers or
separate `.cpp` files. Every tuning value lives in `main()` and is passed to
the functions in the same file.

## Level format

Each level is a text map. A screen is 14 rows tall and 18 columns wide, and
screens can be placed side by side.

| Character | Meaning |
|---|---|
| `.` | Empty cell |
| `#` | Solid block |
| `P` | Player spawn (replaced with empty when loaded) |

The loader currently treats only `#` as solid. The tile legends in
`assets/tiles/` describe the extra tile types reserved for later levels.

## Submission constraints

The current code follows the course restrictions:

- The whole game is one translation unit (`main.cpp`); the Makefile
  compiles it directly.
- No global variables and no global constants. All numbers live in `main()`.
- Plain arrays only; no `std::vector`, `std::string`, or other containers.
- The level loader uses `FILE`, `fgets`, and fixed `char` buffers instead of
  `std::ifstream`.
- `Sprite::setTexture` is called with one argument. `setTextureRect(IntRect())`
  resets the crop explicitly instead of using `setTexture(texture, true)`.
- `window.setFramerateLimit` is gone; a `Clock` paces the loop instead.
- Debug enemy-spawn key events and the `Esc` key event were removed. The event
  loop only checks `Event::Closed`.
- No `Music` or `Sound` playback is used, because the strict SFML subset does
  not include it. Audio assets are kept for later.

`window.isOpen()` and `window.pollEvent()` are kept because SFML has no other
way to receive `Event::Closed` or decide when to stop the loop.
`Clock::getElapsedTime().asMilliseconds()` is used only to pace frames, and
`Space` is the agreed jump binding.

## Credits

- Sprites from The Spriters Resource and course-provided assets
- SFML 2.6
- Game concept and reference gameplay from CS-1002, Fall 2026
