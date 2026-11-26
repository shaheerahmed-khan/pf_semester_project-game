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

Requires C++17 and SFML 2.x (`libsfml-dev` on Debian/Ubuntu).

```bash
make
./vice-project-doom        # Linux/WSL
# .\vice-project-doom.exe  # Windows
```

`make clean` removes the executable and object files.

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
│   └── audio/              music and sfx
├── levels/level1.txt       ASCII level map
├── docs/ASSETS.md          Asset tree and cleanup notes
├── src/
│   ├── main.cpp            All tunables, asset loading, game loop
│   ├── level.*             Plain-buffer ASCII map loader and drawing
│   ├── physics.*           Gravity and tile collision helpers
│   ├── player.*            Player movement and drawing
│   └── enemies.*           Enemy AI, projectiles, and drawing
├── Makefile
└── .gitignore
```

`src/config.h` was removed. Every tuning value now lives in `main()` and is
passed to the functions that need it.

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

Two SFML calls are kept because the window cannot function without them:
`window.isOpen()` and `window.pollEvent()`. SFML has no other way to receive
`Event::Closed` or decide when to stop the loop.

## Credits

- Sprites from The Spriters Resource and course-provided assets
- SFML 2.6
- Game concept and reference gameplay from CS-1002, Fall 2026
