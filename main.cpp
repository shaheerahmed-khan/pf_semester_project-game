#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

using namespace sf;
using namespace std;

// ===========================================================================
// Settings
//
// Plain arrays hold all game settings and entity state so everything stays
// simple and easy to follow without structs or globals.
//
// tuning[64] layout:
//  0   screen width        32   max pickups
//  1   screen height       33   enemy bullet speed
//  2   cell size           34   enemy bullet damage
//  3   ticks per second    35   spawn interval
//  4   gravity             36   max spawn batch
//  5   terminal fall speed 37   belt speed (canvas units per tick)
//  6   collision epsilon   38   current speed (random wander)
//  7   player max health   39   health item spawn interval
//  8   player width        40   health item lifetime
//  9   player height       41   shield ticks
// 10   crouch height       42   dilation ticks
// 11   walk speed          43   boost ticks
// 12   crouch-run speed    44-46  boss max hp (phases 1/2/3)
// 13   jump speed          47-52  boss w/h (phases 1/2/3)
// 14   ladder speed        53   boss intro ticks
// 15   invuln ticks        54   boss gate window
// 16   whip range          55   campaign no-death bonus (8000)
// 17   whip damage         56   campaign no-damage bonus (10000)
// 18   whip cooldown       57   reserved
// 19   magnum damage       58   reserved
// 20   magnum cooldown     59   reserved
// 21   magnum ammo/level
// 22   magnum max ammo
// 23   grenade damage
// 24   grenades/level
// 25   grenade cooldown
// 26   grenade radius
// 27   magnum bullet speed
// 28   grenade throw speed
// ===========================================================================

const int EnemyActive = 0;
const int EnemyType = 1;
const int EnemyHp = 2;
const int EnemyMaxHp = 3;
const int EnemyDir = 4;
const int EnemyState = 5;
const int EnemyTimer = 6;
const int EnemyGrounded = 7;
const int EnemyScored = 8;

const int ShotActive = 0;
const int ShotKind = 1;
const int ShotDamage = 2;
const int ShotLife = 3;

// Sound indices in sfx_buf[15]
const int SfxJump = 0;
const int SfxWhip = 1;
const int SfxShot = 2;
const int SfxGrenade = 3;
const int SfxExplosion = 4;
const int SfxHit = 5;
const int SfxHurt = 6;
const int SfxDeath = 7;
const int SfxPickup = 8;
const int SfxClear = 9;
const int SfxBossHit = 10;
const int SfxBossPhase = 11;
const int SfxMenu = 12;
const int SfxRewind = 13;
const int SfxZap = 14;

struct AudioBank {
  bool enabled = false;
  SoundBuffer buffers[15];
  Sound pool[24];
  int cursor = 0;

  void play(int id, float pitch = 1.0f, float vol = 75.0f) {
    if (!enabled || id < 0 || id >= 15) {
      return;
    }
    pool[cursor].setBuffer(buffers[id]);
    pool[cursor].setPitch(pitch);
    pool[cursor].setVolume(vol);
    pool[cursor].play();
    cursor = (cursor + 1) % 24;
  }
};

int death_penalty(int score);

int run_game(float *tuning, float enemy_cfg[13][14], int spawn_types[5][5],
             int spawn_counts[5][5], int endless_types[5], int level_points[5],
             int l1_limit[3], int l1_bonus[3], int lx_limit[3], int lx_bonus[3],
             int boss_limit[3], int boss_bonus[3], int boss_quota_type[5],
             int boss_quota_count[5]);

int run_self_test(float *tuning, float enemy_cfg[13][14], int spawn_types[5][5],
                  int spawn_counts[5][5], int endless_types[5],
                  int level_points[5], int l1_limit[3], int l1_bonus[3],
                  int lx_limit[3], int lx_bonus[3], int boss_limit[3],
                  int boss_bonus[3], int boss_quota_type[5],
                  int boss_quota_count[5]);

int main(int argc, char **argv) {
  float tuning[64] = {};

  tuning[0] = 1136;  // screen width
  tuning[1] = 896;   // screen height
  tuning[2] = 64;    // cell size
  tuning[3] = 60;    // ticks per second
  tuning[4] = 1.0f;  // gravity
  tuning[5] = 20.0f; // terminal fall speed
  tuning[6] = 0.01f; // collision epsilon

  tuning[7] = 200;    // player max health
  tuning[8] = 54;     // player width
  tuning[9] = 96;     // player height
  tuning[10] = 56;    // crouch height
  tuning[11] = 5.5f;  // walk speed
  tuning[12] = 6.5f;  // crouch-run speed
  tuning[13] = 20.0f; // jump speed
  tuning[14] = 5.0f;  // ladder speed
  tuning[15] = 60;    // invulnerability ticks

  tuning[16] = 120;   // whip range
  tuning[17] = 30;    // whip damage
  tuning[18] = 18;    // whip cooldown
  tuning[19] = 10;    // magnum damage
  tuning[20] = 24;    // magnum cooldown
  tuning[21] = 10;    // magnum ammo per level
  tuning[22] = 30;    // magnum max ammo
  tuning[23] = 40;    // grenade damage
  tuning[24] = 3;     // grenades per level
  tuning[25] = 60;    // grenade cooldown
  tuning[26] = 130;   // grenade radius
  tuning[27] = 18.0f; // magnum bullet speed
  tuning[28] = 8.0f;  // grenade throw speed

  tuning[29] = 13;   // enemy type count
  tuning[30] = 64;   // max enemies
  tuning[31] = 128;  // max shots
  tuning[32] = 16;   // max pickups
  tuning[33] = 6.0f; // enemy bullet speed
  tuning[34] = 10;   // enemy bullet damage
  tuning[35] = 90;   // spawn interval
  tuning[36] = 8;    // max spawn batch
  tuning[37] = 1.6f; // belt speed (canvas units per tick)
  tuning[38] = 1.2f; // current speed (small random wander)

  tuning[39] = 25 * 60; // health item interval
  tuning[40] = 6 * 60;  // health item lifetime
  tuning[41] = 4 * 60;  // shield ticks
  tuning[42] = 7 * 60;  // dilation ticks
  tuning[43] = 6 * 60;  // magnum boost ticks

  tuning[44] = 300;      // tank health
  tuning[45] = 260;      // humanoid health
  tuning[46] = 360;      // skeleton health
  tuning[47] = 240;      // tank width
  tuning[48] = 140;      // tank height
  tuning[49] = 160;      // humanoid width
  tuning[50] = 220;      // humanoid height
  tuning[51] = 220;      // skeleton width
  tuning[52] = 260;      // skeleton height
  tuning[53] = 180;      // boss intro
  tuning[54] = 120 * 60; // boss gate window
  tuning[55] = 8000;     // campaign without dying bonus
  tuning[56] = 10000;    // campaign without damage bonus
  tuning[57] = 20;       // reserved
  tuning[58] = 40;       // reserved
  tuning[59] = 30;       // reserved

  // Enemy table columns:
  // 0 width, 1 height, 2 hp, 3 contact damage, 4 walk speed, 5 score,
  // 6..13 are type-specific numbers used by update_enemy().
  float enemy_cfg[13][14] = {
      {56, 40, 20, 10, 2.0f, 50, 0, 0, 0, 0, 0, 0, 0, 0},
      {48, 64, 30, 10, 0, 75, 4, 16, 60, 500, 0, 0, 0, 0},
      {40, 32, 10, 5, 0, 75, 2.5f, 1.5f, 0.1f, 3, 0, 0, 0, 0},
      {80, 48, 40, 20, 1.5f, 100, 7, 420, 100, 45, 60, 0, 0, 0},
      {48, 80, 20, 10, 0, 100, 520, 120, 90, 0, 0, 0, 0, 0},
      {56, 96, 60, 20, 1.2f, 125, 1.5f, 6, 360, 110, 90, 30, 15, 60},
      {32, 40, 20, 15, 0, 100, 18, 90, 0, 0, 0, 0, 0, 0},
      {96, 120, 90, 25, 0.8f, 100, 500, 150, 0, 0, 0, 0, 0, 0},
      {64, 90, 50, 15, 2.5f, 125, 10, 6, 90, 450, 0, 0, 0, 0},
      {40, 28, 10, 8, 3.5f, 80, 0, 0, 0, 0, 0, 0, 0, 0},
      {56, 56, 40, 20, 1.5f, 175, 1.5f, 460, 420, 0, 0, 0, 0, 0},
      {72, 48, 30, 20, 0, 150, 520, 14, 8, 40, 120, 0, 0, 0},
      {40, 48, 15, 10, 2.0f, 50, 10, 4, 60, 420, 0, 0, 0, 0}};

  int spawn_types[5][5] = {{0, 0, 0, 0, 0},
                           {0, 1, 2, 0, 0},
                           {7, 8, 9, 10, 4},
                           {6, 5, 3, 2, 1},
                           {0, 0, 0, 0, 0}};
  int spawn_counts[5][5] = {{0, 0, 0, 0, 0},
                            {4, 5, 6, 0, 0},
                            {4, 4, 6, 2, 4},
                            {10, 8, 10, 7, 5},
                            {0, 0, 0, 0, 0}};

  int endless_types[5] = {7, 8, 9, 10, 11};
  int level_points[5] = {0, 1000, 1500, 2000, 2500};
  int l1_limit[3] = {60, 45, 30};
  int l1_bonus[3] = {500, 1000, 1200};
  int lx_limit[3] = {120, 90, 75};
  int lx_bonus[3] = {1500, 2000, 2500};
  int boss_limit[3] = {150, 120, 90};
  int boss_bonus[3] = {2500, 2700, 3000};
  int boss_quota_type[5] = {7, 8, 9, 10, 11};
  int boss_quota_count[5] = {2, 3, 3, 4, 2};

  srand(static_cast<unsigned int>(time(nullptr)));

  if (argc > 1 && strcmp(argv[1], "--selftest") == 0) {
    return run_self_test(tuning, enemy_cfg, spawn_types, spawn_counts,
                         endless_types, level_points, l1_limit, l1_bonus,
                         lx_limit, lx_bonus, boss_limit, boss_bonus,
                         boss_quota_type, boss_quota_count);
  }

  return run_game(tuning, enemy_cfg, spawn_types, spawn_counts, endless_types,
                  level_points, l1_limit, l1_bonus, lx_limit, lx_bonus,
                  boss_limit, boss_bonus, boss_quota_type, boss_quota_count);
}

// ===========================================================================
// Small math & random helpers
// ===========================================================================

float clamp_float(float value, float low, float high) {
  if (value < low) {
    return low;
  }
  if (value > high) {
    return high;
  }
  return value;
}

int clamp_int(int value, int low, int high) {
  if (value < low) {
    return low;
  }
  if (value > high) {
    return high;
  }
  return value;
}

bool boxes_overlap(float ax, float ay, float aw, float ah, float bx, float by,
                   float bw, float bh) {
  return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

int random_int(int low, int high) {
  if (high <= low) {
    return low;
  }
  return low + rand() % (high - low + 1);
}

// ===========================================================================
// Bitmap text drawing
// ===========================================================================

void draw_text(RenderWindow &window, const char *text, float x, float y,
               int scale, Color color) {
  const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 :.,-!?/'()<>=+%";
  const unsigned char glyphs[][7] = {
      {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
      {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E},
      {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E},
      {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E},
      {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F},
      {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10},
      {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F},
      {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
      {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E},
      {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C},
      {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11},
      {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F},
      {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11},
      {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11},
      {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
      {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10},
      {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D},
      {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11},
      {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E},
      {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
      {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
      {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04},
      {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11},
      {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11},
      {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04},
      {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F},
      {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E},
      {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E},
      {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F},
      {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E},
      {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02},
      {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E},
      {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E},
      {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},
      {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E},
      {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C},
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00},
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C},
      {0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08},
      {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00},
      {0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04},
      {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04},
      {0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10},
      {0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02},
      {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08},
      {0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02},
      {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08},
      {0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00},
      {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00},
      {0x19, 0x1A, 0x02, 0x04, 0x08, 0x0B, 0x13}};

  float pixel_size = static_cast<float>(scale);
  RectangleShape pixel(Vector2f(pixel_size, pixel_size));
  pixel.setFillColor(color);

  float start_x = x;
  for (int i = 0; text[i] != '\0'; i++) {
    char c = text[i];
    if (c == '\n') {
      x = start_x;
      y += 9.0f * pixel_size;
      continue;
    }
    if (c >= 'a' && c <= 'z') {
      c = static_cast<char>(c - 32);
    }

    const char *found = strchr(chars, c);
    int index = found == nullptr ? 0 : static_cast<int>(found - chars);

    if (c != ' ') {
      for (int row = 0; row < 7; row++) {
        for (int bit = 0; bit < 5; bit++) {
          if (glyphs[index][row] & (1 << (4 - bit))) {
            pixel.setPosition(x + bit * pixel_size, y + row * pixel_size);
            window.draw(pixel);
          }
        }
      }
    }
    x += 6.0f * pixel_size;
  }
}

float text_width(const char *text, int scale) {
  return static_cast<float>(strlen(text)) * 6.0f * static_cast<float>(scale);
}

void draw_text_center(RenderWindow &window, const char *text, float y,
                      int scale, Color color, int screen_w) {
  float centered_x =
      (static_cast<float>(screen_w) - text_width(text, scale)) / 2.0f;
  draw_text(window, text, centered_x, y, scale, color);
}

// ===========================================================================
// Level grid helpers
// ===========================================================================

char tile_at(char **grid, int level_w, int level_h, int row, int col) {
  if (grid == nullptr || row < 0 || row >= level_h || col < 0 ||
      col >= level_w) {
    return '.';
  }
  return grid[row][col];
}

bool tile_solid(char **grid, int level_w, int level_h, int row, int col,
                int electric_timer) {
  if (col < 0 || col >= level_w) {
    return true;
  }
  if (row < 0 || row >= level_h) {
    return false;
  }

  char c = grid[row][col];
  if (c == '#' || c == 'C' || c == '>' || c == '<') {
    return true;
  }
  if (c == 'Z') {
    return (electric_timer / 60) % 5 < 3;
  }
  return false;
}

bool box_solid(char **grid, int level_w, int level_h, float x, float y, float w,
               float h, int cell, float eps, int electric_timer) {
  float cell_f = static_cast<float>(cell);
  int left = static_cast<int>(floor(x / cell_f));
  int right = static_cast<int>(floor((x + w - eps) / cell_f));
  int top = static_cast<int>(floor(y / cell_f));
  int bottom = static_cast<int>(floor((y + h - eps) / cell_f));

  for (int row = top; row <= bottom; row++) {
    for (int col = left; col <= right; col++) {
      if (tile_solid(grid, level_w, level_h, row, col, electric_timer)) {
        return true;
      }
    }
  }
  return false;
}

float find_ground_y(char **grid, int level_w, int level_h, float x,
                    float object_h, int cell) {
  int col =
      clamp_int(static_cast<int>(x / static_cast<float>(cell)), 0, level_w - 1);
  for (int row = 0; row < level_h; row++) {
    char c = tile_at(grid, level_w, level_h, row, col);
    if (c == '#' || c == '=' || c == '>' || c == '<' || c == 'C') {
      return static_cast<float>(row * cell) - object_h;
    }
  }
  return static_cast<float>((level_h - 2) * cell) - object_h;
}

char **load_level(const char *path, int &level_w, int &level_h, float &start_x,
                  float &start_y, int cell, int player_h, int &exit_col,
                  int &exit_row, int *&tile_timer, int *&tile_state) {
  FILE *file = fopen(path, "r");
  if (file == nullptr) {
    return nullptr;
  }

  char line[512];
  int rows = 0;
  int columns = 0;

  while (fgets(line, static_cast<int>(sizeof(line)), file) != nullptr) {
    int length = static_cast<int>(strlen(line));
    while (length > 0 &&
           (line[length - 1] == '\n' || line[length - 1] == '\r')) {
      length--;
    }
    if (length > columns) {
      columns = length;
    }
    rows++;
  }

  if (rows <= 0 || columns <= 0) {
    fclose(file);
    return nullptr;
  }

  char **grid = new char *[rows];
  for (int i = 0; i < rows; i++) {
    grid[i] = new char[columns];
    for (int j = 0; j < columns; j++) {
      grid[i][j] = '.';
    }
  }

  rewind(file);
  start_x = static_cast<float>(cell);
  start_y = static_cast<float>((rows - 1) * cell - player_h);
  exit_col = -1;
  exit_row = -1;

  for (int i = 0; i < rows; i++) {
    if (fgets(line, static_cast<int>(sizeof(line)), file) == nullptr) {
      line[0] = '\0';
    }

    int length = static_cast<int>(strlen(line));
    while (length > 0 &&
           (line[length - 1] == '\n' || line[length - 1] == '\r')) {
      length--;
    }

    for (int j = 0; j < columns; j++) {
      if (j >= length) {
        continue;
      }
      grid[i][j] = line[j];
      if (line[j] == 'P') {
        start_x = static_cast<float>(j * cell);
        start_y = static_cast<float>((i + 1) * cell - player_h);
        grid[i][j] = '.';
      }
      if (line[j] == 'X') {
        exit_col = j;
        exit_row = i;
      }
    }
  }

  fclose(file);

  level_w = columns;
  level_h = rows;
  tile_timer = new int[rows * columns]();
  tile_state = new int[rows * columns]();
  return grid;
}

void free_level(char **grid, int level_h, int *tile_timer, int *tile_state) {
  if (grid != nullptr) {
    for (int i = 0; i < level_h; i++) {
      delete[] grid[i];
    }
    delete[] grid;
  }
  delete[] tile_timer;
  delete[] tile_state;
}

// ===========================================================================
// Player movement & actions
//
// player[8]: 0 x, 1 y, 2 vx, 3 vy, 4 vine anchor x, 5 vine anchor y,
//            6 vine angle, 7 vine speed
// p[32]:     0 width, 1 height, 2 facing, 3 ground char, 4 on ground,
//            5 crouching, 6 on ladder, 7 on vine, 8 health, 9 max health,
//            10 bonus hp carry-over, 11 invuln ticks, 12 shield ticks,
//            13 time-dilation ticks, 14 damage-boost ticks,
//            15 double-jump uses this tick, 16 double-jump available,
//            17 weapon, 18 ammo, 19 grenades, 20 whip cooldown,
//            21 magnum cooldown, 22 grenade cooldown, 23 whip flash,
//            24 waterfall ticks, 25 vine column
// ===========================================================================

void move_player_x(float *player, int *p, char **grid, int level_w, int level_h,
                   int cell, float eps, int electric_timer, float amount) {
  float &x = player[0];
  float &y = player[1];
  int &w = p[0];
  int &h = p[1];
  float cell_f = static_cast<float>(cell);
  float width_f = static_cast<float>(w);

  if (amount == 0) {
    return;
  }

  x += amount;
  if (box_solid(grid, level_w, level_h, x, y, width_f, static_cast<float>(h),
                cell, eps, electric_timer)) {
    if (amount > 0) {
      int col = static_cast<int>((x + width_f) / cell_f);
      x = static_cast<float>(col * cell) - width_f - eps;
    } else {
      int col = static_cast<int>(x / cell_f);
      x = static_cast<float>((col + 1) * cell) + eps;
    }
  }

  x = clamp_float(x, 0, static_cast<float>(level_w * cell - w));
}

void move_player_y(float *player, int *p, char **grid, int level_w, int level_h,
                   int cell, float eps, int electric_timer, float amount) {
  float &x = player[0];
  float &y = player[1];
  float &vy = player[3];
  int &h = p[1];
  int &on_ground = p[4];
  int &ground_char = p[3];
  float cell_f = static_cast<float>(cell);
  float height_f = static_cast<float>(h);

  float old_bottom = y + height_f;
  y += amount;
  bool landed = false;

  if (box_solid(grid, level_w, level_h, x, y, static_cast<float>(p[0]),
                height_f, cell, eps, electric_timer)) {
    if (amount > 0) {
      int row = static_cast<int>((y + height_f) / cell_f);
      y = static_cast<float>(row * cell) - height_f - eps;
      landed = true;
    } else if (amount < 0) {
      int row = static_cast<int>(y / cell_f);
      y = static_cast<float>((row + 1) * cell) + eps;
    }
    vy = 0;
  } else if (amount > 0) {
    int row = static_cast<int>((y + height_f) / cell_f);
    int left = static_cast<int>((x + 2.0f) / cell_f);
    int right =
        static_cast<int>((x + static_cast<float>(p[0]) - 2.0f) / cell_f);

    for (int col = left; col <= right; col++) {
      if (tile_at(grid, level_w, level_h, row, col) == '=' &&
          old_bottom <= static_cast<float>(row * cell) + eps) {
        y = static_cast<float>(row * cell) - height_f - eps;
        vy = 0;
        landed = true;
        break;
      }
    }
  }

  on_ground = landed ? 1 : 0;
  int ground_row = static_cast<int>((y + height_f + 2.0f) / cell_f);
  int ground_col = clamp_int(
      static_cast<int>((x + static_cast<float>(p[0]) / 2.0f) / cell_f), 0,
      level_w - 1);
  ground_char = tile_at(grid, level_w, level_h, ground_row, ground_col);
}

void detach_vine(float *player, int *p, float *tuning, bool left, bool right,
                 bool jump_away, bool jump_pressed, AudioBank *audio) {
  float &vx = player[2];
  float &vy = player[3];
  float &vangle = player[6];
  float &vspeed = player[7];
  int &on_vine = p[7];

  on_vine = 0;
  if (!jump_away) {
    vx = 0;
    vy = 0;
    return;
  }

  vx = cos(vangle) * 210.0f * vspeed;
  vy = -tuning[13] * 0.7f;
  if (jump_pressed) {
    vy = -tuning[13];
  }
  vx += (right ? 3.0f : 0.0f) - (left ? 3.0f : 0.0f);
  if (audio != nullptr) {
    audio->play(SfxJump, 1.05f, 70.0f);
  }
}

void update_vine(float *player, int *p, float *tuning, char **grid, int level_w,
                 int level_h, int cell, bool left, bool right, bool down,
                 bool jump_pressed, AudioBank *audio) {
  float &x = player[0];
  float &y = player[1];
  float &vx = player[2];
  float &vy = player[3];
  float &vanchor_x = player[4];
  float &vanchor_y = player[5];
  float &vangle = player[6];
  float &vspeed = player[7];
  int &facing = p[2];
  int &on_ground = p[4];
  int &on_vine = p[7];
  int &vcol = p[25];
  int &w = p[0];
  float cell_f = static_cast<float>(cell);

  int anchor_row = static_cast<int>(vanchor_y / cell_f);
  if (vcol < 0 || vcol >= level_w ||
      tile_at(grid, level_w, level_h, anchor_row, vcol) != 'V') {
    on_vine = 0;
    return;
  }

  vspeed +=
      (right ? 0.003f : 0.0f) - (left ? 0.003f : 0.0f) - sin(vangle) * 0.0018f;
  vspeed *= 0.995f;
  vspeed = clamp_float(vspeed, -0.08f, 0.08f);
  vangle = clamp_float(vangle + vspeed, -1.2f, 1.2f);

  x = vanchor_x + sin(vangle) * 210.0f - static_cast<float>(w) / 2.0f;
  y = vanchor_y + cos(vangle) * 210.0f;
  on_ground = 0;
  vx = 0;
  vy = 0;
  if (right) {
    facing = 1;
  }
  if (left) {
    facing = -1;
  }

  if (jump_pressed) {
    detach_vine(player, p, tuning, left, right, true, true, audio);
  } else if (down) {
    detach_vine(player, p, tuning, left, right, false, false, audio);
  }
}

void grab_vine(float *player, int *p, char **grid, int level_w, int level_h,
               int cell, bool up) {
  float &x = player[0];
  float &y = player[1];
  float &vx = player[2];
  float &vy = player[3];
  float &vanchor_x = player[4];
  float &vanchor_y = player[5];
  float &vangle = player[6];
  float &vspeed = player[7];
  int &on_vine = p[7];
  int &vcol = p[25];
  int &w = p[0];
  float cell_f = static_cast<float>(cell);

  if (!up || on_vine) {
    return;
  }

  int col =
      clamp_int(static_cast<int>((x + static_cast<float>(w) / 2.0f) / cell_f),
                0, level_w - 1);
  int row = clamp_int(
      static_cast<int>((y + static_cast<float>(p[1]) / 2.0f) / cell_f), 0,
      level_h - 1);

  bool found = false;
  int top_row = row;
  for (int r = row; r >= 0; r--) {
    if (tile_at(grid, level_w, level_h, r, col) == 'V') {
      found = true;
      top_row = r;
    } else if (found) {
      break;
    }
  }
  if (!found) {
    return;
  }

  on_vine = 1;
  vcol = col;
  vanchor_x = static_cast<float>(col * cell + cell / 2);
  vanchor_y = static_cast<float>(top_row * cell);
  vangle = 0;
  vspeed = 0;
  x = vanchor_x - static_cast<float>(w) / 2.0f;
  y = vanchor_y + 210.0f;
  vx = 0;
  vy = 0;
}

void update_player(float *player, int *p, char **grid, int level_w, int level_h,
                   int electric_timer, float *tuning, bool left, bool right,
                   bool up, bool down, bool jump_pressed, AudioBank *audio) {
  float &x = player[0];
  float &y = player[1];
  float &vx = player[2];
  float &vy = player[3];
  int &w = p[0];
  int &h = p[1];
  int &facing = p[2];
  int &on_ground = p[4];
  int &crouching = p[5];
  int &on_ladder = p[6];
  int &on_vine = p[7];
  int &jumps_used = p[15];
  int &waterfall_ticks = p[24];

  int cell = static_cast<int>(tuning[2]);
  float cell_f = static_cast<float>(cell);
  float gravity = tuning[4];
  float terminal = tuning[5];
  float eps = tuning[6];
  float speed = tuning[11];
  float crouch_speed = tuning[12];
  float jump_speed = tuning[13];
  float climb_speed = tuning[14];

  if (on_vine) {
    update_vine(player, p, tuning, grid, level_w, level_h, cell, left, right,
                down, jump_pressed, audio);
    return;
  }

  float feet = y + static_cast<float>(h);
  bool want_crouch = down && (on_ground != 0);

  if (want_crouch && !crouching) {
    crouching = 1;
    h = static_cast<int>(tuning[10]);
    y = feet - static_cast<float>(h);
  } else if (!want_crouch && crouching) {
    float stand_y = feet - tuning[9];
    if (!box_solid(grid, level_w, level_h, x, stand_y, static_cast<float>(w),
                   tuning[9], cell, eps, electric_timer)) {
      crouching = 0;
      h = static_cast<int>(tuning[9]);
      y = stand_y;
    }
  }

  grab_vine(player, p, grid, level_w, level_h, cell, up);
  if (on_vine) {
    return;
  }

  int center_col =
      clamp_int(static_cast<int>((x + static_cast<float>(w) / 2.0f) / cell_f),
                0, level_w - 1);
  int center_row =
      clamp_int(static_cast<int>((y + static_cast<float>(h) / 2.0f) / cell_f),
                0, level_h - 1);
  int lower_row =
      clamp_int(static_cast<int>((y + static_cast<float>(h) - 4.0f) / cell_f),
                0, level_h - 1);
  bool ladder_here =
      tile_at(grid, level_w, level_h, center_row, center_col) == 'H' ||
      tile_at(grid, level_w, level_h, lower_row, center_col) == 'H';

  if (!ladder_here) {
    on_ladder = 0;
  } else if (up || down) {
    on_ladder = 1;
  }

  speed = crouching ? crouch_speed : speed;
  vx = 0;
  if (left) {
    vx = -speed;
    facing = -1;
  }
  if (right) {
    vx = speed;
    facing = 1;
  }

  if (on_ladder) {
    vy = 0;
    if (up) {
      vy = -climb_speed;
    }
    if (down) {
      vy = climb_speed;
    }
    on_ground = 0;
  } else {
    if (on_ground) {
      jumps_used = 0;
    }
    if (jump_pressed && (on_ground || (p[16] && jumps_used < 2))) {
      vy = -jump_speed;
      on_ground = 0;
      jumps_used++;
      if (audio != nullptr) {
        audio->play(SfxJump, jumps_used == 1 ? 1.0f : 1.18f, 70.0f);
      }
    }
    vy += gravity;
    if (vy > terminal) {
      vy = terminal;
    }
  }

  move_player_x(player, p, grid, level_w, level_h, cell, eps, electric_timer,
                vx);
  move_player_y(player, p, grid, level_w, level_h, cell, eps, electric_timer,
                vy);

  char middle = tile_at(grid, level_w, level_h, center_row, center_col);
  float push = 0;
  if (on_ground && p[3] == '>') {
    push = tuning[37];
  } else if (on_ground && p[3] == '<') {
    push = -tuning[37];
  }
  if (middle == '}') {
    push += tuning[38];
  } else if (middle == '{') {
    push -= tuning[38];
  }

  if (push != 0) {
    float old_x = x;
    x += push;
    if (box_solid(grid, level_w, level_h, x, y, static_cast<float>(w),
                  static_cast<float>(h), cell, eps, electric_timer)) {
      x = old_x;
    }
    x = clamp_float(x, 0, static_cast<float>(level_w * cell - w));
  }

  if (middle == 'W' && on_ground && fabs(vx) < 0.1f) {
    waterfall_ticks++;
  } else {
    waterfall_ticks = 0;
  }
  if (waterfall_ticks > 120 && !crouching) {
    float old_feet = y + static_cast<float>(h);
    crouching = 1;
    h = static_cast<int>(tuning[10]);
    y = old_feet - static_cast<float>(h);
  }

  if (y > static_cast<float>(level_h * cell) + 120.0f) {
    p[8] = 0;
  }
}

// ===========================================================================
// Player damage
// ===========================================================================

bool hurt_player(float *player, int *p, int amount, float *tuning, int *game,
                 AudioBank *audio, int sfx_id = SfxHurt) {
  (void)player;
  if (p[11] > 0 || game[0] != 2) {
    return false;
  }

  int damage = amount;
  if (p[12] > 0) {
    damage /= 4;
    if (damage < 1) {
      damage = 1;
    }
  }

  p[8] -= damage;
  p[11] = static_cast<int>(tuning[15]);
  game[11] = 8;
  game[5] = 1;

  if (p[8] <= 0) {
    p[8] = 0;
    game[4]++;
    game[3] = death_penalty(game[3]);
    game[0] = 4;
    if (audio != nullptr) {
      audio->play(SfxDeath, 1.0f, 90.0f);
    }
    return true;
  }
  if (audio != nullptr) {
    audio->play(sfx_id, 1.0f, 85.0f);
  }
  return false;
}

// ===========================================================================
// Enemy spawning, damage, and AI
//
// ef[64][8]: 0 x, 1 y, 2 vx, 3 vy, 4 width, 5 height, 6 water height
// ei[64][9]: 0 active, 1 type, 2 hp, 3 max hp, 4 direction, 5 state,
//            6 timer, 7 grounded, 8 already scored
// ===========================================================================

void spawn_enemy(float ef[64][8], int ei[64][9], int max_enemies,
                 int enemy_types, const float enemy_cfg[13][14], int type,
                 float x, float y) {
  if (type < 0 || type >= enemy_types) {
    return;
  }

  for (int i = 0; i < max_enemies; i++) {
    if (!ei[i][EnemyActive]) {
      ei[i][EnemyActive] = 1;
      ei[i][EnemyType] = type;
      ef[i][0] = x;
      ef[i][1] = y;
      ef[i][2] = 0;
      ef[i][3] = 0;
      ef[i][4] = enemy_cfg[type][0];
      ef[i][5] = enemy_cfg[type][1];
      ef[i][6] = y + ef[i][5];
      ei[i][EnemyHp] = static_cast<int>(enemy_cfg[type][2]);
      ei[i][EnemyMaxHp] = ei[i][EnemyHp];
      ei[i][EnemyDir] = (rand() % 2 == 0) ? -1 : 1;
      ei[i][EnemyState] = 0;
      ei[i][EnemyTimer] = random_int(20, 80);
      ei[i][EnemyGrounded] = 0;
      ei[i][EnemyScored] = 0;
      return;
    }
  }
}

int count_enemies(int ei[64][9], int max_enemies) {
  int total = 0;
  for (int i = 0; i < max_enemies; i++) {
    if (ei[i][EnemyActive]) {
      total++;
    }
  }
  return total;
}

void damage_enemy(float ef[64][8], int ei[64][9], int index, int amount,
                  AudioBank *audio) {
  (void)ef;
  if (index < 0 || index >= 64) {
    return;
  }
  if (!ei[index][EnemyActive]) {
    return;
  }
  ei[index][EnemyHp] -= amount;
  if (audio != nullptr) {
    audio->play(SfxHit, 1.05f, 65.0f);
  }
  if (ei[index][EnemyHp] <= 0) {
    ei[index][EnemyHp] = 0;
    ei[index][EnemyActive] = 0;
    ei[index][EnemyState] = 4;
    ei[index][EnemyTimer] = 0;
    ei[index][EnemyScored] = 1;
  }
}

void enemy_shoot(float ef[64][8], int ei[64][9], int i, float *player, int *p,
                 float sf[128][4], int si[128][4], int max_shots, int kind,
                 float speed, int damage) {
  (void)ei;
  float start_x = ef[i][0] + ef[i][4] / 2.0f;
  float start_y = ef[i][1] + ef[i][5] / 2.0f;
  float dx = player[0] + static_cast<float>(p[0]) / 2.0f - start_x;
  float dy = player[1] + static_cast<float>(p[1]) / 2.0f - start_y;
  float length = sqrt(dx * dx + dy * dy);
  if (length < 1.0f) {
    length = 1.0f;
  }

  for (int s = 0; s < max_shots; s++) {
    if (!si[s][ShotActive]) {
      si[s][ShotActive] = 1;
      si[s][ShotKind] = kind;
      si[s][ShotDamage] = damage;
      si[s][ShotLife] = 180;
      sf[s][0] = start_x;
      sf[s][1] = start_y;
      sf[s][2] = dx / length * speed;
      sf[s][3] = dy / length * speed;
      return;
    }
  }
}

void enemy_gravity(float ef[64][8], int ei[64][9], int i, char **grid,
                   int level_w, int level_h, int cell, float gravity,
                   float terminal, float eps, float scale) {
  float &x = ef[i][0];
  float &y = ef[i][1];
  float &vy = ef[i][3];
  float &w = ef[i][4];
  float &h = ef[i][5];
  float cell_f = static_cast<float>(cell);

  float old_bottom = y + h;
  vy += gravity * scale;
  if (vy > terminal) {
    vy = terminal;
  }

  y += vy * scale;
  ei[i][EnemyGrounded] = 0;

  if (vy >= 0) {
    int row = static_cast<int>((y + h) / cell_f);
    int left = static_cast<int>((x + 3.0f) / cell_f);
    int right = static_cast<int>((x + w - 3.0f) / cell_f);
    for (int col = left; col <= right; col++) {
      char c = tile_at(grid, level_w, level_h, row, col);
      bool floor_here =
          tile_solid(grid, level_w, level_h, row, col, 999999) || c == '=';
      if (floor_here && old_bottom <= static_cast<float>(row * cell) + eps) {
        y = static_cast<float>(row * cell) - h;
        vy = 0;
        ei[i][EnemyGrounded] = 1;
        break;
      }
    }
  }
}

void enemy_move_x(float ef[64][8], int ei[64][9], int i, char **grid,
                  int level_w, int level_h, int cell, float scale) {
  float &x = ef[i][0];
  float &y = ef[i][1];
  float &vx = ef[i][2];
  float &w = ef[i][4];
  float &h = ef[i][5];
  int &dir = ei[i][EnemyDir];
  float cell_f = static_cast<float>(cell);

  float next_x = x + vx * scale;
  int row = static_cast<int>((y + h - 2.0f) / cell_f);
  int col = vx > 0 ? static_cast<int>((next_x + w - 1.0f) / cell_f)
                   : static_cast<int>(next_x / cell_f);

  if (tile_solid(grid, level_w, level_h, row, col, 999999)) {
    dir = -dir;
    vx = -vx;
    return;
  }

  x = next_x;

  if (ei[i][EnemyGrounded] && (ei[i][EnemyType] == 0 || ei[i][EnemyType] == 3 ||
                               ei[i][EnemyType] == 9)) {
    int below_col = vx > 0 ? static_cast<int>((x + w) / cell_f)
                           : static_cast<int>(x / cell_f);
    int below_row = static_cast<int>((y + h + 2.0f) / cell_f);
    char c = tile_at(grid, level_w, level_h, below_row, below_col);
    bool floor_here =
        tile_solid(grid, level_w, level_h, below_row, below_col, 999999) ||
        c == '=';
    if (!floor_here) {
      dir = -dir;
      vx = -vx;
    }
  }

  x = clamp_float(x, 0, static_cast<float>(level_w * cell) - w);
}

void update_enemy(float ef[64][8], int ei[64][9], int i, int enemy_types,
                  const float enemy_cfg[13][14], float *tuning, char **grid,
                  int level_w, int level_h, float *player, int *p,
                  float sf[128][4], int si[128][4], int max_shots,
                  float scale) {
  int type = ei[i][EnemyType];
  if (type < 0 || type >= enemy_types) {
    return;
  }

  const float *cfg = enemy_cfg[type];
  float &x = ef[i][0];
  float &y = ef[i][1];
  float &vx = ef[i][2];
  float &vy = ef[i][3];
  float &w = ef[i][4];
  float &h = ef[i][5];
  int &dir = ei[i][EnemyDir];
  int &state = ei[i][EnemyState];
  int &timer = ei[i][EnemyTimer];

  int cell = static_cast<int>(tuning[2]);
  float gravity = tuning[4];
  float terminal = tuning[5];
  float eps = tuning[6];
  float target_x = player[0] + static_cast<float>(p[0]) / 2.0f;
  float target_y = player[1] + static_cast<float>(p[1]) / 2.0f;
  float cx = x + w / 2.0f;
  float cy = y + h / 2.0f;

  switch (type) {
  case 0:
    vx = static_cast<float>(dir) * cfg[4];
    enemy_gravity(ef, ei, i, grid, level_w, level_h, cell, gravity, terminal,
                  eps, scale);
    enemy_move_x(ef, ei, i, grid, level_w, level_h, cell, scale);
    break;

  case 1:
    enemy_gravity(ef, ei, i, grid, level_w, level_h, cell, gravity, terminal,
                  eps, scale);
    if (ei[i][EnemyGrounded]) {
      vx = 0;
      timer--;
      if (timer <= 0 && fabs(target_x - cx) < cfg[9]) {
        dir = target_x < cx ? -1 : 1;
        vx = static_cast<float>(dir) * cfg[6];
        vy = -cfg[7];
        timer = static_cast<int>(cfg[8]);
      }
    }
    enemy_move_x(ef, ei, i, grid, level_w, level_h, cell, scale);
    break;

  case 2: {
    timer++;
    dir = target_x < cx ? -1 : 1;
    vx = static_cast<float>(dir) * cfg[6];
    float drift;
    if (target_y > cy + cfg[7]) {
      drift = cfg[7];
    } else if (target_y < cy - cfg[7]) {
      drift = -cfg[7];
    } else {
      drift = 0;
    }
    x += vx * scale;
    y += (drift + sin(static_cast<float>(timer) * cfg[8]) * cfg[9]) * scale;
    x = clamp_float(x, 0, static_cast<float>(level_w * cell) - w);
    y = clamp_float(y, 0, static_cast<float>(level_h * cell) - h);
    break;
  }

  case 3:
    enemy_gravity(ef, ei, i, grid, level_w, level_h, cell, gravity, terminal,
                  eps, scale);
    if (state == 0) {
      vx = static_cast<float>(dir) * cfg[4];
      enemy_move_x(ef, ei, i, grid, level_w, level_h, cell, scale);
      if (fabs(target_x - cx) < cfg[7] && fabs(target_y - cy) < cfg[8]) {
        dir = target_x < cx ? -1 : 1;
        vx = static_cast<float>(dir) * cfg[6];
        state = 1;
        timer = static_cast<int>(cfg[9]);
      }
    } else if (state == 1) {
      enemy_move_x(ef, ei, i, grid, level_w, level_h, cell, scale);
      timer--;
      if (timer <= 0) {
        vx = 0;
        state = 2;
        timer = static_cast<int>(cfg[10]);
      }
    } else {
      timer--;
      if (timer <= 0) {
        state = 0;
      }
    }
    break;

  case 4:
    enemy_gravity(ef, ei, i, grid, level_w, level_h, cell, gravity, terminal,
                  eps, scale);
    dir = target_x < cx ? -1 : 1;
    if (timer > 0) {
      timer--;
    }
    if (timer <= 0 && fabs(target_x - cx) < cfg[6] &&
        fabs(target_y - cy) < cfg[7]) {
      enemy_shoot(ef, ei, i, player, p, sf, si, max_shots, 0, tuning[33],
                  static_cast<int>(tuning[34]));
      timer = static_cast<int>(cfg[8]);
    }
    break;

  case 5: {
    enemy_gravity(ef, ei, i, grid, level_w, level_h, cell, gravity, terminal,
                  eps, scale);
    float distance = fabs(target_x - cx);
    if (state == 0) {
      if (distance < cfg[8] && fabs(target_y - cy) < cfg[9]) {
        dir = target_x < cx ? -1 : 1;
        if (distance < cfg[10]) {
          vx = 0;
          state = 1;
          timer = static_cast<int>(cfg[11]);
        } else {
          vx = static_cast<float>(dir) * cfg[6];
          enemy_move_x(ef, ei, i, grid, level_w, level_h, cell, scale);
        }
      } else {
        vx = 0;
      }
    } else if (state == 1) {
      vx = 0;
      timer--;
      if (timer <= 0) {
        vx = static_cast<float>(dir) * cfg[7];
        state = 2;
        timer = static_cast<int>(cfg[12]);
      }
    } else if (state == 2) {
      enemy_move_x(ef, ei, i, grid, level_w, level_h, cell, scale);
      timer--;
      if (timer <= 0) {
        vx = 0;
        state = 3;
        timer = static_cast<int>(cfg[13]);
      }
    } else {
      timer--;
      if (timer <= 0) {
        state = 0;
      }
    }
    break;
  }

  case 6:
    if (state == 0) {
      timer--;
      if (timer <= 0) {
        dir = target_x < cx ? -1 : 1;
        vy = -cfg[6];
        vx = static_cast<float>(dir) * 2.0f;
        state = 1;
        enemy_shoot(ef, ei, i, player, p, sf, si, max_shots, 5, 5.0f, 15);
      }
    } else {
      vy += gravity * scale;
      y += vy * scale;
      x += vx * scale;
      if (y + h >= ef[i][6]) {
        y = ef[i][6] - h;
        vy = 0;
        state = 0;
        timer = static_cast<int>(cfg[7]);
      }
    }
    break;

  case 7:
    enemy_gravity(ef, ei, i, grid, level_w, level_h, cell, gravity, terminal,
                  eps, scale);
    if (fabs(target_x - cx) < cfg[6]) {
      dir = target_x < cx ? -1 : 1;
      vx = static_cast<float>(dir) * cfg[4];
    } else {
      vx = 0;
    }
    if (ei[i][EnemyGrounded] && timer <= 0 && fabs(target_x - cx) < cfg[7]) {
      vy = -9.0f;
      vx = static_cast<float>(dir) * 3.0f;
      timer = 150;
    } else {
      timer--;
    }
    enemy_move_x(ef, ei, i, grid, level_w, level_h, cell, scale);
    break;

  case 8:
    enemy_gravity(ef, ei, i, grid, level_w, level_h, cell, gravity, terminal,
                  eps, scale);
    dir = target_x < cx ? -1 : 1;
    if (ei[i][EnemyGrounded]) {
      vx = static_cast<float>(dir) * cfg[4];
      timer--;
      if (timer <= 0 && fabs(target_x - cx) < cfg[9]) {
        vy = -cfg[6];
        vx = static_cast<float>(dir) * cfg[7];
        timer = static_cast<int>(cfg[8]);
      }
    }
    enemy_move_x(ef, ei, i, grid, level_w, level_h, cell, scale);
    break;

  case 9:
    enemy_gravity(ef, ei, i, grid, level_w, level_h, cell, gravity, terminal,
                  eps, scale);
    if (fabs(target_x - cx) < 520.0f) {
      dir = target_x < cx ? -1 : 1;
    }
    vx = static_cast<float>(dir) * cfg[4];
    enemy_move_x(ef, ei, i, grid, level_w, level_h, cell, scale);
    timer++;
    if (timer % 150 == 0 && fabs(target_x - cx) < 460.0f) {
      enemy_shoot(ef, ei, i, player, p, sf, si, max_shots, 0, 3.0f,
                  static_cast<int>(tuning[34]));
    }
    break;

  case 10:
    timer++;
    dir = target_x < cx ? -1 : 1;
    x += static_cast<float>(dir) * cfg[6] * scale;
    y += sin(static_cast<float>(timer) * 0.05f) * 0.8f * scale;
    if (timer % static_cast<int>(cfg[8]) == 0 && fabs(target_x - cx) < cfg[7]) {
      enemy_shoot(ef, ei, i, player, p, sf, si, max_shots, 3, 4.5f,
                  static_cast<int>(tuning[34]));
    }
    break;

  case 11: {
    enemy_gravity(ef, ei, i, grid, level_w, level_h, cell, gravity, terminal,
                  eps, scale);
    float distance = fabs(target_x - cx);
    if (state == 0) {
      vx = 0;
      if (ei[i][EnemyGrounded] && distance < cfg[6]) {
        dir = target_x < cx ? -1 : 1;
        state = 1;
        timer = static_cast<int>(cfg[9]);
      }
    } else if (state == 1) {
      timer--;
      if (timer <= 0) {
        vy = -cfg[7];
        vx = static_cast<float>(dir) * cfg[8];
        state = 2;
      }
    } else {
      enemy_move_x(ef, ei, i, grid, level_w, level_h, cell, scale);
      if (ei[i][EnemyGrounded]) {
        state = 0;
        timer = static_cast<int>(cfg[10]);
      }
    }
    break;
  }

  case 12:
    enemy_gravity(ef, ei, i, grid, level_w, level_h, cell, gravity, terminal,
                  eps, scale);
    dir = target_x < cx ? -1 : 1;
    if (ei[i][EnemyGrounded]) {
      vx = static_cast<float>(dir) * cfg[4];
      timer--;
      if (timer <= 0) {
        vy = -cfg[6];
        vx = static_cast<float>(dir) * cfg[7];
        timer = static_cast<int>(cfg[8]);
      }
    }
    enemy_move_x(ef, ei, i, grid, level_w, level_h, cell, scale);
    break;

  default:
    break;
  }
}

void update_enemies(float ef[64][8], int ei[64][9], int max_enemies,
                    int enemy_types, const float enemy_cfg[13][14],
                    float *tuning, char **grid, int level_w, int level_h,
                    float *player, int *p, float sf[128][4], int si[128][4],
                    int max_shots, int *game, AudioBank *audio) {
  float scale = p[13] > 0 ? 0.5f : 1.0f;
  float world_w = static_cast<float>(level_w) * tuning[2];
  float world_h = static_cast<float>(level_h) * tuning[2];

  for (int i = 0; i < max_enemies; i++) {
    if (!ei[i][EnemyActive]) {
      continue;
    }
    update_enemy(ef, ei, i, enemy_types, enemy_cfg, tuning, grid, level_w,
                 level_h, player, p, sf, si, max_shots, scale);

    if (ef[i][1] > world_h + 300.0f || ef[i][0] < -800.0f ||
        ef[i][0] > world_w + 800.0f) {
      ei[i][EnemyActive] = 0;
      ei[i][EnemyHp] = 0;
      ei[i][EnemyScored] = 2;
      continue;
    }

    if (boxes_overlap(player[0], player[1], static_cast<float>(p[0]),
                      static_cast<float>(p[1]), ef[i][0], ef[i][1], ef[i][4],
                      ef[i][5])) {
      hurt_player(player, p, static_cast<int>(enemy_cfg[ei[i][EnemyType]][3]),
                  tuning, game, audio);
    }
  }
}

// ===========================================================================
// Shots, effects, and pickups
// ===========================================================================

void spawn_shot(float sf[128][4], int si[128][4], int max_shots, int kind,
                float x, float y, float vx, float vy, int damage, int life) {
  for (int i = 0; i < max_shots; i++) {
    if (!si[i][ShotActive]) {
      si[i][ShotActive] = 1;
      si[i][ShotKind] = kind;
      si[i][ShotDamage] = damage;
      si[i][ShotLife] = life;
      sf[i][0] = x;
      sf[i][1] = y;
      sf[i][2] = vx;
      sf[i][3] = vy;
      return;
    }
  }
}

void spawn_effect(float xf[128][4], int xi[128][4], int max_effects, int kind,
                  float x, float y, int life) {
  for (int i = 0; i < max_effects; i++) {
    if (!xi[i][0]) {
      xi[i][0] = 1;
      xi[i][1] = kind;
      xi[i][2] = life;
      xi[i][3] = life;
      xf[i][0] = x;
      xf[i][1] = y;
      return;
    }
  }
}

void spawn_pickup(float uf[16][3], int ui[16][2], int max_pickups, int kind,
                  float x, float y, int life) {
  for (int i = 0; i < max_pickups; i++) {
    if (!ui[i][0]) {
      ui[i][0] = 1;
      ui[i][1] = kind;
      uf[i][0] = x;
      uf[i][1] = y;
      uf[i][2] = static_cast<float>(life);
      return;
    }
  }
}

void update_effects(float xf[128][4], int xi[128][4], int max_effects) {
  (void)xf;
  for (int i = 0; i < max_effects; i++) {
    if (!xi[i][0]) {
      continue;
    }
    xi[i][2]--;
    if (xi[i][2] <= 0) {
      xi[i][0] = 0;
    }
  }
}

void apply_pickup(int kind, float *player, int *p, float *tuning,
                  AudioBank *audio) {
  (void)player;
  if (kind == 0) {
    p[18] += 3;
    if (p[18] > static_cast<int>(tuning[22])) {
      p[18] = static_cast<int>(tuning[22]);
    }
    if (audio != nullptr) {
      audio->play(SfxPickup, 1.05f, 80.0f);
    }
  } else if (kind == 1) {
    p[8] += 100;
    if (p[8] > static_cast<int>(tuning[7])) {
      p[8] = static_cast<int>(tuning[7]);
    }
    p[10] = p[8] - 100;
    p[12] = static_cast<int>(tuning[41]);
    if (audio != nullptr) {
      audio->play(SfxPickup, 1.0f, 80.0f);
    }
  } else if (kind == 2) {
    p[13] = static_cast<int>(tuning[42]);
    if (audio != nullptr) {
      audio->play(SfxRewind, 0.9f, 85.0f);
    }
  } else if (kind == 3) {
    p[16] = 1;
    if (audio != nullptr) {
      audio->play(SfxPickup, 1.15f, 80.0f);
    }
  } else if (kind == 4) {
    p[14] = static_cast<int>(tuning[43]);
    if (audio != nullptr) {
      audio->play(SfxPickup, 1.1f, 80.0f);
    }
  }
}

void update_pickups(float uf[16][3], int ui[16][2], int max_pickups,
                    float *player, int *p, float *tuning, AudioBank *audio) {
  for (int i = 0; i < max_pickups; i++) {
    if (!ui[i][0]) {
      continue;
    }

    uf[i][2]--;
    bool taken = boxes_overlap(player[0], player[1], static_cast<float>(p[0]),
                               static_cast<float>(p[1]), uf[i][0], uf[i][1],
                               32.0f, 32.0f);
    if (taken) {
      apply_pickup(ui[i][1], player, p, tuning, audio);
    }
    if (taken || uf[i][2] <= 0.0f) {
      ui[i][0] = 0;
    }
  }
}

void explode_shots(float xf[128][4], int xi[128][4], int max_effects, float ex,
                   float ey, float ef[64][8], int ei[64][9], float radius,
                   int damage, float *bossf, int *bossi, AudioBank *audio) {
  spawn_effect(xf, xi, max_effects, 0, ex, ey, 24);
  if (audio != nullptr) {
    audio->play(SfxExplosion, 1.0f, 85.0f);
  }

  for (int i = 0; i < 64; i++) {
    if (!ei[i][EnemyActive]) {
      continue;
    }
    float cx = ef[i][0] + ef[i][4] / 2.0f;
    float cy = ef[i][1] + ef[i][5] / 2.0f;
    float dx = cx - ex;
    float dy = cy - ey;
    if (sqrt(dx * dx + dy * dy) < radius) {
      damage_enemy(ef, ei, i, damage, audio);
    }
  }

  if (bossi[0] > 0 && bossi[7] == 0) {
    float cx = bossf[0] + bossf[4] / 2.0f;
    float cy = bossf[1] + bossf[5] / 2.0f;
    float dx = cx - ex;
    float dy = cy - ey;
    if (sqrt(dx * dx + dy * dy) < radius + bossf[4] / 2.0f) {
      bossi[1] -= damage;
      if (audio != nullptr) {
        audio->play(SfxBossHit, 1.0f, 75.0f);
      }
    }
  }
}

void update_shots(float sf[128][4], int si[128][4], int max_shots,
                  float *tuning, char **grid, int level_w, int level_h,
                  float *player, int *p, float ef[64][8], int ei[64][9],
                  float xf[128][4], int xi[128][4], int max_effects,
                  int enemy_types, float enemy_cfg[13][14], int *game,
                  float *bossf, int *bossi, AudioBank *audio) {
  int cell = static_cast<int>(tuning[2]);
  float cell_f = static_cast<float>(cell);
  float gravity = tuning[4];
  float scale = p[13] > 0 ? 0.5f : 1.0f;

  for (int i = 0; i < max_shots; i++) {
    if (!si[i][ShotActive]) {
      continue;
    }

    int kind = si[i][ShotKind];
    if (kind == 2) {
      float dx = player[0] + static_cast<float>(p[0]) / 2.0f - sf[i][0];
      float dy = player[1] + static_cast<float>(p[1]) / 2.0f - sf[i][1];
      float length = sqrt(dx * dx + dy * dy);
      if (length > 1.0f) {
        sf[i][2] += dx / length * 0.18f;
        sf[i][3] += dy / length * 0.18f;
        float speed = sqrt(sf[i][2] * sf[i][2] + sf[i][3] * sf[i][3]);
        if (speed > 7.0f) {
          sf[i][2] = sf[i][2] / speed * 7.0f;
          sf[i][3] = sf[i][3] / speed * 7.0f;
        }
      }
    }

    if (kind == 4 || kind == 6) {
      sf[i][3] += gravity * 0.5f * scale;
    }

    sf[i][0] += sf[i][2] * scale;
    sf[i][1] += sf[i][3] * scale;
    si[i][ShotLife]--;
    bool dead = false;

    if (kind == 4) {
      int row = static_cast<int>((sf[i][1] + 12.0f) / cell_f);
      int col = static_cast<int>(sf[i][0] / cell_f);
      if (tile_solid(grid, level_w, level_h, row, col, 999999) ||
          tile_at(grid, level_w, level_h, row, col) == '=') {
        sf[i][1] = static_cast<float>(row * cell) - 12.0f;
        sf[i][3] = -fabs(sf[i][3]) * 0.35f;
        sf[i][2] *= 0.8f;
      }
      if (si[i][ShotLife] <= 0) {
        explode_shots(xf, xi, max_effects, sf[i][0], sf[i][1], ef, ei,
                      tuning[26], si[i][ShotDamage], bossf, bossi, audio);
        dead = true;
      }
    } else if (kind == 6) {
      if (si[i][ShotLife] <= 0) {
        spawn_enemy(ef, ei, 64, enemy_types, enemy_cfg, 12, sf[i][0], sf[i][1]);
        dead = true;
      }
    } else {
      int row = static_cast<int>(sf[i][1] / cell_f);
      int col = static_cast<int>(sf[i][0] / cell_f);
      if (tile_solid(grid, level_w, level_h, row, col, 999999)) {
        dead = true;
      }
    }

    if (!dead && kind == 1) {
      for (int e = 0; e < 64; e++) {
        if (ei[e][EnemyActive] &&
            boxes_overlap(sf[i][0] - 6.0f, sf[i][1] - 6.0f, 12.0f, 12.0f,
                          ef[e][0], ef[e][1], ef[e][4], ef[e][5])) {
          damage_enemy(ef, ei, e, si[i][ShotDamage], audio);
          dead = true;
          break;
        }
      }
      if (!dead && bossi[0] > 0 && bossi[7] == 0 &&
          boxes_overlap(sf[i][0] - 6.0f, sf[i][1] - 6.0f, 12.0f, 12.0f,
                        bossf[0], bossf[1], bossf[4], bossf[5])) {
        bossi[1] -= si[i][ShotDamage];
        if (audio != nullptr) {
          audio->play(SfxBossHit, 1.0f, 75.0f);
        }
        dead = true;
      }
    } else if (!dead && kind != 4 && kind != 6) {
      if (boxes_overlap(sf[i][0] - 8.0f, sf[i][1] - 8.0f, 16.0f, 16.0f,
                        player[0], player[1], static_cast<float>(p[0]),
                        static_cast<float>(p[1]))) {
        hurt_player(player, p, si[i][ShotDamage], tuning, game, audio);
        dead = true;
      }
    }

    if (sf[i][0] < -200.0f ||
        sf[i][0] > cell_f * static_cast<float>(level_w) + 200.0f ||
        sf[i][1] < -400.0f ||
        sf[i][1] > cell_f * static_cast<float>(level_h) + 400.0f) {
      dead = true;
    }

    if (dead) {
      si[i][ShotActive] = 0;
    }
  }
}

// ===========================================================================
// Weapons
// ===========================================================================

void cut_vines(float *player, int *p, char **grid, int level_w, int level_h,
               int *tile_timer, int *tile_state, int cell, int whip_range) {
  int facing = p[2];
  float cell_f = static_cast<float>(cell);
  float range_f = static_cast<float>(whip_range);
  float start_x =
      facing > 0 ? player[0] + static_cast<float>(p[0]) : player[0] - range_f;
  int first_col = static_cast<int>(start_x / cell_f);
  int last_col = static_cast<int>((start_x + range_f) / cell_f);
  int top_row =
      clamp_int(static_cast<int>(player[1] / cell_f) - 1, 0, level_h - 1);
  int bottom_row = clamp_int(
      static_cast<int>((player[1] + static_cast<float>(p[1])) / cell_f) + 1, 0,
      level_h - 1);

  for (int row = top_row; row <= bottom_row; row++) {
    for (int col = first_col; col <= last_col; col++) {
      if (tile_at(grid, level_w, level_h, row, col) == 'V') {
        grid[row][col] = 'v';
        int index = row * level_w + col;
        tile_timer[index] = 8 * 60;
        tile_state[index] = 1;
        if (p[7] && p[25] == col) {
          p[7] = 0;
        }
      }
    }
  }
}

void try_whip(float *player, int *p, char **grid, int level_w, int level_h,
              int *tile_timer, int *tile_state, float *tuning, float ef[64][8],
              int ei[64][9], float *bossf, int *bossi, AudioBank *audio) {
  if (p[20] > 0) {
    return;
  }

  int cell = static_cast<int>(tuning[2]);
  int range = static_cast<int>(tuning[16]);
  p[20] = static_cast<int>(tuning[18]);
  p[23] = 8;

  if (audio != nullptr) {
    audio->play(SfxWhip, 1.0f, 75.0f);
  }

  float range_f = static_cast<float>(range);
  float x =
      p[2] > 0 ? player[0] + static_cast<float>(p[0]) : player[0] - range_f;
  float y = player[1] + 10.0f;
  float w = range_f;
  float h = static_cast<float>(p[1]) - 20.0f;

  for (int i = 0; i < 64; i++) {
    if (!ei[i][EnemyActive]) {
      continue;
    }
    if (boxes_overlap(x, y, w, h, ef[i][0], ef[i][1], ef[i][4], ef[i][5])) {
      damage_enemy(ef, ei, i, static_cast<int>(tuning[17]), audio);
    }
  }

  if (bossi[0] > 0 && bossi[7] == 0 &&
      boxes_overlap(x, y, w, h, bossf[0], bossf[1], bossf[4], bossf[5])) {
    bossi[1] -= static_cast<int>(tuning[17]);
    if (audio != nullptr) {
      audio->play(SfxBossHit, 1.0f, 75.0f);
    }
  }

  cut_vines(player, p, grid, level_w, level_h, tile_timer, tile_state, cell,
            range);
}

void fire_gun(float *player, int *p, float *tuning, float sf[128][4],
              int si[128][4], int max_shots, AudioBank *audio) {
  if (p[21] > 0 || p[18] <= 0) {
    return;
  }

  p[21] = static_cast<int>(tuning[20]);
  p[18]--;
  int damage = static_cast<int>(tuning[19]);
  if (p[14] > 0) {
    damage *= 2;
  }

  if (audio != nullptr) {
    audio->play(SfxShot, p[14] > 0 ? 1.12f : 1.0f, 75.0f);
  }

  float x = p[2] > 0 ? player[0] + static_cast<float>(p[0]) : player[0] - 18.0f;
  float y = player[1] + static_cast<float>(p[1]) / 2.0f;
  spawn_shot(sf, si, max_shots, 1, x, y, static_cast<float>(p[2]) * tuning[27],
             0, damage, 180);
}

void throw_grenade(float *player, int *p, float *tuning, float sf[128][4],
                   int si[128][4], int max_shots, AudioBank *audio) {
  if (p[22] > 0 || p[19] <= 0) {
    return;
  }

  p[22] = static_cast<int>(tuning[25]);
  p[19]--;

  if (audio != nullptr) {
    audio->play(SfxGrenade, 1.0f, 80.0f);
  }

  float x = p[2] > 0 ? player[0] + static_cast<float>(p[0]) : player[0] - 24.0f;
  spawn_shot(sf, si, max_shots, 4, x, player[1] + 20.0f,
             static_cast<float>(p[2]) * tuning[28], -9.0f,
             static_cast<int>(tuning[23]), 90);
}

void handle_weapons(float *player, int *p, char **grid, int level_w,
                    int level_h, int *tile_timer, int *tile_state,
                    float *tuning, float ef[64][8], int ei[64][9],
                    float sf[128][4], int si[128][4], int max_shots,
                    float *bossf, int *bossi, bool attack_pressed,
                    bool switch_pressed, AudioBank *audio) {
  if (p[20] > 0) {
    p[20]--;
  }
  if (p[21] > 0) {
    p[21]--;
  }
  if (p[22] > 0) {
    p[22]--;
  }
  if (p[23] > 0) {
    p[23]--;
  }

  if (switch_pressed) {
    p[17]++;
    if (p[17] > 2) {
      p[17] = 0;
    }
    if (audio != nullptr) {
      audio->play(SfxMenu, 1.2f, 60.0f);
    }
  }

  if (!attack_pressed) {
    return;
  }

  if (p[17] == 0) {
    try_whip(player, p, grid, level_w, level_h, tile_timer, tile_state, tuning,
             ef, ei, bossf, bossi, audio);
  } else if (p[17] == 1) {
    fire_gun(player, p, tuning, sf, si, max_shots, audio);
  } else {
    throw_grenade(player, p, tuning, sf, si, max_shots, audio);
  }
}

// ===========================================================================
// Boss phases & behavior
//
// bossf[6]: x, y, vx, vy, width, height
// bossi[8]: phase, hp, max hp, direction, timer, state, grounded, intro
// ===========================================================================

void begin_boss_phase(int phase, float *bossf, int *bossi, float *tuning,
                      int level_w, int level_h, AudioBank *audio) {
  int cell = static_cast<int>(tuning[2]);
  bossi[0] = phase;
  bossi[1] = static_cast<int>(tuning[43 + phase]);
  bossi[2] = bossi[1];
  bossf[4] = tuning[45 + phase * 2];
  bossf[5] = tuning[46 + phase * 2];
  bossf[0] = static_cast<float>(level_w * cell) - bossf[4] - 40.0f;
  bossi[3] = -1;
  bossi[4] = 0;
  bossi[5] = 0;
  bossi[6] = 1;
  bossf[1] = static_cast<float>((level_h - 1) * cell) - bossf[5];
  bossf[2] = 0;
  bossf[3] = 0;
  if (audio != nullptr) {
    audio->play(SfxBossPhase, 1.0f, 85.0f);
  }
}

void update_boss(float *bossf, int *bossi, float *player, int *p,
                 float sf[128][4], int si[128][4], int max_shots, float *tuning,
                 char **grid, int level_w, int level_h, float xf[128][4],
                 int xi[128][4], int max_effects, int *game, AudioBank *audio) {
  (void)grid;
  if (bossi[7] > 0) {
    bossi[7]--;
    return;
  }
  if (bossi[0] <= 0) {
    return;
  }

  int cell = static_cast<int>(tuning[2]);
  float scale = p[13] > 0 ? 0.5f : 1.0f;
  float floor_y = static_cast<float>((level_h - 1) * cell) - bossf[5];
  float target_x = player[0] + static_cast<float>(p[0]) / 2.0f;

  bossi[4]++;

  if (bossi[0] == 1) {
    bossi[3] = target_x < bossf[0] + bossf[4] / 2.0f ? -1 : 1;
    bossf[0] += static_cast<float>(bossi[3]) * 1.4f * scale;
    bossf[1] = floor_y;
    if (bossi[4] % 110 == 0) {
      spawn_shot(sf, si, max_shots, 2, bossf[0] + bossf[4] / 2.0f,
                 bossf[1] + 30.0f, static_cast<float>(bossi[3]) * 4.0f, -2.0f,
                 20, 240);
    }
    if (bossi[4] % 170 == 0) {
      spawn_shot(sf, si, max_shots, 0, bossf[0] + bossf[4] / 2.0f,
                 bossf[1] + 60.0f, static_cast<float>(bossi[3]) * 5.0f, 0, 15,
                 240);
    }
  } else if (bossi[0] == 2) {
    float speed = 2.0f + powf(1.03f, static_cast<float>(bossi[4]) / 60.0f);
    if (speed > 12.0f) {
      speed = 12.0f;
    }
    bossi[3] = target_x < bossf[0] + bossf[4] / 2.0f ? -1 : 1;
    bossf[0] += static_cast<float>(bossi[3]) * speed * scale;
    bossf[1] = floor_y;
    if (bossi[4] % 70 == 0) {
      spawn_shot(sf, si, max_shots, 2, bossf[0] + bossf[4] / 2.0f,
                 bossf[1] + 40.0f, static_cast<float>(bossi[3]) * 4.0f, -1.0f,
                 20, 240);
      spawn_shot(sf, si, max_shots, 2, bossf[0] + bossf[4] / 2.0f,
                 bossf[1] + 80.0f, static_cast<float>(bossi[3]) * 4.0f, 1.0f,
                 20, 240);
    }
  } else {
    bossi[3] = target_x < bossf[0] + bossf[4] / 2.0f ? -1 : 1;
    if (bossi[6] && bossi[4] % 190 == 0) {
      bossf[3] = -15.0f;
      bossf[2] = static_cast<float>(bossi[3]) * 5.0f;
      bossi[5] = 1;
      bossi[6] = 0;
    }

    bossf[3] += tuning[4] * scale;
    bossf[1] += bossf[3] * scale;
    bossf[0] += bossf[2] * scale;

    if (bossf[1] >= floor_y) {
      bool was_falling = bossi[5] == 1;
      bossf[1] = floor_y;
      bossf[3] = 0;
      bossf[2] = 0;
      bossi[6] = 1;
      if (was_falling) {
        bossi[5] = 0;
        spawn_effect(xf, xi, max_effects, 0, bossf[0] + bossf[4] / 2.0f,
                     bossf[1] + bossf[5], 20);
        if (audio != nullptr) {
          audio->play(SfxExplosion, 0.85f, 80.0f);
        }
        if (p[4] && fabs(player[0] - bossf[0]) < 300.0f) {
          hurt_player(player, p, 25, tuning, game, audio);
        }
      }
    }

    if (bossi[1] < bossi[2] / 5 && bossi[4] % 240 == 0) {
      for (int i = 0; i < 3; i++) {
        spawn_shot(sf, si, max_shots, 6, bossf[0] + bossf[4] / 2.0f,
                   bossf[1] + bossf[5] / 2.0f, static_cast<float>(i - 1) * 4.0f,
                   -8.0f, 20, 120);
      }
    }
  }

  bossf[0] = clamp_float(bossf[0], 80.0f,
                         static_cast<float>(level_w * cell) - bossf[4] - 20.0f);

  if (boxes_overlap(player[0], player[1], static_cast<float>(p[0]),
                    static_cast<float>(p[1]), bossf[0], bossf[1], bossf[4],
                    bossf[5])) {
    hurt_player(player, p, 30, tuning, game, audio);
  }
}

// ===========================================================================
// Rendering helpers
// ===========================================================================

void draw_picture(RenderWindow &window, Sprite &sprite, Texture *tex, int index,
                  float x, float y, float w, float h, bool flip, Color color) {
  if (index < 0 || index >= 40 || tex[index].getSize().x <= 0 ||
      tex[index].getSize().y <= 0) {
    return;
  }

  int tex_w = static_cast<int>(tex[index].getSize().x);
  int tex_h = static_cast<int>(tex[index].getSize().y);
  sprite.setTexture(tex[index]);
  sprite.setTextureRect(IntRect(0, 0, tex_w, tex_h));
  sprite.setColor(color);

  float scale_x = w / static_cast<float>(tex_w);
  float scale_y = h / static_cast<float>(tex_h);
  if (flip) {
    sprite.setScale(-scale_x, scale_y);
    sprite.setPosition(x + w, y);
  } else {
    sprite.setScale(scale_x, scale_y);
    sprite.setPosition(x, y);
  }
  window.draw(sprite);
  sprite.setColor(Color::White);
}

void draw_background(RenderWindow &window, Sprite &sprite, Texture *tex,
                     int camera_x, int stage, int screen_w, int screen_h) {
  int index = stage == 0 ? 0 : stage - 1;
  float screen_w_f = static_cast<float>(screen_w);
  float screen_h_f = static_cast<float>(screen_h);
  float offset = fmodf(static_cast<float>(camera_x) * 0.25f, screen_w_f);

  int tex_w = static_cast<int>(tex[index].getSize().x);
  int tex_h = static_cast<int>(tex[index].getSize().y);
  if (tex_w <= 0 || tex_h <= 0) {
    return;
  }

  for (int i = -1; i <= 1; i++) {
    sprite.setTexture(tex[index]);
    sprite.setTextureRect(IntRect(0, 0, tex_w, tex_h));
    sprite.setColor(Color::White);
    sprite.setScale(screen_w_f / static_cast<float>(tex_w),
                    screen_h_f / static_cast<float>(tex_h));
    sprite.setPosition(static_cast<float>(i) * screen_w_f - offset, 0);
    window.draw(sprite);
  }
}

int tile_index(char c) {
  if (c == '#') {
    return 0;
  }
  if (c == '=') {
    return 1;
  }
  if (c == 'H') {
    return 2;
  }
  if (c == 'C') {
    return 3;
  }
  if (c == 'V') {
    return 4;
  }
  if (c == 'v') {
    return 5;
  }
  if (c == '>') {
    return 6;
  }
  if (c == '<') {
    return 7;
  }
  if (c == '}') {
    return 8;
  }
  if (c == '{') {
    return 9;
  }
  if (c == 'W') {
    return 10;
  }
  if (c == 'Z') {
    return 11;
  }
  if (c == 'T') {
    return 12;
  }
  return -1;
}

void draw_tiles(RenderWindow &window, Sprite &sprite, Texture *tex, char **grid,
                int level_w, int level_h, int camera_x, int electric_timer,
                int stage, int cell, int screen_w) {
  int atlas = stage == 0 ? 0 : stage - 1;
  int first_col = camera_x / cell - 1;
  int last_col = first_col + screen_w / cell + 2;
  float cell_f = static_cast<float>(cell);

  RectangleShape block(Vector2f(cell_f, cell_f));

  for (int row = 0; row < level_h; row++) {
    for (int col = first_col; col <= last_col; col++) {
      if (col < 0 || col >= level_w) {
        continue;
      }

      char c = grid[row][col];
      float x = static_cast<float>(col * cell - camera_x);
      float y = static_cast<float>(row * cell);

      if (c == 'X') {
        block.setPosition(x + 4.0f, y + 4.0f);
        block.setSize(Vector2f(56.0f, 56.0f));
        block.setFillColor(Color(255, 210, 60, 220));
        window.draw(block);
        block.setFillColor(Color(90, 50, 0, 220));
        block.setSize(Vector2f(44.0f, 48.0f));
        block.setPosition(x + 10.0f, y + 12.0f);
        window.draw(block);
        block.setSize(Vector2f(cell_f, cell_f));
        continue;
      }

      if (c == 'F') {
        block.setSize(Vector2f(cell_f, cell_f));
        block.setPosition(x, y);
        block.setFillColor(Color(20, 80, 190, 110));
        window.draw(block);
        block.setFillColor(Color(70, 160, 255, 90));
        block.setSize(Vector2f(cell_f, 10.0f));
        block.setPosition(x, y + 8.0f);
        window.draw(block);
        block.setSize(Vector2f(cell_f, cell_f));
        continue;
      }

      if (c == 'P' || c == '.') {
        continue;
      }

      int index = tile_index(c);
      if (index < 0) {
        continue;
      }

      sprite.setTexture(tex[5 + atlas]);
      sprite.setTextureRect(
          IntRect((index % 8) * cell, (index / 8) * cell, cell, cell));
      sprite.setScale(1.0f, 1.0f);
      sprite.setPosition(x, y);
      if (c == 'Z') {
        bool on = (electric_timer / 60) % 5 < 3;
        sprite.setColor(on ? Color(255, 255, 120) : Color(120, 120, 120));
      } else {
        sprite.setColor(Color::White);
      }
      window.draw(sprite);
      sprite.setColor(Color::White);
    }
  }

  if (stage == 0) {
    const char *signs[6] = {"MOVE", "JUMP", "DUCK", "CROUCH", "WHIP", "CLIMB"};
    int cols[6] = {3, 10, 18, 26, 34, 42};
    for (int i = 0; i < 6; i++) {
      draw_text(window, signs[i], static_cast<float>(cols[i] * cell - camera_x),
                330.0f, 2, Color(255, 220, 120));
    }
  }
}

void draw_enemies(RenderWindow &window, Sprite &sprite, Texture *tex,
                  float ef[64][8], int ei[64][9], int max_enemies,
                  int camera_x) {
  float cam_x = static_cast<float>(camera_x);
  for (int i = 0; i < max_enemies; i++) {
    if (!ei[i][EnemyActive]) {
      continue;
    }
    int type = ei[i][EnemyType];
    bool flip = ef[i][2] < 0 || (ef[i][2] == 0 && ei[i][EnemyDir] < 0);
    draw_picture(window, sprite, tex, 10 + type, ef[i][0] - cam_x, ef[i][1],
                 ef[i][4], ef[i][5], flip, Color::White);
  }
}

void draw_shots(RenderWindow &window, Sprite &sprite, Texture *tex,
                float sf[128][4], int si[128][4], int max_shots, int camera_x) {
  float cam_x = static_cast<float>(camera_x);
  for (int i = 0; i < max_shots; i++) {
    if (!si[i][ShotActive]) {
      continue;
    }
    int kind = si[i][ShotKind];
    int index = 28;
    float size = 20.0f;

    if (kind == 0) {
      index = 29;
      size = 18.0f;
    } else if (kind == 1 || kind == 4) {
      index = 28;
      size = (kind == 4) ? 24.0f : 20.0f;
    } else if (kind == 2) {
      index = 30;
      size = 28.0f;
    } else if (kind == 3 || kind == 6) {
      index = 32;
      size = (kind == 6) ? 30.0f : 20.0f;
    } else if (kind == 5) {
      index = 31;
    }

    draw_picture(window, sprite, tex, index, sf[i][0] - cam_x - size / 2.0f,
                 sf[i][1] - size / 2.0f, size, size, sf[i][2] < 0,
                 Color::White);
  }
}

void draw_effects(RenderWindow &window, Sprite &sprite, Texture *tex,
                  float xf[128][4], int xi[128][4], int max_effects,
                  int camera_x) {
  float cam_x = static_cast<float>(camera_x);
  for (int i = 0; i < max_effects; i++) {
    if (!xi[i][0]) {
      continue;
    }
    int max_life = xi[i][3] > 0 ? xi[i][3] : 1;
    float progress =
        1.0f - static_cast<float>(xi[i][2]) / static_cast<float>(max_life);
    float size = 24.0f + progress * 40.0f;
    int index = 33;
    if (xi[i][1] == 1) {
      index = 28;
      size = 18.0f;
    } else if (xi[i][1] == 2) {
      index = 32;
      size = 48.0f;
    }
    draw_picture(window, sprite, tex, index, xf[i][0] - cam_x - size / 2.0f,
                 xf[i][1] - size / 2.0f, size, size, false, Color::White);
  }
}

void draw_pickups(RenderWindow &window, Sprite &sprite, Texture *tex,
                  float uf[16][3], int ui[16][2], int max_pickups, int camera_x,
                  int level_ticks) {
  float cam_x = static_cast<float>(camera_x);
  for (int i = 0; i < max_pickups; i++) {
    if (!ui[i][0]) {
      continue;
    }
    int index = ui[i][1] > 4 ? 0 : ui[i][1];
    float phase = static_cast<float>(level_ticks) + uf[i][0];
    float bob = sin(phase * 0.08f) * 4.0f;
    Color tint = ui[i][1] == 4 ? Color(255, 140, 140) : Color::White;
    draw_picture(window, sprite, tex, 23 + index, uf[i][0] - cam_x,
                 uf[i][1] + bob, 32.0f, 32.0f, false, tint);
  }
}

void draw_player(RenderWindow &window, Sprite &sprite, Texture *tex,
                 float *player, int *p, int camera_x) {
  if (p[11] > 0 && (p[11] / 4) % 2 == 0) {
    return;
  }

  draw_picture(window, sprite, tex, 37,
               player[0] - static_cast<float>(camera_x), player[1],
               static_cast<float>(p[0]), static_cast<float>(p[1]), p[2] < 0,
               Color::White);
}

void draw_boss(RenderWindow &window, Sprite &sprite, Texture *tex, float *bossf,
               int *bossi, int camera_x) {
  if (bossi[0] <= 0) {
    return;
  }
  draw_picture(window, sprite, tex, 34 + bossi[0] - 1,
               bossf[0] - static_cast<float>(camera_x), bossf[1], bossf[4],
               bossf[5], bossi[3] < 0, Color::White);
}

void draw_bar(RenderWindow &window, float x, float y, float w, float h,
              float ratio, Color fill, Color back) {
  RectangleShape bar(Vector2f(w, h));
  bar.setPosition(x, y);
  bar.setFillColor(back);
  window.draw(bar);
  bar.setSize(Vector2f(w * clamp_float(ratio, 0.0f, 1.0f), h));
  bar.setFillColor(fill);
  window.draw(bar);
}

// ===========================================================================
// Stage info & run helpers
// ===========================================================================

const char *stage_path(int stage) {
  if (stage == 0) {
    return "levels/tutorial.txt";
  }
  if (stage == 1) {
    return "levels/level1.txt";
  }
  if (stage == 2) {
    return "levels/level2.txt";
  }
  if (stage == 3) {
    return "levels/level3.txt";
  }
  return "levels/levelx.txt";
}

const char *stage_name(int stage) {
  if (stage == 0) {
    return "TUTORIAL - THE BASICS";
  }
  if (stage == 1) {
    return "LEVEL 1 - THE CONSTRUCTION SITE";
  }
  if (stage == 2) {
    return "LEVEL 2 - THE SEWERS";
  }
  if (stage == 3) {
    return "LEVEL 3 - CLIFFS, WATERFALLS AND JUNGLE";
  }
  return "LEVEL X - THE BIO-TECH LAB";
}

const char *stage_objective(int stage) {
  if (stage == 0) {
    return "MOVE. JUMP. DUCK. CROUCH-RUN. WHIP. CLIMB.";
  }
  if (stage == 1) {
    return "CROSS THE GIRDERS AND REACH THE EXIT.";
  }
  if (stage == 2) {
    return "SURVIVE THE SEWERS AND FIND THE EXIT.";
  }
  if (stage == 3) {
    return "SWING THE VINES AND CROSS THE WATERFALLS.";
  }
  return "KILL THE QUOTA, THEN DESTROY THE BOSS.";
}

void reset_entities(float ef[64][8], int ei[64][9], float sf[128][4],
                    int si[128][4], float xf[128][4], int xi[128][4],
                    float uf[16][3], int ui[16][2]) {
  (void)ef;
  (void)sf;
  (void)xf;
  (void)uf;
  for (int i = 0; i < 64; i++) {
    ei[i][EnemyActive] = 0;
    ei[i][EnemyHp] = 0;
    ei[i][EnemyScored] = 2;
  }
  for (int i = 0; i < 128; i++) {
    si[i][ShotActive] = 0;
    xi[i][0] = 0;
  }
  for (int i = 0; i < 16; i++) {
    ui[i][0] = 0;
  }
}

void reset_run(int *game, float *player, int *p, int kill_time[13][32],
               int *kill_slot) {
  game[3] = 0;
  game[4] = 0;
  game[5] = 0;
  player[0] = 0;
  player[1] = 0;
  p[8] = 100;
  p[10] = 0;
  p[16] = 0;
  p[12] = 0;
  p[13] = 0;
  p[14] = 0;
  for (int i = 0; i < 13; i++) {
    kill_slot[i] = 0;
    for (int j = 0; j < 32; j++) {
      kill_time[i][j] = -100000;
    }
  }
}

void record_kill(int type, int level_ticks, int kill_time[13][32],
                 int *kill_slot) {
  if (type < 0 || type >= 13) {
    return;
  }
  int slot = kill_slot[type] % 32;
  kill_time[type][slot] = level_ticks;
  kill_slot[type]++;
}

int recent_kills(int type, int level_ticks, int window, int kill_time[13][32]) {
  int total = 0;
  for (int i = 0; i < 32; i++) {
    if (kill_time[type][i] >= level_ticks - window) {
      total++;
    }
  }
  return total;
}

int recent_kills_all(int level_ticks, int window, int kill_time[13][32]) {
  int total = 0;
  for (int i = 0; i < 13; i++) {
    total += recent_kills(i, level_ticks, window, kill_time);
  }
  return total;
}

int award_kill(int type, int base_points, int level_ticks,
               int kill_time[13][32], int *kill_slot, bool mid_air) {
  record_kill(type, level_ticks, kill_time, kill_slot);
  int points = base_points;
  int streak = recent_kills_all(level_ticks, 60, kill_time);
  if (streak == 2) {
    points += 200;
  } else if (streak >= 3) {
    points += 500;
  }
  if (mid_air) {
    points += 350;
  }
  return points;
}

int death_penalty(int score) {
  score -= 200;
  if (score < 0) {
    score = 0;
  }
  return score;
}

void prepare_stage(int new_stage, float *tuning, int *game, float *player,
                   int *p, char **&grid, int &level_w, int &level_h,
                   int *&tile_timer, int *&tile_state, float &start_x,
                   float &start_y, int &exit_col, int &exit_row,
                   float ef[64][8], int ei[64][9], float sf[128][4],
                   int si[128][4], float xf[128][4], int xi[128][4],
                   float uf[16][3], int ui[16][2], int *spawn_queue,
                   int &queue_count, int &queue_index, int &spawn_timer,
                   int &spawn_cycle, int spawn_types[5][5],
                   int spawn_counts[5][5], float enemy_cfg[13][14],
                   int enemy_types, int max_enemies, float *bossf, int *bossi,
                   char *message) {
  (void)bossf;
  reset_entities(ef, ei, sf, si, xf, xi, uf, ui);
  free_level(grid, level_h, tile_timer, tile_state);

  int cell = static_cast<int>(tuning[2]);
  grid = load_level(stage_path(new_stage), level_w, level_h, start_x, start_y,
                    cell, static_cast<int>(tuning[9]), exit_col, exit_row,
                    tile_timer, tile_state);
  if (grid == nullptr) {
    printf("Could not load %s\n", stage_path(new_stage));
    return;
  }

  game[1] = new_stage;
  game[6] = 0;
  game[7] = 0;
  game[8] = 0;
  game[9] = 0;
  game[11] = 0;
  game[10] = 120;

  player[0] = start_x;
  player[1] = start_y;
  player[2] = 0;
  player[3] = 0;
  p[0] = static_cast<int>(tuning[8]);
  p[1] = static_cast<int>(tuning[9]);
  p[2] = 1;
  p[3] = '.';
  p[4] = 0;
  p[5] = 0;
  p[6] = 0;
  p[7] = 0;
  if (p[8] < 100) {
    p[8] = 100;
  }
  p[9] = static_cast<int>(tuning[7]);
  p[17] = 0;
  p[18] = static_cast<int>(tuning[21]);
  p[19] = static_cast<int>(tuning[24]);
  p[20] = 0;
  p[21] = 0;
  p[22] = 0;
  p[23] = 0;
  p[24] = 0;
  p[25] = -1;
  p[11] = 90;

  queue_count = 0;
  queue_index = 0;
  spawn_timer = static_cast<int>(tuning[35]);
  spawn_cycle = 0;
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < spawn_counts[new_stage][i]; j++) {
      if (queue_count < 64) {
        spawn_queue[queue_count] = spawn_types[new_stage][i];
        queue_count++;
      }
    }
  }
  for (int i = queue_count - 1; i > 0; i--) {
    int j = random_int(0, i);
    int temp = spawn_queue[i];
    spawn_queue[i] = spawn_queue[j];
    spawn_queue[j] = temp;
  }

  bossi[0] = 0;
  bossi[1] = 0;
  bossi[2] = 0;
  bossi[4] = 0;
  bossi[5] = 0;
  bossi[6] = 1;
  bossi[7] = 0;

  if (new_stage == 0) {
    spawn_enemy(ef, ei, max_enemies, enemy_types, enemy_cfg, 0,
                static_cast<float>(20 * cell), 792.0f);
    spawn_enemy(ef, ei, max_enemies, enemy_types, enemy_cfg, 0,
                static_cast<float>(36 * cell), 792.0f);
  }

  game[0] = 1;
  game[2] = 120;
  snprintf(message, 96, "%s", stage_name(new_stage));
}

// ===========================================================================
// Main game loop
// ===========================================================================

int run_game(float *tuning, float enemy_cfg[13][14], int spawn_types[5][5],
             int spawn_counts[5][5], int endless_types[5], int level_points[5],
             int l1_limit[3], int l1_bonus[3], int lx_limit[3], int lx_bonus[3],
             int boss_limit[3], int boss_bonus[3], int boss_quota_type[5],
             int boss_quota_count[5]) {
  int screen_w = static_cast<int>(tuning[0]);
  int screen_h = static_cast<int>(tuning[1]);
  float screen_w_f = static_cast<float>(screen_w);
  float screen_h_f = static_cast<float>(screen_h);
  int cell = static_cast<int>(tuning[2]);
  float cell_f = static_cast<float>(cell);
  int tick_rate = static_cast<int>(tuning[3]);
  int max_enemies = static_cast<int>(tuning[30]);
  int max_shots = static_cast<int>(tuning[31]);
  int max_effects = 128;
  int max_pickups = static_cast<int>(tuning[32]);
  int enemy_types = static_cast<int>(tuning[29]);

  Texture tex[40];
  Sprite sprite;

  const char *paths[40] = {"assets/backgrounds/level1.png",
                           "assets/backgrounds/level2.png",
                           "assets/backgrounds/level3.png",
                           "assets/backgrounds/levelx.png",
                           "assets/backgrounds/menu.png",
                           "assets/tiles/level1.png",
                           "assets/tiles/level2.png",
                           "assets/tiles/level3.png",
                           "assets/tiles/levelx.png",
                           "assets/tiles/levelx.png",
                           "assets/sprites/enemies/spiked_crawler.png",
                           "assets/sprites/enemies/blue_jumper.png",
                           "assets/sprites/enemies/flying_insect.png",
                           "assets/sprites/enemies/leopard.png",
                           "assets/sprites/enemies/green_shooter.png",
                           "assets/sprites/enemies/tribal_warrior.png",
                           "assets/sprites/enemies/leaping_fish.png",
                           "assets/sprites/enemies/brute.png",
                           "assets/sprites/enemies/jack.png",
                           "assets/sprites/enemies/sewer_rat.png",
                           "assets/sprites/enemies/orb_boss.png",
                           "assets/sprites/enemies/projectile_leaper.png",
                           "assets/sprites/bosses/minion.png",
                           "assets/sprites/items/ammo.png",
                           "assets/sprites/items/health.png",
                           "assets/sprites/items/time_dilation.png",
                           "assets/sprites/items/double_jump.png",
                           "assets/sprites/items/rewind_token.png",
                           "assets/sprites/fx/bullet.png",
                           "assets/sprites/fx/rocket.png",
                           "assets/sprites/fx/fireball.png",
                           "assets/sprites/fx/orb.png",
                           "assets/sprites/fx/explosion_0.png",
                           "assets/sprites/fx/enemy_bolt.png",
                           "assets/sprites/bosses/metal_tank.png",
                           "assets/sprites/bosses/boss_humanoid.png",
                           "assets/sprites/bosses/skeleton.png",
                           "assets/sprites/player/hart_sprite.png",
                           nullptr,
                           nullptr};

  for (int i = 0; i < 38; i++) {
    if (!tex[i].loadFromFile(paths[i])) {
      printf("Could not load %s\n", paths[i]);
      return 1;
    }
  }

  AudioBank audio;
  const char *sfx_paths[15] = {
      "assets/music/jump.wav",      "assets/music/whip.wav",
      "assets/music/shot.wav",      "assets/music/grenade.wav",
      "assets/music/explosion.wav", "assets/music/hit.wav",
      "assets/music/hurt.wav",      "assets/music/death.wav",
      "assets/music/pickup.wav",    "assets/music/clear.wav",
      "assets/music/boss_hit.wav",  "assets/music/boss_phase.wav",
      "assets/music/menu.wav",      "assets/music/rewind.wav",
      "assets/music/zap.wav"};

  audio.enabled = true;
  for (int i = 0; i < 15; i++) {
    if (!audio.buffers[i].loadFromFile(sfx_paths[i])) {
      printf("Warning: could not load %s\n", sfx_paths[i]);
      audio.enabled = false;
    }
  }

  Music bgm_menu;
  Music bgm_level;
  bool menu_music_ok = bgm_menu.openFromFile("assets/music/music_menu.wav");
  bool level_music_ok = bgm_level.openFromFile("assets/music/music_level1.ogg");
  if (menu_music_ok) {
    bgm_menu.setLoop(true);
    bgm_menu.setVolume(45.0f);
  }
  if (level_music_ok) {
    bgm_level.setLoop(true);
    bgm_level.setVolume(40.0f);
  }

  int game[13] = {};
  float player[8] = {};
  int p[32] = {};
  char **grid = nullptr;
  int level_w = 0;
  int level_h = 0;
  int *tile_timer = nullptr;
  int *tile_state = nullptr;
  float start_x = 0;
  float start_y = 0;
  int exit_col = -1;
  int exit_row = -1;

  float ef[64][8] = {};
  int ei[64][9] = {};
  float sf[128][4] = {};
  int si[128][4] = {};
  float xf[128][4] = {};
  int xi[128][4] = {};
  float uf[16][3] = {};
  int ui[16][2] = {};

  float bossf[6] = {};
  int bossi[8] = {};

  int spawn_queue[64] = {};
  int queue_count = 0;
  int queue_index = 0;
  int spawn_timer = 0;
  int spawn_cycle = 0;
  int kill_time[13][32];
  int kill_slot[13] = {};
  for (int i = 0; i < 13; i++) {
    for (int j = 0; j < 32; j++) {
      kill_time[i][j] = -100000;
    }
  }

  char message[96] = "PRESS SPACE TO START";

  float &px = player[0];
  float &py = player[1];
  float &pvx = player[2];
  float &pvy = player[3];
  int &pw = p[0];
  int &ph = p[1];
  int &ground_char = p[3];
  int &on_ground = p[4];
  int &crouching = p[5];
  int &on_ladder = p[6];
  int &on_vine = p[7];
  int &hp = p[8];
  int &bonus_hp = p[10];
  int &invuln = p[11];
  int &shield_timer = p[12];
  int &dilation_timer = p[13];
  int &boost_timer = p[14];
  int &double_jump = p[16];
  int &weapon = p[17];
  int &ammo = p[18];
  int &grenades = p[19];

  float &boss_x = bossf[0];
  float &boss_y = bossf[1];
  float &boss_w = bossf[4];
  float &boss_h = bossf[5];
  int &boss_phase = bossi[0];
  int &boss_hp = bossi[1];
  int &boss_max_hp = bossi[2];
  int &boss_intro = bossi[7];

  int &state = game[0];
  int &stage = game[1];
  int &state_timer = game[2];
  int &score = game[3];
  int &deaths = game[4];
  int &damaged = game[5];
  int &level_ticks = game[6];
  int &camera_x = game[7];
  int &electric_timer = game[8];
  int &health_timer = game[9];
  int &message_timer = game[10];
  int &damage_flash = game[11];
  int &boss_start = game[12];

  RenderWindow window(VideoMode(static_cast<unsigned int>(screen_w),
                                static_cast<unsigned int>(screen_h)),
                      "VICE: Project Doom");
  window.setKeyRepeatEnabled(false);

  Clock clock;
  float accumulator = 0;
  float step = 1.0f / static_cast<float>(tick_rate);
  bool jump_held = false;
  bool attack_held = false;
  bool switch_held = false;

  while (window.isOpen()) {
    Event event;
    while (window.pollEvent(event)) {
      if (event.type == Event::Closed) {
        window.close();
      }
    }

    bool move_left = Keyboard::isKeyPressed(Keyboard::Left);
    bool move_right = Keyboard::isKeyPressed(Keyboard::Right);
    bool move_up = Keyboard::isKeyPressed(Keyboard::Up);
    bool move_down = Keyboard::isKeyPressed(Keyboard::Down);

    bool jump_down = Keyboard::isKeyPressed(Keyboard::Space);
    bool attack_down = Keyboard::isKeyPressed(Keyboard::J);
    bool switch_down = Keyboard::isKeyPressed(Keyboard::Q);

    bool jump_triggered = jump_down && !jump_held;
    bool attack_triggered = attack_down && !attack_held;
    bool switch_triggered = switch_down && !switch_held;

    jump_held = jump_down;
    attack_held = attack_down;
    switch_held = switch_down;

    float elapsed = clock.restart().asSeconds();
    if (elapsed > 0.25f) {
      elapsed = 0.25f;
    }
    accumulator += elapsed;

    int guard = 0;
    while (accumulator >= step && guard < 6) {
      accumulator -= step;
      guard++;

      bool step_jump = jump_triggered;
      bool step_attack = attack_triggered;
      bool step_switch = switch_triggered;
      jump_triggered = false;
      attack_triggered = false;
      switch_triggered = false;

      if (state == 0) {
        if (step_jump) {
          audio.play(SfxMenu, 1.0f, 80.0f);
          reset_run(game, player, p, kill_time, kill_slot);
          prepare_stage(0, tuning, game, player, p, grid, level_w, level_h,
                        tile_timer, tile_state, start_x, start_y, exit_col,
                        exit_row, ef, ei, sf, si, xf, xi, uf, ui, spawn_queue,
                        queue_count, queue_index, spawn_timer, spawn_cycle,
                        spawn_types, spawn_counts, enemy_cfg, enemy_types,
                        max_enemies, bossf, bossi, message);
        }
        continue;
      }

      if (state == 1) {
        if (state_timer > 0) {
          state_timer--;
        }
        if (state_timer <= 0) {
          state = 2;
        }
        continue;
      }

      if (state == 3) {
        if (state_timer > 0) {
          state_timer--;
        }
        if (state_timer <= 0) {
          if (stage < 4) {
            prepare_stage(stage + 1, tuning, game, player, p, grid, level_w,
                          level_h, tile_timer, tile_state, start_x, start_y,
                          exit_col, exit_row, ef, ei, sf, si, xf, xi, uf, ui,
                          spawn_queue, queue_count, queue_index, spawn_timer,
                          spawn_cycle, spawn_types, spawn_counts, enemy_cfg,
                          enemy_types, max_enemies, bossf, bossi, message);
          } else {
            state = 5;
          }
        }
        continue;
      }

      if (state == 4) {
        if (step_jump) {
          audio.play(SfxRewind, 1.0f, 80.0f);
          px = start_x;
          py = start_y;
          pvx = 0;
          pvy = 0;
          ph = static_cast<int>(tuning[9]);
          crouching = 0;
          on_ground = 0;
          on_ladder = 0;
          on_vine = 0;
          hp = 100 + bonus_hp;
          if (hp > static_cast<int>(tuning[7])) {
            hp = static_cast<int>(tuning[7]);
          }
          invuln = 120;
          ammo = static_cast<int>(tuning[21]);
          grenades = static_cast<int>(tuning[24]);
          if (stage == 4 && boss_phase > 0) {
            reset_entities(ef, ei, sf, si, xf, xi, uf, ui);
            begin_boss_phase(1, bossf, bossi, tuning, level_w, level_h, &audio);
            boss_intro = 120;
          }
          state = 2;
        }
        continue;
      }

      if (state == 5) {
        if (step_jump) {
          audio.play(SfxMenu, 1.0f, 80.0f);
          state = 0;
        }
        continue;
      }

      // Active gameplay tick.
      level_ticks++;
      electric_timer++;
      if (invuln > 0) {
        invuln--;
      }
      if (shield_timer > 0) {
        shield_timer--;
      }
      if (dilation_timer > 0) {
        dilation_timer--;
      }
      if (boost_timer > 0) {
        boost_timer--;
      }
      if (message_timer > 0) {
        message_timer--;
      }
      if (damage_flash > 0) {
        damage_flash--;
      }

      update_player(player, p, grid, level_w, level_h, electric_timer, tuning,
                    move_left, move_right, move_up, move_down, step_jump,
                    &audio);
      handle_weapons(player, p, grid, level_w, level_h, tile_timer, tile_state,
                     tuning, ef, ei, sf, si, max_shots, bossf, bossi,
                     step_attack, step_switch, &audio);
      update_enemies(ef, ei, max_enemies, enemy_types, enemy_cfg, tuning, grid,
                     level_w, level_h, player, p, sf, si, max_shots, game,
                     &audio);
      update_shots(sf, si, max_shots, tuning, grid, level_w, level_h, player, p,
                   ef, ei, xf, xi, max_effects, enemy_types, enemy_cfg, game,
                   bossf, bossi, &audio);
      update_effects(xf, xi, max_effects);
      update_pickups(uf, ui, max_pickups, player, p, tuning, &audio);
      update_boss(bossf, bossi, player, p, sf, si, max_shots, tuning, grid,
                  level_w, level_h, xf, xi, max_effects, game, &audio);

      if (state == 4) {
        for (int i = 0; i < max_shots; i++) {
          si[i][ShotActive] = 0;
        }
        on_vine = 0;
        on_ladder = 0;
      }

      if (hp <= 0) {
        hp = 0;
        if (state == 2) {
          audio.play(SfxDeath, 1.0f, 90.0f);
          deaths++;
          score = death_penalty(score);
          state = 4;
          for (int i = 0; i < max_shots; i++) {
            si[i][ShotActive] = 0;
          }
          on_vine = 0;
          on_ladder = 0;
        }
      }

      int player_left = static_cast<int>(px / cell_f);
      int player_right =
          static_cast<int>((px + static_cast<float>(pw)) / cell_f);
      int player_top = static_cast<int>(py / cell_f);
      int player_bottom =
          static_cast<int>((py + static_cast<float>(ph)) / cell_f);
      for (int row = player_top; row <= player_bottom; row++) {
        for (int col = player_left; col <= player_right; col++) {
          if (tile_at(grid, level_w, level_h, row, col) == 'Z' &&
              tile_solid(grid, level_w, level_h, row, col, electric_timer)) {
            hurt_player(player, p, 12, tuning, game, &audio, SfxZap);
          }
        }
      }

      // Crumbling girders, regrowth of vines, and ambush spawners.
      for (int row = 0; row < level_h; row++) {
        for (int col = 0; col < level_w; col++) {
          int index = row * level_w + col;
          char c = grid[row][col];
          if (c == 'C' || tile_state[index] == 2) {
            if (tile_state[index] == 1) {
              tile_timer[index]--;
              if (tile_timer[index] <= 0) {
                grid[row][col] = '.';
                tile_state[index] = 2;
                tile_timer[index] = 6 * 60;
              }
            } else if (tile_state[index] == 2) {
              tile_timer[index]--;
              if (tile_timer[index] <= 0) {
                grid[row][col] = 'C';
                tile_state[index] = 0;
              }
            }
          } else if (c == 'v') {
            tile_timer[index]--;
            if (tile_timer[index] <= 0) {
              grid[row][col] = 'V';
              tile_state[index] = 0;
            }
          } else if (c == 'T' && tile_state[index] == 0) {
            float tx = static_cast<float>(col * cell);
            float ty = static_cast<float>(row * cell);
            if (fabs(px - tx) < 220.0f && fabs(py - ty) < 180.0f) {
              spawn_enemy(ef, ei, max_enemies, enemy_types, enemy_cfg, 7, tx,
                          ty - 120.0f);
              grid[row][col] = '.';
              tile_state[index] = 1;
            }
          }
        }
      }

      if (ground_char == 'C') {
        int row =
            static_cast<int>((py + static_cast<float>(ph) + 2.0f) / cell_f);
        int col =
            static_cast<int>((px + static_cast<float>(pw) / 2.0f) / cell_f);
        if (row >= 0 && row < level_h && col >= 0 && col < level_w) {
          int index = row * level_w + col;
          if (tile_state[index] == 0) {
            tile_state[index] = 1;
            tile_timer[index] = 45;
          }
        }
      }

      // Award kills once and let crawlers drop pickups.
      for (int i = 0; i < max_enemies; i++) {
        if (!ei[i][EnemyActive] && ei[i][EnemyHp] <= 0 &&
            ei[i][EnemyScored] == 1) {
          ei[i][EnemyScored] = 2;
          int type = ei[i][EnemyType];
          score +=
              award_kill(type, static_cast<int>(enemy_cfg[type][5]),
                         level_ticks, kill_time, kill_slot, on_ground == 0);
          spawn_effect(xf, xi, max_effects, 0, ef[i][0] + ef[i][4] / 2.0f,
                       ef[i][1] + ef[i][5] / 2.0f, 18);
          if (type == 0 && rand() % 100 < 35) {
            int kind = rand() % 100 < 70 ? 0 : 4;
            spawn_pickup(uf, ui, max_pickups, kind, ef[i][0], ef[i][1],
                         10 * 60);
          }
        }
      }

      // Boss defeat and phase progression.
      if (boss_phase > 0 && boss_intro == 0 && boss_hp <= 0) {
        spawn_effect(xf, xi, max_effects, 2, boss_x + boss_w / 2.0f,
                     boss_y + boss_h / 2.0f, 30);
        audio.play(SfxExplosion, 0.9f, 90.0f);
        int next = boss_phase + 1;
        if (next > 3) {
          audio.play(SfxClear, 1.0f, 90.0f);
          score += 5000;
          float seconds = static_cast<float>(level_ticks - boss_start) /
                          static_cast<float>(tick_rate);
          for (int i = 0; i < 3; i++) {
            if (seconds < static_cast<float>(boss_limit[i])) {
              score += boss_bonus[i];
              break;
            }
          }
          if (deaths == 0) {
            score += static_cast<int>(tuning[55]);
          }
          if (!damaged) {
            score += static_cast<int>(tuning[56]);
          }
          boss_phase = 0;
          state = 5;
          snprintf(message, sizeof(message), "VICTORY");
        } else {
          begin_boss_phase(next, bossf, bossi, tuning, level_w, level_h,
                           &audio);
          boss_intro = 120;
          message_timer = 120;
          snprintf(message, sizeof(message), "BOSS PHASE %d", next);
        }
      }

      // Enemy spawning (endless rush on Level X, queued waves otherwise).
      float world_w_f = static_cast<float>(level_w * cell);
      if (stage == 4 && boss_phase == 0 && boss_intro == 0) {
        spawn_timer--;
        if (spawn_timer <= 0) {
          int batch = 1 << (spawn_cycle / 3);
          if (batch > static_cast<int>(tuning[36])) {
            batch = static_cast<int>(tuning[36]);
          }
          for (int i = 0; i < batch; i++) {
            int type = endless_types[random_int(0, 4)];
            float w = enemy_cfg[type][0];
            float h = enemy_cfg[type][1];
            float x = (rand() % 2 == 0)
                          ? static_cast<float>(camera_x) - w - 50.0f
                          : static_cast<float>(camera_x) + screen_w_f + 50.0f;
            if (x < 0) {
              x = static_cast<float>(camera_x) + screen_w_f + 50.0f;
            }
            if (x + w > world_w_f) {
              x = static_cast<float>(camera_x) - w - 50.0f;
            }
            x = clamp_float(x, 0, world_w_f - w);
            float y = find_ground_y(grid, level_w, level_h, x, h, cell);
            if (type == 2 || type == 10) {
              y = static_cast<float>(random_int(80, 360));
            }
            spawn_enemy(ef, ei, max_enemies, enemy_types, enemy_cfg, type, x,
                        y);
          }
          spawn_cycle++;
          spawn_timer =
              std::max(30, static_cast<int>(tuning[35]) - spawn_cycle * 2);
        }

        bool ready = true;
        for (int i = 0; i < 5; i++) {
          if (recent_kills(boss_quota_type[i], level_ticks,
                           static_cast<int>(tuning[54]),
                           kill_time) < boss_quota_count[i]) {
            ready = false;
          }
        }
        if (ready) {
          for (int i = 0; i < max_enemies; i++) {
            ei[i][EnemyActive] = 0;
          }
          for (int i = 0; i < max_shots; i++) {
            si[i][ShotActive] = 0;
          }
          score += level_points[4];
          float clear_seconds =
              static_cast<float>(level_ticks) / static_cast<float>(tick_rate);
          for (int i = 0; i < 3; i++) {
            if (clear_seconds < static_cast<float>(lx_limit[i])) {
              score += lx_bonus[i];
              break;
            }
          }
          boss_start = level_ticks;
          begin_boss_phase(1, bossf, bossi, tuning, level_w, level_h, &audio);
          boss_intro = static_cast<int>(tuning[53]);
          message_timer = boss_intro;
          snprintf(message, sizeof(message), "WARNING - BOSS INCOMING");
        }
      } else if (queue_index < queue_count) {
        if (spawn_timer > 0) {
          spawn_timer--;
        }
        if (spawn_timer <= 0 && count_enemies(ei, max_enemies) < max_enemies) {
          int type = spawn_queue[queue_index++];
          float w = enemy_cfg[type][0];
          float h = enemy_cfg[type][1];
          float x = (rand() % 2 == 0)
                        ? static_cast<float>(camera_x) - w - 50.0f
                        : static_cast<float>(camera_x) + screen_w_f + 50.0f;
          if (x < 0) {
            x = static_cast<float>(camera_x) + screen_w_f + 50.0f;
          }
          if (x + w > world_w_f) {
            x = static_cast<float>(camera_x) - w - 50.0f;
          }
          x = clamp_float(x, 0, world_w_f - w);
          float y = find_ground_y(grid, level_w, level_h, x, h, cell);
          if (type == 6) {
            for (int row = level_h - 1; row >= 0; row--) {
              int col = static_cast<int>(x / cell_f);
              if (tile_at(grid, level_w, level_h, row, col) == 'F') {
                y = static_cast<float>(row * cell) - h;
                break;
              }
            }
          }
          spawn_enemy(ef, ei, max_enemies, enemy_types, enemy_cfg, type, x, y);
          spawn_timer = static_cast<int>(tuning[35]);
        }
      }

      // Periodic health pickup.
      if (stage > 0) {
        health_timer++;
        if (health_timer >= static_cast<int>(tuning[39])) {
          health_timer = 0;
          float x =
              static_cast<float>(camera_x + random_int(120, screen_w - 160));
          x = clamp_float(x, 20.0f, world_w_f - 60.0f);
          float y =
              find_ground_y(grid, level_w, level_h, x + 16.0f, 32.0f, cell);
          spawn_pickup(uf, ui, max_pickups, 1, x, y,
                       static_cast<int>(tuning[40]));
        }
      }

      // Smoothly keep the player centered horizontally.
      float target_cam = px + static_cast<float>(pw) / 2.0f - screen_w_f / 2.0f;
      camera_x = static_cast<int>(
          clamp_float(target_cam, 0.0f, world_w_f - screen_w_f));

      // Check stage exit.
      if (stage < 4 && state == 2) {
        int left_col = clamp_int(static_cast<int>(px / cell_f), 0, level_w - 1);
        int right_col = clamp_int(
            static_cast<int>((px + static_cast<float>(pw) - 1.0f) / cell_f), 0,
            level_w - 1);
        int top_row = clamp_int(static_cast<int>(py / cell_f), 0, level_h - 1);
        int bottom_row = clamp_int(
            static_cast<int>((py + static_cast<float>(ph) - 1.0f) / cell_f), 0,
            level_h - 1);
        bool found_exit = false;
        for (int row = top_row; row <= bottom_row; row++) {
          for (int col = left_col; col <= right_col; col++) {
            if (grid[row][col] == 'X') {
              found_exit = true;
            }
          }
        }
        if (found_exit) {
          audio.play(SfxClear, 1.0f, 85.0f);
          if (stage > 0) {
            score += level_points[stage];
          }
          float seconds =
              static_cast<float>(level_ticks) / static_cast<float>(tick_rate);
          if (stage == 1) {
            for (int i = 0; i < 3; i++) {
              if (seconds < static_cast<float>(l1_limit[i])) {
                score += l1_bonus[i];
                break;
              }
            }
          }
          state = 3;
          state_timer = 180;
          snprintf(message, sizeof(message), "LEVEL CLEAR");
          message_timer = 180;
        }
      }
    }

    // Keep menu and level background music in sync with state.
    if (state == 0) {
      if (level_music_ok && bgm_level.getStatus() == Music::Playing) {
        bgm_level.stop();
      }
      if (menu_music_ok && bgm_menu.getStatus() != Music::Playing) {
        bgm_menu.play();
      }
    } else {
      if (menu_music_ok && bgm_menu.getStatus() == Music::Playing) {
        bgm_menu.stop();
      }
      if (level_music_ok) {
        if (bgm_level.getStatus() != Music::Playing) {
          bgm_level.play();
        }
        bgm_level.setPitch(dilation_timer > 0 ? 0.8f : 1.0f);
      }
    }

    // ---------- Draw frame ----------
    window.clear(Color(8, 10, 20));

    if (state == 0 || grid == nullptr) {
      draw_picture(window, sprite, tex, 4, 0, 0, screen_w_f, screen_h_f, false,
                   Color::White);
      draw_text_center(window, "VICE: PROJECT DOOM", 100.0f, 6,
                       Color(255, 220, 80), screen_w);
      draw_text_center(window, "DETECTIVE QUINN HART VS THE GEL CARTEL", 170.0f,
                       2, Color(180, 230, 255), screen_w);
      draw_text_center(window, "LEFT / RIGHT  -  MOVE", 270.0f, 2, Color::White,
                       screen_w);
      draw_text_center(window, "SPACE  -  JUMP", 300.0f, 2, Color::White,
                       screen_w);
      draw_text_center(window, "DOWN  -  DUCK / CROUCH-RUN", 330.0f, 2,
                       Color::White, screen_w);
      draw_text_center(window, "UP  -  CLIMB / GRAB VINE", 360.0f, 2,
                       Color::White, screen_w);
      draw_text_center(window, "J  -  USE SELECTED WEAPON", 390.0f, 2,
                       Color::White, screen_w);
      draw_text_center(window, "Q  -  SWITCH WEAPON", 420.0f, 2, Color::White,
                       screen_w);
      draw_text_center(window, "PRESS SPACE TO START", 640.0f, 3,
                       Color(255, 160, 80), screen_w);
      window.display();
      sleep(milliseconds(1));
      continue;
    }

    draw_background(window, sprite, tex, camera_x, stage, screen_w, screen_h);
    draw_tiles(window, sprite, tex, grid, level_w, level_h, camera_x,
               electric_timer, stage, cell, screen_w);
    draw_pickups(window, sprite, tex, uf, ui, max_pickups, camera_x,
                 level_ticks);
    draw_enemies(window, sprite, tex, ef, ei, max_enemies, camera_x);
    draw_boss(window, sprite, tex, bossf, bossi, camera_x);
    draw_shots(window, sprite, tex, sf, si, max_shots, camera_x);
    draw_effects(window, sprite, tex, xf, xi, max_effects, camera_x);
    draw_player(window, sprite, tex, player, p, camera_x);

    char buffer[128];
    draw_bar(
        window, 20.0f, 20.0f, 280.0f, 22.0f, static_cast<float>(hp) / tuning[7],
        hp > 60 ? Color(60, 220, 100) : Color(240, 70, 70), Color(20, 20, 30));
    snprintf(buffer, sizeof(buffer), "HP %d/%d", hp,
             static_cast<int>(tuning[7]));
    draw_text(window, buffer, 24.0f, 46.0f, 2, Color::White);

    snprintf(buffer, sizeof(buffer), "SCORE %d", score);
    draw_text(window, buffer, screen_w_f - 300.0f, 24.0f, 2,
              Color(255, 230, 140));

    const char *weapon_name = "LASER WHIP";
    if (weapon == 1) {
      weapon_name = ".44 MAGNUM";
    }
    if (weapon == 2) {
      weapon_name = "GRENADE";
    }
    draw_text(window, weapon_name, 20.0f, 80.0f, 2, Color(120, 230, 255));
    snprintf(buffer, sizeof(buffer), "AMMO %d   GRENADES %d", ammo, grenades);
    draw_text(window, buffer, 20.0f, 104.0f, 2, Color::White);

    draw_text(window, stage_name(stage), 20.0f, screen_h_f - 40.0f, 1,
              Color(210, 210, 210));
    snprintf(buffer, sizeof(buffer), "TIME %d", level_ticks / 60);
    draw_text(window, buffer, screen_w_f - 180.0f, screen_h_f - 40.0f, 1,
              Color(210, 210, 210));

    if (shield_timer > 0) {
      draw_text(window, "SHIELD", 20.0f, 132.0f, 1, Color(120, 200, 255));
    }
    if (dilation_timer > 0) {
      draw_text(window, "TIME DILATION", 120.0f, 132.0f, 1,
                Color(180, 120, 255));
    }
    if (boost_timer > 0) {
      draw_text(window, "DAMAGE BOOST", 280.0f, 132.0f, 1, Color(255, 140, 80));
    }
    if (double_jump) {
      draw_text(window, "DOUBLE JUMP", 440.0f, 132.0f, 1, Color(120, 255, 180));
    }

    if (boss_phase > 0 && boss_intro == 0) {
      draw_bar(window, 318.0f, 20.0f, 500.0f, 18.0f,
               static_cast<float>(boss_hp) / static_cast<float>(boss_max_hp),
               Color(230, 50, 80), Color(30, 10, 20));
      draw_text_center(window, "BOSS", 44.0f, 1, Color(255, 180, 180),
                       screen_w);
    }

    if (stage == 0 && state == 2) {
      draw_text_center(window, "LEFT / RIGHT  -  MOVE", 280.0f, 2,
                       Color(190, 235, 255), screen_w);
      draw_text_center(window, "SPACE  -  JUMP", 316.0f, 2,
                       Color(190, 235, 255), screen_w);
      draw_text_center(window, "DOWN  -  DUCK AND CROUCH-RUN", 352.0f, 2,
                       Color(190, 235, 255), screen_w);
      draw_text_center(window, "UP / DOWN  -  CLIMB THE LADDER", 388.0f, 2,
                       Color(190, 235, 255), screen_w);
      draw_text_center(window, "J  -  USE WEAPON    Q  -  SWITCH", 424.0f, 2,
                       Color(190, 235, 255), screen_w);
      draw_text_center(window, "CLIMB THE LADDER AND TAKE THE EXIT", 500.0f, 2,
                       Color(255, 220, 120), screen_w);
    }

    if (message_timer > 0) {
      draw_text_center(window, message, 170.0f, 3, Color(255, 240, 180),
                       screen_w);
    }

    if (state == 1 || state == 3 || state == 4 || state == 5) {
      RectangleShape dark(Vector2f(screen_w_f, screen_h_f));
      dark.setFillColor(Color(0, 0, 0, 185));
      window.draw(dark);
      if (state == 1) {
        draw_text_center(window, stage_name(stage), 270.0f, 4,
                         Color(255, 220, 80), screen_w);
        draw_text_center(window, stage_objective(stage), 340.0f, 2,
                         Color::White, screen_w);
        draw_text_center(window, "GET READY", 430.0f, 2, Color(120, 230, 255),
                         screen_w);
      } else if (state == 3) {
        draw_text_center(window, "LEVEL CLEAR", 300.0f, 5, Color(255, 220, 80),
                         screen_w);
        snprintf(buffer, sizeof(buffer), "SCORE %d", score);
        draw_text_center(window, buffer, 390.0f, 3, Color::White, screen_w);
      } else if (state == 4) {
        draw_text_center(window, "YOU DIED", 300.0f, 5, Color(240, 60, 60),
                         screen_w);
        draw_text_center(window, "PRESS SPACE TO RETRY", 400.0f, 2,
                         Color::White, screen_w);
      } else {
        draw_text_center(window, "VICTORY", 240.0f, 6, Color(255, 220, 80),
                         screen_w);
        snprintf(buffer, sizeof(buffer), "FINAL SCORE %d", score);
        draw_text_center(window, buffer, 340.0f, 3, Color::White, screen_w);
        draw_text_center(window, "PRESS SPACE FOR MENU", 440.0f, 2,
                         Color(180, 230, 255), screen_w);
      }
    }

    if (damage_flash > 0) {
      RectangleShape flash(Vector2f(screen_w_f, screen_h_f));
      flash.setFillColor(Color(220, 20, 20, 45));
      window.draw(flash);
    }

    window.display();
    sleep(milliseconds(1));
  }

  free_level(grid, level_h, tile_timer, tile_state);
  return 0;
}

// ===========================================================================
// Headless self-test
// ===========================================================================

int run_self_test(float *tuning, float enemy_cfg[13][14], int spawn_types[5][5],
                  int spawn_counts[5][5], int endless_types[5],
                  int level_points[5], int l1_limit[3], int l1_bonus[3],
                  int lx_limit[3], int lx_bonus[3], int boss_limit[3],
                  int boss_bonus[3], int boss_quota_type[5],
                  int boss_quota_count[5]) {
  (void)spawn_types;
  (void)spawn_counts;
  (void)endless_types;
  (void)level_points;
  (void)l1_limit;
  (void)l1_bonus;
  (void)lx_limit;
  (void)lx_bonus;
  (void)boss_limit;
  (void)boss_bonus;
  (void)boss_quota_type;
  (void)boss_quota_count;

  int failures = 0;
  int cell = static_cast<int>(tuning[2]);
  int enemy_types = static_cast<int>(tuning[29]);
  int max_enemies = static_cast<int>(tuning[30]);
  int max_shots = static_cast<int>(tuning[31]);
  int max_effects = 128;

  char **grid = nullptr;
  int level_w = 0;
  int level_h = 0;
  int *tile_timer = nullptr;
  int *tile_state = nullptr;
  float start_x = 0;
  float start_y = 0;
  int exit_col = -1;
  int exit_row = -1;

  float player[8] = {};
  int p[32] = {};
  p[8] = 100;
  p[9] = static_cast<int>(tuning[7]);
  p[0] = static_cast<int>(tuning[8]);
  p[1] = static_cast<int>(tuning[9]);
  p[2] = 1;
  p[18] = static_cast<int>(tuning[21]);
  p[19] = static_cast<int>(tuning[24]);

  float ef[64][8] = {};
  int ei[64][9] = {};
  float sf[128][4] = {};
  int si[128][4] = {};
  float xf[128][4] = {};
  int xi[128][4] = {};
  float bossf[6] = {};
  int bossi[8] = {};
  int game[12] = {};
  game[0] = 2;

  int expected_width[5] = {48, 64, 64, 64, 64};
  const char *paths[5] = {"levels/tutorial.txt", "levels/level1.txt",
                          "levels/level2.txt", "levels/level3.txt",
                          "levels/levelx.txt"};

  for (int stage = 0; stage < 5; stage++) {
    free_level(grid, level_h, tile_timer, tile_state);
    grid = load_level(paths[stage], level_w, level_h, start_x, start_y, cell,
                      static_cast<int>(tuning[9]), exit_col, exit_row,
                      tile_timer, tile_state);
    if (grid == nullptr || level_h != 14 || level_w != expected_width[stage]) {
      printf("FAIL stage %d did not load correctly\n", stage);
      failures++;
    } else {
      printf("PASS stage %d loaded: %dx%d, exit %d,%d\n", stage, level_w,
             level_h, exit_col, exit_row);
    }
    if (stage < 4 && (exit_col < 0 || exit_row < 0)) {
      printf("FAIL stage %d has no exit\n", stage);
      failures++;
    }
  }

  free_level(grid, level_h, tile_timer, tile_state);
  grid = load_level("levels/level1.txt", level_w, level_h, start_x, start_y,
                    cell, static_cast<int>(tuning[9]), exit_col, exit_row,
                    tile_timer, tile_state);
  player[0] = start_x;
  player[1] = start_y;
  float old_x = player[0];
  for (int i = 0; i < 90; i++) {
    update_player(player, p, grid, level_w, level_h, 0, tuning, false, true,
                  false, false, false, nullptr);
  }
  if (player[0] <= old_x + 30.0f) {
    printf("FAIL player did not move right\n");
    failures++;
  } else {
    printf("PASS player movement\n");
  }

  player[0] = static_cast<float>(2 * cell);
  player[1] = static_cast<float>((level_h - 1) * cell - p[1]);
  spawn_enemy(ef, ei, max_enemies, enemy_types, enemy_cfg, 0, player[0] + 70.0f,
              player[1]);
  ei[0][EnemyHp] = 1000;
  int before = ei[0][EnemyHp];
  try_whip(player, p, grid, level_w, level_h, tile_timer, tile_state, tuning,
           ef, ei, bossf, bossi, nullptr);
  if (ei[0][EnemyHp] >= before) {
    printf("FAIL laser whip did not damage a crawler\n");
    failures++;
  } else {
    printf("PASS laser whip\n");
  }

  p[17] = 1;
  p[18] = 10;
  p[21] = 0;
  handle_weapons(player, p, grid, level_w, level_h, tile_timer, tile_state,
                 tuning, ef, ei, sf, si, max_shots, bossf, bossi, true, false,
                 nullptr);
  if (p[18] != 9 || !si[0][ShotActive]) {
    printf("FAIL magnum did not use ammo\n");
    failures++;
  } else {
    printf("PASS magnum ammo\n");
  }

  for (int i = 0; i < 64; i++) {
    ei[i][EnemyActive] = 0;
  }
  for (int type = 0; type < enemy_types; type++) {
    float y = static_cast<float>((level_h - 1) * cell) - enemy_cfg[type][1];
    spawn_enemy(ef, ei, max_enemies, enemy_types, enemy_cfg, type,
                static_cast<float>(5 * cell + type * 50), y);
  }
  for (int i = 0; i < 120; i++) {
    update_enemies(ef, ei, max_enemies, enemy_types, enemy_cfg, tuning, grid,
                   level_w, level_h, player, p, sf, si, max_shots, game,
                   nullptr);
    update_shots(sf, si, max_shots, tuning, grid, level_w, level_h, player, p,
                 ef, ei, xf, xi, max_effects, enemy_types, enemy_cfg, game,
                 bossf, bossi, nullptr);
    update_effects(xf, xi, max_effects);
  }
  printf("PASS all %d enemy types updated\n", enemy_types);

  begin_boss_phase(1, bossf, bossi, tuning, level_w, level_h, nullptr);
  bossi[7] = 0;
  bossi[1] -= 10000;
  if (bossi[1] > 0) {
    printf("FAIL boss damage\n");
    failures++;
  }
  begin_boss_phase(2, bossf, bossi, tuning, level_w, level_h, nullptr);
  begin_boss_phase(3, bossf, bossi, tuning, level_w, level_h, nullptr);
  if (bossi[0] != 3) {
    printf("FAIL boss phase setup\n");
    failures++;
  } else {
    printf("PASS boss phases\n");
  }

  int kill_time[13][32];
  int kill_slot[13] = {};
  for (int i = 0; i < 13; i++) {
    for (int j = 0; j < 32; j++) {
      kill_time[i][j] = -100000;
    }
  }
  int first = award_kill(0, 50, 500, kill_time, kill_slot, false);
  int second = award_kill(0, 50, 500, kill_time, kill_slot, false);
  int third = award_kill(0, 50, 500, kill_time, kill_slot, false);
  if (first != 50 || second != 250 || third != 550) {
    printf("FAIL multi-kill scoring: %d %d %d\n", first, second, third);
    failures++;
  } else {
    printf("PASS multi-kill scoring\n");
  }

  for (int i = 0; i < 13; i++) {
    for (int j = 0; j < 32; j++) {
      kill_time[i][j] = -100000;
    }
    kill_slot[i] = 0;
  }
  int air = award_kill(0, 50, 500, kill_time, kill_slot, true);
  if (air != 400) {
    printf("FAIL aerial defeat bonus: %d\n", air);
    failures++;
  } else {
    printf("PASS aerial defeat bonus\n");
  }

  if (death_penalty(1000) != 800 || death_penalty(100) != 0) {
    printf("FAIL death penalty\n");
    failures++;
  } else {
    printf("PASS death penalty\n");
  }

  free_level(grid, level_h, tile_timer, tile_state);

  if (failures == 0) {
    printf("SELFTEST PASS\n");
    return 0;
  }
  printf("SELFTEST FAILED: %d problem(s)\n", failures);
  return 1;
}
