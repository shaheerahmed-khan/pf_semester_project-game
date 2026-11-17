# pf_semester_project-game

**VICE: Project Doom** — a 2D side-scrolling action platformer written in C++ with SFML.

This is a group project for **CS-1002 Programming Fundamentals (Fall 2026)**. It also serves as a showcase of **cybersecurity practices, privacy, and the responsible use of AI** in how we build, store and share our work (see [Security, Privacy & Responsible AI](#security-privacy--responsible-ai)).

**Collaborators:** Shaheer · Tehreem · Maryam

---

## Table of Contents

1. [Story](#story)
2. [Game Overview](#game-overview)
3. [Requirements & Build](#requirements--build)
4. [Controls](#controls)
5. [Project Structure](#project-structure)
6. [Coding Rules](#coding-rules)
7. [Team & Ownership](#team--ownership)
8. [Git Workflow](#git-workflow)
9. [Level Map Format](#level-map-format)
10. [Security, Privacy & Responsible AI](#security-privacy--responsible-ai)
11. [Milestones](#milestones)
12. [Assets & Credits](#assets--credits)

---

## Story

In 2139 the City is overrun by insane criminals and mutants. Authorities blame a new, highly lethal drug called **Gel**. You play **Detective Quinn Hart** of the VICE squad, fighting through hostile territory to uncover the truth behind it, while the BEDA president watches from the top of the company headquarters and his plans near completion.

## Game Overview

The game has **three contiguous levels**, **seven enemy types** in varying numbers, **three weapons**, and a player with several abilities.

### Camera
Levels use **flip-screen (flick-screen) scrolling**, not smooth scrolling. The world is a grid of static single screens. When the player crosses the right edge, the view cuts to the next screen and places the player at the left edge, and the reverse going back. Enemies keep their state across screens. Level 3's initial waterfall needs no vertical scrolling, only platform jumping.

### Player abilities
- Move left and right
- Jump
- Duck
- Run while crouching
- Climb ladders (up and down)
- Swing between platforms on hanging brown vines (Level 3)

### Weapons
Only the currently selected weapon can be used. Switch with a key.

| Weapon | Ammo | Damage | Notes |
|---|---|---|---|
| Laser Whip | Unlimited | — | Close-range signature weapon |
| .44 Magnum | 10 bullets per level | 10 HP | Ranged |
| Grenades | 3 per level | 40 HP | Explosive, for dangerous enemies |

Magnum ammo and grenades are restored by pickups. The player also has a health bar, restored by health pickups.

### Enemies

| Enemy | Behavior | Defeated by |
|---|---|---|
| Spiked Crawler | Ground-based, patrols platforms and belts, contact damage only | One laser or grenade hit |
| Blue Jumping Enemy | Bounces toward the player, contact damage, dangerous near gaps | One whip strike, or grenades in groups |
| Flying Insect | Hovers and flies across gaps on elevated paths | One whip strike (standing or mid-air) or Magnum |
| Leopard | Fast, low predator that sprints and pounces | Whip or a well-timed Magnum shot |
| Green Shooter | Stationary on platforms, fires low projectiles across the screen | Standing whip strike or ranged fire |
| Tribal Warrior | Masked humanoid with shield and bat (jungle) | See team decisions log |
| Leaping Fish | Jumps in and out of water and shoots fireballs | See team decisions log |

### Levels

| Level | Setting | Enemies |
|---|---|---|
| 1 | The Construction Site | 4 spiked crawlers, 5 blue jumping, 6 flying insects (spawn sequentially, in random timing, each run). Vertical, horizontal and crumbling girders, plus climbable ladders |
| 2 | The Ricardo Port | 10 leopards, 4 green shooters, 8 tribal warriors. Variable-height platforms, containers and meshes |
| 3 | The Cliffs, Waterfalls and Jungle | 10 flying fish, 8 tribal warriors, 10 leopards, 7 flying insects, 5 blue jumping. Interactive swinging vines, destructible and regrowing vines |

### Power-ups

| Power-up | Effect |
|---|---|
| Ammo drop / refill | Killing a Spiked Crawler may drop extra ammo or a temporary Magnum damage boost |
| Health regeneration | Appears at a random location every 25 s, disappears after 6 s. Restores 100 HP, reduces incoming damage by 75% for 4 s, and extra HP carries into the next life |
| Time dilation | Enemies, projectiles and bullets move at 50% speed for 7 s |
| Double jump | A second jump in mid-air (see decisions log for the exact energy rule) |

### Scoring

| Category | Action | Points |
|---|---|---|
| Enemy kills | Spiked Crawler | 50 |
| | Blue Jumping Enemy | 75 |
| | Flying Insect | 75 |
| | Leopard | 100 |
| | Green Shooter | 100 |
| | Tribal Warrior | 125 |
| | Leaping Fish | 100 |
| Level clear | Level 1 / 2 / 3 | 1000 / 1500 / 2000 |
| Perfect run | Clear all levels without dying | 5000 |
| Level 1 speed bonus | Under 60 s / 45 s / 30 s | 500 / 1000 / 1200 |

---

## Requirements & Build

Tested with **SFML 2.6.1** on Linux (Ubuntu / WSL). The code is written for the SFML **2.x** API and **will not compile against SFML 3.x**. All team members must use the same major version.

**One-time install**

```bash
sudo apt-get install g++
sudo apt-get install libsfml-dev
```

**Build and run (Makefile)**

```bash
make            # builds ./vice-project-doom
./vice-project-doom
make clean      # remove build output
```

**Manual build (matches the course instructions)**

```bash
g++ -c vice_project_doom.cpp
g++ vice_project_doom.o -o vice-project-doom -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system
./vice-project-doom
```

> **Run the game from the repository root** (the folder that contains `Data/`), not from inside `Data/`. Assets are loaded with relative paths such as `Data/bg.png`.

**Debug builds** add `-fsanitize=address,undefined` (the Makefile does this by default) to catch out-of-bounds array access early.

---

## Controls

> Keys are our choice. Update this table to match the final implementation.

| Action | Key |
|---|---|
| Move left / right | `A` / `D` or arrow keys |
| Jump | `Space` |
| Duck / crouch | `S` or Down |
| Climb ladder | `W` / `S` |
| Attack | `J` |
| Switch weapon | `Q` |
| Quit | `Esc` |

---

## Project Structure

```
pf_semester_project-game/
├── README.md
├── Makefile
├── .gitignore
├── Data/                 sprites, backgrounds, audio (.ogg only; SFML has no mp3)
├── levels/               level1.txt, level2.txt, level3.txt (ASCII maps)
├── docs/
│   └── PLAN.md           plan and decisions log
└── src/
    ├── main.cpp          window, main loop, game states
    ├── config.h          ALL constants (sizes, speeds, HP, damage)
    ├── state.h / .cpp    shared global arrays and variables
    ├── level.h / .cpp    map loading, tile queries, flip-screen
    ├── physics.h / .cpp  collision helpers
    ├── player.h / .cpp   movement, jump, duck, ladders, vines
    ├── weapons.h / .cpp  whip, magnum, grenades, projectiles
    ├── enemies.h / .cpp  all enemy types and spawning
    ├── powerups.h / .cpp
    └── hud_score.h / .cpp
```

---

## Coding Rules

The course restricts what we may use. **These rules are mandatory.**

- **No `struct`, no `class`, no `vector`.** Use plain variables, arrays, pointers and functions.
- Store entities as **parallel arrays** indexed by entity number (for example `enemyX[i]`, `enemyHP[i]`). Array sizes come from constants in `config.h` (`MAX_ENEMIES`, `MAX_BULLETS`, ...).
- Dead entities are never deleted. Mark `active = false` and reuse the first free slot.
- Shared state is declared `extern` in `state.h` and defined once in `state.cpp`. Each owner adds their variables under their own comment banner. Never reorder existing lines.
- **No magic numbers.** Every tunable value (speed, damage, HP, timers, spawn counts) lives in `config.h`.
- Per-entity behavior goes in functions that take an index (`updateLeopard(int i)`).
- Free everything allocated with `new` (use `delete[]`). Check the return value of every asset load.
- Style: 4-space indent or tabs (stay consistent with the file), descriptive names, one function does one job, comment the *why*.
- The code must compile with `-Wall -Wextra` without new warnings.

---

## Team & Ownership

Each person mostly edits their own files, which keeps merge conflicts rare.

| Area | Owner | Files |
|---|---|---|
| Player and physics: movement, jump, duck, ladders, vines, flip-screen camera, health/lives | _TBD_ | `player`, `physics` |
| Combat and enemies: weapons, projectiles, all enemy types, spawn rules | _TBD_ | `weapons`, `enemies` |
| World and systems: level loader and maps, power-ups, HUD, scoring, menus, audio, assets | _TBD_ | `level`, `powerups`, `hud_score`, `Data/`, `levels/` |

Shared files (`config.h`, `state.h`, `state.cpp`, `main.cpp`) are conflict hotspots: change them in **tiny PRs**, merge fast, and tell the others.

Every team member must be able to explain **all** of the code, not just their own part.

---

## Git Workflow

- `main` **always compiles and runs.** Never push to it directly. It is protected: pull request plus one approval.
- One short-lived branch per task: `feat/<area>-<thing>`, `fix/<bug>`, `chore/<task>` (for example `feat/enemy-leopard`).
- Before pushing: `git pull --rebase origin main`.
- Open a PR when the feature works. The reviewer **builds and runs** it before approving.
- Merge small and often (at least daily). Delete merged branches.
- Commit messages: imperative, specific, one line ("Add leopard pounce state"). Everyone commits regularly so the history shows individual contributions.
- Track work with **one GitHub Issue per spec requirement**, grouped by milestone.
- Binary assets in `Data/` are committed by the asset owner only. Keep files small.
- Do **not** commit build output (`build/`, `*.o`, the executable) or the 36 MB spec PDF.

Quick reference:

```bash
git clone <repo-url>
git checkout -b feat/player-ladders
git add -p                       # review what you stage
git commit -m "Add ladder climbing"
git pull --rebase origin main
git push -u origin feat/player-ladders   # then open a PR
```

---

## Level Map Format

Each level is a text file in `levels/`. A level is **14 rows tall**, and each screen is **18 columns wide**; screens are laid side by side.

| Char | Meaning |
|---|---|
| `.` | empty |
| `#` | solid block |
| `=` | horizontal girder |
| `\|` | vertical girder |
| `c` | crumbling girder |
| `H` | ladder |
| `v` | vine |
| `~` | water |

> Update this legend when new tiles are added. Use `.` rather than spaces for empty cells so maps stay readable and diff-friendly.

---

## Security, Privacy & Responsible AI

- **No secrets or personal data in the repo.** No passwords, tokens, API keys, personal files or contact details are ever committed. If something sensitive is committed by accident, tell the team immediately and rotate or remove it (deleting a file in a later commit does not remove it from history).
- **Accounts:** GitHub accounts use two-factor authentication. The repo is **private** and only the collaborators listed above have access.
- **Access control:** `main` is branch-protected, and changes come in through reviewed pull requests.
- **Memory safety is a security practice.** Out-of-bounds array access and leaks are real vulnerability classes. We bounds-check every array and grid access, free what we allocate, and develop with address/UB sanitizers enabled.
- **Input handling:** map files are treated as untrusted input. The loader validates dimensions and characters and fails safely with a clear message, never by overrunning a buffer.
- **Dependencies:** SFML is installed from the distribution package, at the single agreed version. No unknown third-party code is copied in.
- **Responsible use of AI:**
  - AI tools may help us plan, explain concepts and review code, but we **do not submit code we cannot explain**.
  - We **read, test and understand** anything an AI suggests before committing it.
  - We **follow the course's academic-integrity policy** on AI use and disclose AI assistance where required.
  - We never paste private data, credentials or unreleased course material into AI tools.
- **Assets and licensing:** sprites and audio are used for this educational project only, credited below.

---

## Milestones

Put real dates next to these.

- [ ] **M0 Foundation:** repo, starter split into files, bug fixes, Makefile, shared headers agreed
- [ ] **M1 Walking skeleton:** player moves, jumps and ducks; flip-screen works; one enemy killed by the whip
- [ ] **M2 Level 1 complete:** ladders, all girder types, 3 enemy types, 3 weapons, HUD, scoring, time bonus
- [ ] **M3 Level 2 complete:** leopards, shooters, warriors, power-ups
- [ ] **M4 Level 3 complete:** waterfall, vines, fish, mixed enemy counts
- [ ] **M5 Polish and freeze:** menu, game over and win states, perfect-run bonus, audio, bug bash

Phase 2 is announced as "coming soon", so keep behavior data-driven (`config.h` and level files) and avoid hardcoding.

### Open decisions (record answers in `docs/PLAN.md`)

- Enemy HP for leopards, shooters, warriors and fish; whip damage
- Player max HP and number of lives
- How time dilation and double jump are obtained, and the exact double-jump energy rule
- Exact meaning of "spawn sequentially but randomly (in their order)"
- Details taken from the reference gameplay videos (crumbling girder timing, whip range, ladder behavior)

---

## Assets & Credits

- Sprites: [The Spriters Resource](https://www.spriters-resource.com/) and the files supplied with the course PDF
- Green-screen removal and audio conversion (`.mp3` to `.ogg`) done with the tools linked in the course helper document
- Built with [SFML](https://www.sfml-dev.org/) 2.6
- Game concept and reference gameplay: provided by the course (CS-1002, Fall 2026)
