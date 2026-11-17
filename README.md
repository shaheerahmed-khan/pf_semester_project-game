# VICE: Project Doom

This is our C++ SFML game for the CS-1002 Programming Fundamentals project. It’s a 2D side-scrolling action platformer where you play as Detective Quinn Hart and fight through three levels full of enemies, hazards, and pickups.

Built by: Shaheer · Tehreem · Maryam

## What is the game?

You’re a VICE detective in a dystopian future where the city is overrun by criminals and mutants. The game follows a simple action-platformer flow: move, jump, fight enemies, grab pickups, clear each level, and survive the final push.

### Player abilities
- Move left and right
- Jump
- Duck and crouch
- Climb ladders
- Run while crouching
- Swing across vines in Level 3

### Weapons

| Weapon | Ammo | Damage / Effect |
|---|---:|---|
| Laser Whip | Unlimited | Close-range melee attack |
| .44 Magnum | 10 per level | 10 HP per shot |
| Grenades | 3 per level | 40 HP explosion |

Only one weapon is active at a time, and the player can switch between them during gameplay.

### Enemy types
- Spiked Crawler
- Blue Jumping Enemy
- Flying Insect
- Leopard
- Green Shooter
- Tribal Warrior
- Leaping Fish

Each enemy behaves differently and uses different methods to attack or pressure the player. The exact combat balance and some behavior details are tracked in `docs/PLAN.md`.

### Levels

| Level | Theme | Main features |
|---|---|---|
| 1 | Construction Site | Ladders, girders, random enemy spawns, basic platforming |
| 2 | Ricardo Port | More aggressive enemies, variable platforms, combat pressure |
| 3 | Cliffs, Waterfalls and Jungle | Vines, mixed enemies, tougher movement and combat situations |

The game uses flip-screen scrolling instead of smooth camera movement. As the player crosses the screen boundary, the camera jumps to the next static screen.

## Start the project

This project is built with C++ and SFML 2.x. It has been tested on Linux/WSL and should be developed using the same SFML major version across the team.

### Install dependencies

```bash
sudo apt-get install g++
sudo apt-get install libsfml-dev
```

### Build and run

From the repository root:

```bash
make
./vice-project-doom
```

To clean build files:

```bash
make clean
```

### Manual compile

```bash
g++ -c vice_project_doom.cpp
g++ vice_project_doom.o -o vice-project-doom -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system
./vice-project-doom
```

> Run the game from the repo root so asset paths like `Data/bg.png` resolve correctly.

## Controls

This is the default control layout for now. If we change keys later, update this section with the final version.

| Action | Key |
|---|---|
| Move left / right | `A` / `D` or arrow keys |
| Jump | `Space` |
| Duck / crouch | `S` or Down |
| Climb | `W` / `S` |
| Attack | `J` |
| Switch weapon | `Q` |
| Quit | `Esc` |

## Project structure

```text
pf_semester_project-game/
├── README.md
├── Makefile
├── .gitignore
├── Data/                  Sprites, backgrounds, and audio
├── levels/                ASCII level files
├── docs/
│   └── PLAN.md            Project decisions and planning notes
├── src/
│   ├── main.cpp           Game loop and main window
│   ├── config.h           Constants and tuning values
│   ├── level.*            Level loading and tile logic
│   ├── physics.*           Collision helpers
│   ├── player.*           Player movement and abilities
│   ├── weapons.*          Weapons and projectiles
│   ├── enemies.*          Enemy behavior and spawning
│   ├── powerups.*         Pickups and power-up logic
│   └── hud_score.*        HUD and scoring
└── ...
```

## Important project rules

These rules are mandatory for the course and for the quality of the project.

- No global variables.
- No `struct`, no custom `class`, and no `std::vector` or similar containers.
- Use only basic C++ built-ins and arrays.
- Keep entities in parallel arrays indexed by ID (for example `enemyX[i]`, `enemyHP[i]`).
- Put all important values in `config.h` instead of using magic numbers.
- Mark dead entities as inactive and reuse free slots instead of deleting them.
- Use small functions that handle one behavior at a time.
- Free allocated memory properly and check asset loads.
- Keep code clean, readable, and warning-free with `-Wall -Wextra`.

The overall design is meant to be simple, data-driven, and safe. This makes it easier to work in a team and keeps the project aligned with the course restrictions.

## Level map format

Each level is stored as a text map in `levels/`. A single level is 14 rows tall and each screen is 18 columns wide. Screens are placed side by side to create the full world.

| Character | Meaning |
|---|---|
| `.` | Empty cell |
| `#` | Solid block |
| `=` | Horizontal girder |
| `|` | Vertical girder |
| `c` | Crumbling girder |
| `H` | Ladder |
| `v` | Vine |
| `~` | Water |

Use `.` for empty space so the map stays readable and easy to edit.

## Gameplay systems

### Pickups
- Ammo refill
- Health pickup
- Time dilation effect
- Double jump

### Scoring
- Enemy kills give points
- Level completion gives a big bonus
- Clearing every level without dying gives a perfect-run bonus
- Speed bonus for finishing levels quickly

### Milestones

- [ ] Foundation setup
- [ ] Walking skeleton with movement and basic combat
- [ ] Level 1 complete
- [ ] Level 2 complete
- [ ] Level 3 complete
- [ ] Polish and final bug fixing

Anything still uncertain should be written down in `docs/PLAN.md` so the entire team is aligned.

## Team workflow

We keep work divided by area, but everyone should still understand the full game. Shared files such as `config.h` and `main.cpp` should be changed carefully and in small pull requests.

| Area | Owner | Responsibility |
|---|---|---|
| Player and physics | TBD | Movement, jumping, ladders, vines, camera, health |
| Combat and enemies | TBD | Weapons, projectiles, enemy AI, spawning |
| World and systems | TBD | Level loading, pickups, HUD, score, audio, assets |

### Git habits
- Keep `main` buildable
- Use feature branches such as `feat/player-ladders` or `fix/enemy-ai`
- Pull and rebase before pushing
- Open PRs for review
- Merge small changes often
- Use clear one-line commit messages

Example:

```bash
git clone <repo-url>
git checkout -b feat/player-ladders
git add -p
git commit -m "Add ladder climbing"
git pull --rebase origin main
git push -u origin feat/player-ladders
```

## Security, privacy, and responsible AI

We take data and code safety seriously.

- Do not commit passwords, keys, personal info, or private files.
- Use GitHub security best practices and keep repo access limited.
- Treat level files as untrusted input and validate them properly.
- Check memory safety and avoid bad array access or leaks.
- Use the agreed SFML version only.
- AI can help with planning or explaining ideas, but we still need to understand and test any code before committing it.
- Never share private or sensitive information with AI tools.

## Credits

- Sprites from The Spriters Resource and course-provided assets
- SFML 2.6
- Game concept and reference gameplay from CS-1002, Fall 2026
- Audio conversion and asset preparation done with course-approved tools

## Final note

This project is more than just a game. It is also a chance to learn how to build a project under real constraints, work together as a team, and handle code quality and safety with discipline.
