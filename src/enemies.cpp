#include "enemies.h"
#include "config.h"
#include <SFML/Graphics.hpp>

using namespace sf;

// these are the helper functions, they are made static so they will remain
// private to this file

static int enemy_width(int type) {
  switch (type) {
  case ENEMY_CRAWLER:
    return CRAWLER_WIDTH;
  default:
    return 1;
  }
}

static int enemy_height(int type) {
  switch (type) {
  case ENEMY_CRAWLER:
    return CRAWLER_HEIGHT;
  default:
    return 1;
  }
}

static int enemy_start_hp(int type) {
  switch (type) {
  case ENEMY_CRAWLER:
    return CRAWLER_HP;
  default:
    return 1;
  }
}

static float enemy_start_speed(int type) {
  switch (type) {
  case ENEMY_CRAWLER:
    return CRAWLER_SPEED;
  default:
    return 1;
  }
}

static int enemy_damage(int type) {
  switch (type) {
  case ENEMY_CRAWLER:
    return CRAWLER_DAMAGE;
  default:
    return 1;
  }
}

static char enemy_tile_at(char **lvl, int row, int col, const int height,
                          const int width) {
  if (row < 0)
    return '.';
  else if (row >= height || col < 0 || col >= width)
    return '#';
  else
    return lvl[row][col];
}

static void update_crawler(int i, float *x, float *y, float *velocity_x,
                           float *velocity_y, int *direction, char **lvl,
                           const int height, const int width,
                           const int cell_size) {
  const int w = CRAWLER_WIDTH;
  const int h = CRAWLER_HEIGHT;

  // gravity pulls it down (was added to velocity_x before)
  velocity_y[i] += ENEMY_GRAVITY;

  if (velocity_y[i] > ENEMY_TERMINAL_VELOCITY) {
    velocity_y[i] = ENEMY_TERMINAL_VELOCITY;
  }

  float next_y = y[i] + velocity_y[i];
  int feet_row = (int)(next_y + h) / cell_size;
  int left_col = (int)x[i] / cell_size;
  int right_col = (int)(x[i] + w - 1) / cell_size;

  bool landed = false;

  // checks both bottom corners (only the left one was checked before)
  char left_tile = enemy_tile_at(lvl, feet_row, left_col, height, width);
  char right_tile = enemy_tile_at(lvl, feet_row, right_col, height, width);

  if (left_tile == '#' || right_tile == '#')
    landed = true;

  if (landed) {
    y[i] = feet_row * cell_size - h;
    velocity_y[i] = 0;
  } else {
    y[i] = next_y;
    return;
  }

  float next_x = x[i] + velocity_x[i];

  int front_col;

  if (velocity_x[i] > 0)
    front_col = (int)(next_x + w - 1) / cell_size;
  else
    front_col = (int)next_x / cell_size;

  int body_row = (int)(y[i] + h - 1) / cell_size;
  int below_row = (int)(y[i] + h) / cell_size;

  bool out_of_screen = (next_x < 0 || ((next_x + w) > (width * cell_size)));
  // wall checks the row of the body, ledge checks the row under the feet
  bool wall = (enemy_tile_at(lvl, body_row, front_col, height, width) == '#');
  bool ledge = (enemy_tile_at(lvl, below_row, front_col, height, width) != '#');

  if (out_of_screen || wall || ledge) {
    velocity_x[i] = -velocity_x[i];
    direction[i] = -direction[i];
  } else
    x[i] = next_x;
}

// these are the core functions for the enemies
void init_enemies(bool *active, int *type, float *x, float *y,
                  float *velocity_x, float *velocity_y, int *hp, int *direction,
                  int *state, int *timer) {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    active[i] = false;
    type[i] = 0;
    x[i] = 0;
    y[i] = 0;
    velocity_x[i] = 0;
    velocity_y[i] = 0;
    hp[i] = 0;
    direction[i] = 0;
    state[i] = 0;
    timer[i] = 0;
  }
}

// it spawns the enemy and assigns it to a slot and returns the slot number
int spawn_enemy(bool *active, int *type, float *x, float *y, float *velocity_x,
                float *velocity_y, int *hp, int *direction, int *state,
                int *timer, int newType, float new_x, float new_y) {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!active[i]) {
      active[i] = true;
      type[i] = newType;
      x[i] = new_x;
      y[i] = new_y;
      direction[i] = -1;
      velocity_x[i] = direction[i] * enemy_start_speed(newType);
      velocity_y[i] = 0;
      hp[i] = enemy_start_hp(newType);
      state[i] = 0;
      timer[i] = 0;
      return i;
    }
  }

  return -1;
}

void update_enemies(bool *active, int *type, float *x, float *y,
                    float *velocity_x, float *velocity_y, int *hp,
                    int *direction, int *state, int *timer, char **lvl,
                    const int height, const int width, const int cell_size) {
  // hp, state and timer will be used by the enemy types that come next
  (void)hp;
  (void)state;
  (void)timer;

  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!active[i])
      continue;

    switch (type[i]) {
    case ENEMY_CRAWLER:
      update_crawler(i, x, y, velocity_x, velocity_y, direction, lvl, height,
                     width, cell_size);
      break;
    default:
      break; // was "return", which skipped all the enemies after this one
    }
  }
}

void draw_enemies(RenderWindow &window, Sprite &enemySprite, bool *active,
                  int *type, float *x, float *y) {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!active[i])
      continue;

    enemySprite.setScale(enemy_width(type[i]), enemy_height(type[i]));
    enemySprite.setPosition(x[i], y[i]);

    switch (type[i]) {
    case ENEMY_CRAWLER:
      enemySprite.setColor(Color(200, 40, 40));
      break;
    default:
      enemySprite.setColor(Color::Magenta);
    }

    window.draw(enemySprite);
  }
}

// this reduces the hp of the enemy and tells if it was killed by a hit or not
bool damage_enemy(bool *active, int *hp, int index, int amount) {
  if (index < 0 || index >= MAX_ENEMIES || !active[index])
    return false;

  hp[index] -= amount;

  if (hp[index] <= 0) {
    active[index] = false;
    return true;
  }

  return false;
}

int enemy_damage_on_contact(bool *active, int *type, float *x, float *y,
                            float player_x, float player_y, float player_w,
                            float player_h) {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!active[i])
      continue;

    float enemy_w = enemy_width(type[i]);
    float enemy_h = enemy_height(type[i]);

    bool overlap =
        ((x[i] < player_x + player_w) && (x[i] + enemy_w > player_x) &&
         (y[i] < player_y + player_h) && (y[i] + enemy_h > player_y));

    if (overlap)
      return enemy_damage(type[i]);
  }

  return 0;
}

