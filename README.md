# VICE: Project Doom

A 2D side-scrolling action platformer written in C++ with SFML 2. This group project is for **CS-1002 Programming Fundamentals (Fall 2026)** and emphasizes secure, responsible software development.

**Collaborators:** Shaheer · Tehreem · Maryam

## Game

Play as Detective Quinn Hart, a VICE detective fighting criminals and mutants across three levels. The game uses **flip-screen scrolling**: the view changes to the next static screen as the player crosses an edge.

### Player
- Move, jump, duck, and run while crouching
- Climb ladders
- Swing on hanging vines in Level 3

### Weapons

| Weapon | Ammo | Damage / use |
|---|---:|---|
| Laser Whip | Unlimited | Close-range attack |
| .44 Magnum | 10 bullets per level | 10 HP per shot |
| Grenades | 3 per level | 40 HP per explosion |

Ammo, grenades, and health can be replenished with pickups. Only the selected weapon is active.

### Enemies and levels

| Level | Setting | Enemies and features |
|---|---|---|
| 1 | Construction Site | 4 spiked crawlers, 5 blue jumping enemies, 6 flying insects; ladders, girders, and sequential random spawns |
| 2 | Ricardo Port | 10 leopards, 4 green shooters, 8 tribal warriors; platforms, containers, and meshes |
| 3 | Cliffs, Waterfalls and Jungle | 10 leaping fish, 8 tribal warriors, 10 leopards, 7 flying insects, 5 blue jumping enemies; swinging, destructible, and regrowing vines |

Enemy types include ground patrols, jumpers, flying insects, leopards, stationary shooters, shielded warriors, and leaping fish. Their behaviors and defeat rules are being finalized in `docs/PLAN.md`.

### Pickups, scoring, and progression

- A spiked crawler may drop ammo or a temporary Magnum damage boost.
- Health regeneration appears at a random location every 25 seconds, remains for 6 seconds, restores 100 HP, and reduces incoming damage by 75% for 4 seconds. Extra HP carries into the next life.
- Time dilation slows enemies and projectiles to half speed for 7 seconds. Double jump allows a second mid-air jump; see `docs/PLAN.md` for unresolved rules.
- Enemy points: crawler 50; blue jumper and flying insect 75; leopard, green shooter, and leaping fish 100; tribal warrior 125.
- Level-clear bonuses: Level 1 = 1,000; Level 2 = 1,500; Level 3 = 2,000. Clear all levels without dying for a 5,000-point perfect-run bonus.
- Level 1 speed bonus: finish under 60 / 45 / 30 seconds for 500 / 1,000 / 1,200 points.

## Requirements and build

Tested on Linux (Ubuntu / WSL) with **g++** and **SFML 2.6.1**. The project uses the SFML 2.x API and is not compatible with SFML 3.x.

Install dependencies:

```bash
sudo apt-get install g++ libsfml-dev
```

Build and run from the repository root (the directory containing `Data/`):

```bash
make
./vice-project-doom
make clean
```

Manual build:

```bash
g++ -c vice_project_doom.cpp
g++ vice_project_doom.o -o vice-project-doom -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system
./vice-project-doom
```

The Makefile enables AddressSanitizer and UndefinedBehaviorSanitizer by default. Assets use relative paths (for example, `Data/bg.png`), and SFML audio assets must be `.ogg` rather than `.mp3`.

## Controls

Update this table to match the final implementation.

| Action | Key |
|---|---|
| Move left / right | `A` / `D` or arrow keys |
| Jump | `Space` |
| Duck / crouch | `S` or Down |
| Climb | `W` / `S` |
| Attack | `J` |
| Switch weapon | `Q` |
| Quit | `Esc` |

## Project layout

```text
Data/                 Sprites, backgrounds, and audio
levels/               ASCII maps: level1.txt, level2.txt, level3.txt
docs/PLAN.md          Plan and decisions log
src/main.cpp          Window, main loop, and game states
src/config.h          Constants for sizes, speeds, health, and damage
src/level.*           Map loading, tile queries, and flip-screen behavior
src/physics.*         Collision helpers
src/player.*          Player movement and abilities
src/weapons.*         Weapons and projectiles
src/enemies.*         Enemy behavior and spawning
src/powerups.*        Power-ups
src/hud_score.*       HUD and scoring
Makefile              Build and clean targets
```

## Mandatory coding constraints

These course rules apply to all project code:

- **No global variables.** Do not declare shared mutable state at file or namespace scope, and do not use `extern` state. Keep state in local variables and pass what functions need through parameters.
- **No `struct`, custom `class`, `std::vector`, or other container types.** Use only permitted C++ built-in types and arrays. SFML types may be used where required to call the library; do not create project-defined classes or structs.
- Store multiple entities in parallel arrays indexed by entity number. Define array capacities and tunable values as named constants in `config.h`; mark inactive entries and reuse free slots rather than deleting array elements.
- Avoid magic numbers. Put speeds, damage, health, timers, spawn counts, and capacities in `config.h`.
- Use small, focused functions; per-entity functions should take an entity index. Bounds-check array and map access. Free memory allocated with `new` using `delete[]`, and check asset-loading results.
- Use consistent indentation, descriptive names, and comments that explain why. Compile with `-Wall -Wextra` without introducing warnings.

## Level maps

Each text map in `levels/` is 14 rows high. Each screen is 18 columns wide, with screens arranged side by side. Use `.` for empty cells.

| Character | Meaning |
|---|---|
| `.` | Empty |
| `#` | Solid block |
| `=` | Horizontal girder |
| `|` | Vertical girder |
| `c` | Crumbling girder |
| `H` | Ladder |
| `v` | Vine |
| `~` | Water |

Update this legend when adding tile types. The map loader must validate dimensions and characters before accessing map data.

## Team workflow

Work primarily in the files assigned to your area. Shared files such as `config.h`, `main.cpp`, and any interfaces should be changed in small, coordinated pull requests. Everyone should understand the full project.

| Area | Owner | Files / responsibilities |
|---|---|---|
| Player and physics | TBD | Movement, jump, duck, ladders, vines, camera, health, lives |
| Combat and enemies | TBD | Weapons, projectiles, enemy types, spawning |
| World and systems | TBD | Levels, maps, power-ups, HUD, scoring, menus, audio, assets |

- Keep the default branch buildable; use a short-lived branch per task (`feat/<area>-<thing>`, `fix/<bug>`, or `chore/<task>`).
- Open pull requests for review; reviewers should build and run the game. Merge small changes frequently and delete merged branches.
- Use specific, imperative, one-line commit messages. Track specification work with GitHub Issues.
- Coordinate binary asset changes with the asset owner. Do not commit build output (`build/`, `*.o`, or the executable) or the 36 MB specification PDF.

Example workflow:

```bash
git clone <repo-url>
git checkout -b feat/player-ladders
git add -p
git commit -m "Add ladder climbing"
git pull --rebase origin main
git push -u origin feat/player-ladders
```

## Security, privacy, and responsible AI

- Never commit passwords, tokens, API keys, personal data, or private files. Report accidental exposure promptly and rotate affected credentials.
- Use two-factor authentication and reviewed pull requests. Keep repository access limited to collaborators.
- Treat map files as untrusted input. Validate dimensions, characters, and array bounds; check asset loads and release allocated memory.
- Use the agreed SFML version and avoid unreviewed third-party code.
- AI tools may support planning, explanation, or review, but contributors must read, test, and understand suggestions, follow course academic-integrity rules, and disclose assistance when required. Never share credentials, private data, or unreleased course materials with AI tools.
- Use and credit assets in accordance with their licenses and the course requirements.

## Milestones and open decisions

- [ ] **M0 — Foundation:** repository setup, starter split, bug fixes, Makefile, and agreed interfaces
- [ ] **M1 — Walking skeleton:** movement, jumping, ducking, flip-screen, and one whip-defeated enemy
- [ ] **M2 — Level 1:** ladders, girders, enemies, weapons, HUD, score, and time bonus
- [ ] **M3 — Level 2:** leopards, shooters, warriors, and power-ups
- [ ] **M4 — Level 3:** waterfall, vines, fish, and mixed enemies
- [ ] **M5 — Polish:** menus, win/game-over states, perfect-run bonus, audio, and bug bash

Record decisions in `docs/PLAN.md`, including enemy/player health and damage, lives, double-jump and time-dilation rules, spawn timing, and details from reference gameplay. Add milestone dates when agreed. Keep behavior data-driven where practical because Phase 2 requirements may change.

## Credits

- Sprites: [The Spriters Resource](https://www.spriters-resource.com/) and assets supplied with the course PDF
- SFML: [SFML](https://www.sfml-dev.org/) 2.6
- Game concept and reference gameplay: CS-1002 Programming Fundamentals, Fall 2026
- Green-screen removal and audio conversion: tools linked in the course helper document
