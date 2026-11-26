# Asset layout

All runtime assets live under `assets/`. The old `data/` folder mixed three
copies of the same tree (`data/`, `data/Data/`, and `data/Assets_Curated/`), so
it was consolidated into one canonical location.

```text
assets/
├── backgrounds/          Full-screen 1136x896 backgrounds
│   ├── level1.png        Used by the current level
│   ├── level2.png
│   ├── level3.png
│   ├── levelx.png
│   ├── menu.png
│   └── starter_city.png
├── sprites/
│   ├── player/
│   │   ├── hart_sprite.png     18x32 sprite used in game
│   │   └── hart_portrait.png   310x464 portrait/UI art
│   ├── enemies/
│   │   ├── spiked_crawler.png
│   │   ├── blue_jumper.png
│   │   ├── flying_insect.png
│   │   ├── leopard.png
│   │   ├── green_shooter.png
│   │   ├── tribal_warrior.png
│   │   ├── leaping_fish.png
│   │   ├── brute.png
│   │   ├── jack.png
│   │   ├── sewer_rat.png
│   │   ├── orb_boss.png
│   │   ├── projectile_leaper.png
│   │   └── frames/       Alternate animation frames
│   ├── bosses/           Boss and minion art
│   ├── fx/               Bullets, explosions, muzzle flashes, etc.
│   ├── items/            Ammo, health, double jump, time dilation, rewind
│   └── ui/               Weapon icons, badge, header art
├── tiles/
│   ├── block1.png        64x64 solid block used by the current level
│   ├── level1.png        Tile atlases (512x128, 8 columns of 64px cells)
│   ├── level2.png
│   ├── level3.png
│   ├── levelx.png
│   └── *.legend.txt      Tile index/behaviour notes
└── audio/
    ├── music/            Level and menu music
    └── sfx/              Jump, hit, pickup, explosion, etc.
```

## What the current build loads

| Purpose | Path |
|---|---|
| Background | `assets/backgrounds/level1.png` |
| Solid block | `assets/tiles/block1.png` |
| Player | `assets/sprites/player/hart_sprite.png` |
| Enemies | `assets/sprites/enemies/*.png` |
| Shooter bullet | `assets/sprites/fx/enemy_bolt.png` |
| Level map | `levels/level1.txt` |

The music and sound-effect files are kept for future levels, but the current
strict SFML subset does not include `Music` playback, so the build is silent.

## Redundancy cleanup

Byte-identical copies were removed rather than kept in parallel:

- `data/bg.png`, `data/Data/bg/starter_city.png`, and `levels/bg.png` were the
  same image; one copy remains at `assets/backgrounds/starter_city.png`.
- `data/player.png` and `hart_placeholder.png` were the same 18x32 sprite; one
  copy remains at `assets/sprites/player/hart_sprite.png`.
- The three `quinn_hart.png` / `hart_portrait.png` copies were identical; one
  remains.
- `detective.png`, `weapons/weapon_sheet.png`, and `ui/weapon_sheet.png` were
  identical; one remains.
- `mus.ogg` and `music_level1.ogg` were identical; one remains.
- `Assets_Curated` duplicates of the phase-one sprites, bosses, UI art, and
  pickups were removed. Unique animation frames and minion variants were kept.
- Raw/keyed sprite sheets, tile-atlas extractions, reference screenshots, and
  marketing art were removed to keep the repository focused on runtime assets.

Tracked `.o` files and the prebuilt Linux binary were also removed; build
outputs are now ignored by `.gitignore`.
