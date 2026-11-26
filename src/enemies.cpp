#include "enemies.h"
#include "config.h"
#include <SFML/Graphics.hpp>
#include <cmath>

using namespace sf;

// these are the helper functions, they are made static so they will remain
// private to this file and cannot be used outside of this file, please dont use
// them outside otherwise big errors
static int enemy_width(int type) {
  switch (type) {
  case ENEMY_CRAWLER:
    return CRAWLER_WIDTH;
  case ENEMY_JUMPER:
    return JUMPER_WIDTH;
  case ENEMY_INSECT:
    return INSECT_WIDTH;
  case ENEMY_LEOPARD:
    return LEOPARD_WIDTH;
  case ENEMY_SHOOTER:
    return SHOOTER_WIDTH;
  case ENEMY_TRIBAL:
    return TRIBAL_WIDTH;
  case ENEMY_FISH:
    return FISH_WIDTH;
  default:
    return 1;
  }
}

static int enemy_height(int type) {
  switch (type) {
  case ENEMY_CRAWLER:
    return CRAWLER_HEIGHT;
  case ENEMY_JUMPER:
    return JUMPER_HEIGHT;
  case ENEMY_INSECT:
    return INSECT_HEIGHT;
  case ENEMY_LEOPARD:
    return LEOPARD_HEIGHT;
  case ENEMY_SHOOTER:
    return SHOOTER_HEIGHT;
  case ENEMY_TRIBAL:
    return TRIBAL_HEIGHT;
  case ENEMY_FISH:
    return FISH_HEIGHT;
  default:
    return 1;
  }
}

static int enemy_start_hp(int type) {
  switch (type) {
  case ENEMY_CRAWLER:
    return CRAWLER_HP;
  case ENEMY_JUMPER:
    return JUMPER_HP;
  case ENEMY_INSECT:
    return INSECT_HP;
  case ENEMY_LEOPARD:
    return LEOPARD_HP;
  case ENEMY_SHOOTER:
    return SHOOTER_HP;
  case ENEMY_TRIBAL:
    return TRIBAL_HP;
  case ENEMY_FISH:
    return FISH_HP;
  default:
    return 1;
  }
}

// only the walking enemies start with a sideways speed, the others set their
// own speed while they update
static float enemy_start_speed(int type) {
  switch (type) {
  case ENEMY_CRAWLER:
    return CRAWLER_SPEED;
  case ENEMY_LEOPARD:
    return LEOPARD_PATROL_SPEED;
  default:
    return 0;
  }
}

static int enemy_damage(int type) {
  switch (type) {
  case ENEMY_CRAWLER:
    return CRAWLER_DAMAGE;
  case ENEMY_JUMPER:
    return JUMPER_DAMAGE;
  case ENEMY_INSECT:
    return INSECT_DAMAGE;
  case ENEMY_LEOPARD:
    return LEOPARD_DAMAGE;
  case ENEMY_SHOOTER:
    return SHOOTER_DAMAGE;
  case ENEMY_TRIBAL:
    return TRIBAL_DAMAGE;
  case ENEMY_FISH:
    return FISH_DAMAGE;
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

// -1 if the target is on the left, 1 if it is on the right
static int direction_to(float from_x, float to_x) {
  if (to_x < from_x)
    return -1;
  return 1;
}

// pulls the enemy down and lands it on solid blocks, also stops it when it
// hits a ceiling. returns true if the enemy is standing on the ground
static bool apply_gravity(int i, float *x, float *y, float *velocity_y, int w,
                          int h, char **lvl, const int height, const int width,
                          const int cell_size) {
  velocity_y[i] += ENEMY_GRAVITY;
  if (velocity_y[i] > ENEMY_TERMINAL_VELOCITY) {
    velocity_y[i] = ENEMY_TERMINAL_VELOCITY;
  }

  float next_y = y[i] + velocity_y[i];
  int left_col = (int)x[i] / cell_size;
  int right_col = (int)(x[i] + w - 1) / cell_size;

  if (velocity_y[i] >= 0) {
    // falling or standing, check under the feet
    int feet_row = (int)(next_y + h) / cell_size;
    char left_tile = enemy_tile_at(lvl, feet_row, left_col, height, width);
    char right_tile = enemy_tile_at(lvl, feet_row, right_col, height, width);

    if (left_tile == '#' || right_tile == '#') {
      y[i] = feet_row * cell_size - h;
      velocity_y[i] = 0;
      return true;
    }
  } else {
    // going up, check above the head
    int head_row = -1;
    if (next_y >= 0)
      head_row = (int)next_y / cell_size;

    char left_tile = enemy_tile_at(lvl, head_row, left_col, height, width);
    char right_tile = enemy_tile_at(lvl, head_row, right_col, height, width);

    if (left_tile == '#' || right_tile == '#') {
      y[i] = (head_row + 1) * cell_size;
      velocity_y[i] = 0;
      return false;
    }
  }

  y[i] = next_y;
  return false;
}

// the column of blocks in front of the enemy
static int front_column(float next_x, bool moving_right, int w,
                        const int cell_size) {
  if (moving_right)
    return (int)(next_x + w - 1) / cell_size;
  return (int)next_x / cell_size;
}

// true if there is a solid block (or the screen edge) in front of the body
static bool wall_ahead(float next_x, float y, bool moving_right, int w, int h,
                       char **lvl, const int height, const int width,
                       const int cell_size) {
  if (next_x < 0 || next_x + w > width * cell_size)
    return true;

  int col = front_column(next_x, moving_right, w, cell_size);
  int body_row = (int)(y + h - 1) / cell_size;

  return enemy_tile_at(lvl, body_row, col, height, width) == '#';
}

// true if there is no floor in front of the feet
static bool ledge_ahead(float next_x, float y, bool moving_right, int w, int h,
                        char **lvl, const int height, const int width,
                        const int cell_size) {
  int col = front_column(next_x, moving_right, w, cell_size);
  int below_row = (int)(y + h) / cell_size;

  return enemy_tile_at(lvl, below_row, col, height, width) != '#';
}

// moves the enemy sideways by its velocity. returns false (and does not move)
// if a wall, ledge or the screen edge is in the way
static bool try_walk(int i, float *x, float *y, float *velocity_x, int w, int h,
                     char **lvl, const int height, const int width,
                     const int cell_size) {
  float next_x = x[i] + velocity_x[i];
  bool moving_right = velocity_x[i] > 0;

  if (wall_ahead(next_x, y[i], moving_right, w, h, lvl, height, width,
                 cell_size))
    return false;
  if (ledge_ahead(next_x, y[i], moving_right, w, h, lvl, height, width,
                  cell_size))
    return false;

  x[i] = next_x;
  return true;
}

// ---------- shooter bullets ----------

void init_enemy_projectiles(bool *bullet_active, float *bullet_x,
                            float *bullet_y, float *bullet_velocity_x) {
  for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
    bullet_active[i] = false;
    bullet_x[i] = 0;
    bullet_y[i] = 0;
    bullet_velocity_x[i] = 0;
  }
}

static void spawn_enemy_projectile(bool *bullet_active, float *bullet_x,
                                   float *bullet_y, float *bullet_velocity_x,
                                   float new_x, float new_y,
                                   float new_velocity_x) {
  for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
    if (!bullet_active[i]) {
      bullet_active[i] = true;
      bullet_x[i] = new_x;
      bullet_y[i] = new_y;
      bullet_velocity_x[i] = new_velocity_x;
      return;
    }
  }
}

void update_enemy_projectiles(bool *bullet_active, float *bullet_x,
                              float *bullet_y, float *bullet_velocity_x,
                              char **lvl, const int height, const int width,
                              const int cell_size) {
  for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
    if (!bullet_active[i])
      continue;

    bullet_x[i] += bullet_velocity_x[i];

    if (bullet_x[i] < 0 ||
        bullet_x[i] + ENEMY_BULLET_WIDTH > width * cell_size) {
      bullet_active[i] = false;
      continue;
    }

    int col = (int)(bullet_x[i] + ENEMY_BULLET_WIDTH / 2) / cell_size;
    int row = (int)(bullet_y[i] + ENEMY_BULLET_HEIGHT / 2) / cell_size;

    if (enemy_tile_at(lvl, row, col, height, width) == '#')
      bullet_active[i] = false;
  }
}

// draws every live bullet, stretched so the picture matches the bullet's
// hitbox. bullets flying left get flipped so they face the right way
void draw_enemy_projectiles(RenderWindow &window, Sprite &enemySprite,
                            Texture &bullet_texture, bool *bullet_active,
                            float *bullet_x, float *bullet_y,
                            float *bullet_velocity_x) {
  // the true flag resets the sprite's crop to the new texture's full size,
  // without it the sprite keeps the old texture's size and the picture gets cut
  enemySprite.setTexture(bullet_texture, true);

  float scale_x = (float)ENEMY_BULLET_WIDTH / bullet_texture.getSize().x;
  float scale_y = (float)ENEMY_BULLET_HEIGHT / bullet_texture.getSize().y;

  for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
    if (!bullet_active[i])
      continue;

    if (bullet_velocity_x[i] < 0) {
      // a negative scale mirrors the sprite around its left edge, so we shift
      // it right by the width to keep it sitting on the same spot
      enemySprite.setScale(-scale_x, scale_y);
      enemySprite.setPosition(bullet_x[i] + ENEMY_BULLET_WIDTH, bullet_y[i]);
    } else {
      enemySprite.setScale(scale_x, scale_y);
      enemySprite.setPosition(bullet_x[i], bullet_y[i]);
    }

    window.draw(enemySprite);
  }
}

// returns the bullet damage if a bullet touches the player, and removes it
int enemy_projectile_damage_on_contact(bool *bullet_active, float *bullet_x,
                                       float *bullet_y, float player_x,
                                       float player_y, float player_w,
                                       float player_h) {
  for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
    if (!bullet_active[i])
      continue;

    bool overlap = ((bullet_x[i] < player_x + player_w) &&
                    (bullet_x[i] + ENEMY_BULLET_WIDTH > player_x) &&
                    (bullet_y[i] < player_y + player_h) &&
                    (bullet_y[i] + ENEMY_BULLET_HEIGHT > player_y));

    if (overlap) {
      bullet_active[i] = false;
      return ENEMY_BULLET_DAMAGE;
    }
  }

  return 0;
}

// ---------- Spiked Crawler ----------
// walks back and forth, turns at walls and ledges

static void update_crawler(int i, float *x, float *y, float *velocity_x,
                           float *velocity_y, int *direction, char **lvl,
                           const int height, const int width,
                           const int cell_size) {
  const int w = CRAWLER_WIDTH;
  const int h = CRAWLER_HEIGHT;

  if (!apply_gravity(i, x, y, velocity_y, w, h, lvl, height, width, cell_size))
    return;

  if (!try_walk(i, x, y, velocity_x, w, h, lvl, height, width, cell_size)) {
    velocity_x[i] = -velocity_x[i];
    direction[i] = -direction[i];
  }
}

// ---------- Blue Jumping Enemy ----------
// waits on the ground, then jumps toward the player

static void update_jumper(int i, float *x, float *y, float *velocity_x,
                          float *velocity_y, int *direction, int *state,
                          int *timer, float target_x, char **lvl,
                          const int height, const int width,
                          const int cell_size) {
  const int w = JUMPER_WIDTH;
  const int h = JUMPER_HEIGHT;

  bool landed =
      apply_gravity(i, x, y, velocity_y, w, h, lvl, height, width, cell_size);

  if (landed) {
    if (state[i] == JUMPER_STATE_AIR) {
      // just came down
      state[i] = JUMPER_STATE_GROUND;
      timer[i] = JUMPER_JUMP_DELAY;
      velocity_x[i] = 0;
    }

    float center_x = x[i] + w / 2.0f;
    direction[i] = direction_to(center_x, target_x);

    if (timer[i] > 0) {
      timer[i]--;
    } else if (fabs(target_x - center_x) < JUMPER_SIGHT_RANGE) {
      velocity_x[i] = direction[i] * JUMPER_SPEED;
      velocity_y[i] = -JUMPER_JUMP_SPEED;
      state[i] = JUMPER_STATE_AIR;
    }
  } else {
    // in the air, keep moving sideways unless a wall is in the way
    float next_x = x[i] + velocity_x[i];
    bool moving_right = velocity_x[i] > 0;

    if (wall_ahead(next_x, y[i], moving_right, w, h, lvl, height, width,
                   cell_size))
      velocity_x[i] = 0;
    else
      x[i] = next_x;
  }
}

// ---------- Flying Insect ----------
// flies toward the player, bobbing up and down

static void update_insect(int i, float *x, float *y, float *velocity_x,
                          int *direction, int *timer, float target_x,
                          float target_y, const int height, const int width,
                          const int cell_size) {
  const int w = INSECT_WIDTH;
  const int h = INSECT_HEIGHT;

  timer[i]++;

  float center_x = x[i] + w / 2.0f;
  float center_y = y[i] + h / 2.0f;

  direction[i] = direction_to(center_x, target_x);

  // stop sideways movement when it is right above or below the player
  if (fabs(target_x - center_x) < INSECT_SPEED)
    velocity_x[i] = 0;
  else
    velocity_x[i] = direction[i] * INSECT_SPEED;

  x[i] += velocity_x[i];

  float drift = 0;
  if (target_y > center_y + INSECT_VERTICAL_SPEED)
    drift = INSECT_VERTICAL_SPEED;
  else if (target_y < center_y - INSECT_VERTICAL_SPEED)
    drift = -INSECT_VERTICAL_SPEED;

  y[i] += drift + sin(timer[i] * INSECT_WAVE_SPEED) * INSECT_WAVE_SIZE;

  // keep it inside the screen
  if (x[i] < 0)
    x[i] = 0;
  if (x[i] + w > width * cell_size)
    x[i] = width * cell_size - w;
  if (y[i] < 0)
    y[i] = 0;
  if (y[i] + h > height * cell_size)
    y[i] = height * cell_size - h;
}

// ---------- Leopard ----------
// patrols, charges when it sees the player, then rests

static void update_leopard(int i, float *x, float *y, float *velocity_x,
                           float *velocity_y, int *direction, int *state,
                           int *timer, float target_x, float target_y,
                           char **lvl, const int height, const int width,
                           const int cell_size) {
  const int w = LEOPARD_WIDTH;
  const int h = LEOPARD_HEIGHT;

  if (!apply_gravity(i, x, y, velocity_y, w, h, lvl, height, width, cell_size))
    return;

  float center_x = x[i] + w / 2.0f;
  float center_y = y[i] + h / 2.0f;

  if (state[i] == LEOPARD_STATE_PATROL) {
    if (!try_walk(i, x, y, velocity_x, w, h, lvl, height, width, cell_size)) {
      velocity_x[i] = -velocity_x[i];
      direction[i] = -direction[i];
    }

    bool sees_player = (fabs(target_x - center_x) < LEOPARD_SIGHT_RANGE &&
                        fabs(target_y - center_y) < LEOPARD_SIGHT_HEIGHT);

    if (sees_player) {
      direction[i] = direction_to(center_x, target_x);
      velocity_x[i] = direction[i] * LEOPARD_CHARGE_SPEED;
      state[i] = LEOPARD_STATE_CHARGE;
      timer[i] = LEOPARD_CHARGE_FRAMES;
    }
  } else if (state[i] == LEOPARD_STATE_CHARGE) {
    bool moved =
        try_walk(i, x, y, velocity_x, w, h, lvl, height, width, cell_size);
    timer[i]--;

    if (!moved || timer[i] <= 0) {
      velocity_x[i] = 0;
      state[i] = LEOPARD_STATE_RECOVER;
      timer[i] = LEOPARD_RECOVER_FRAMES;
    }
  } else {
    timer[i]--;

    if (timer[i] <= 0) {
      state[i] = LEOPARD_STATE_PATROL;
      velocity_x[i] = direction[i] * LEOPARD_PATROL_SPEED;
    }
  }
}

// ---------- Green Shooter ----------
// stands still, faces the player and shoots when the player is in sight

static void update_shooter(int i, float *x, float *y, float *velocity_y,
                           int *direction, int *timer, bool *bullet_active,
                           float *bullet_x, float *bullet_y,
                           float *bullet_velocity_x, float target_x,
                           float target_y, char **lvl, const int height,
                           const int width, const int cell_size) {
  const int w = SHOOTER_WIDTH;
  const int h = SHOOTER_HEIGHT;

  if (!apply_gravity(i, x, y, velocity_y, w, h, lvl, height, width, cell_size))
    return;

  float center_x = x[i] + w / 2.0f;
  float center_y = y[i] + h / 2.0f;

  direction[i] = direction_to(center_x, target_x);

  if (timer[i] > 0)
    timer[i]--;

  bool sees_player = (fabs(target_x - center_x) < SHOOTER_SIGHT_RANGE &&
                      fabs(target_y - center_y) < SHOOTER_SIGHT_HEIGHT);

  if (sees_player && timer[i] == 0) {
    float start_x = x[i] + w;
    if (direction[i] < 0)
      start_x = x[i] - ENEMY_BULLET_WIDTH;

    spawn_enemy_projectile(bullet_active, bullet_x, bullet_y, bullet_velocity_x,
                           start_x, y[i] + h / 3.0f,
                           direction[i] * ENEMY_BULLET_SPEED);
    timer[i] = SHOOTER_SHOOT_DELAY;
  }
}

// ---------- Tribal Warrior ----------
// walks toward the player, winds up, lunges, then rests

static void update_tribal(int i, float *x, float *y, float *velocity_x,
                          float *velocity_y, int *direction, int *state,
                          int *timer, float target_x, float target_y,
                          char **lvl, const int height, const int width,
                          const int cell_size) {
  const int w = TRIBAL_WIDTH;
  const int h = TRIBAL_HEIGHT;

  if (!apply_gravity(i, x, y, velocity_y, w, h, lvl, height, width, cell_size))
    return;

  float center_x = x[i] + w / 2.0f;
  float center_y = y[i] + h / 2.0f;

  if (state[i] == TRIBAL_STATE_WALK) {
    float distance = fabs(target_x - center_x);
    bool sees_player = (distance < TRIBAL_SIGHT_RANGE &&
                        fabs(target_y - center_y) < TRIBAL_SIGHT_HEIGHT);

    if (!sees_player) {
      velocity_x[i] = 0;
      return;
    }

    direction[i] = direction_to(center_x, target_x);

    if (distance < TRIBAL_ATTACK_RANGE) {
      velocity_x[i] = 0;
      state[i] = TRIBAL_STATE_WINDUP;
      timer[i] = TRIBAL_WINDUP_FRAMES;
    } else {
      velocity_x[i] = direction[i] * TRIBAL_SPEED;
      try_walk(i, x, y, velocity_x, w, h, lvl, height, width, cell_size);
    }
  } else if (state[i] == TRIBAL_STATE_WINDUP) {
    timer[i]--;

    if (timer[i] <= 0) {
      velocity_x[i] = direction[i] * TRIBAL_LUNGE_SPEED;
      state[i] = TRIBAL_STATE_LUNGE;
      timer[i] = TRIBAL_LUNGE_FRAMES;
    }
  } else if (state[i] == TRIBAL_STATE_LUNGE) {
    bool moved =
        try_walk(i, x, y, velocity_x, w, h, lvl, height, width, cell_size);
    timer[i]--;

    if (!moved || timer[i] <= 0) {
      velocity_x[i] = 0;
      state[i] = TRIBAL_STATE_COOLDOWN;
      timer[i] = TRIBAL_COOLDOWN_FRAMES;
    }
  } else {
    timer[i]--;

    if (timer[i] <= 0)
      state[i] = TRIBAL_STATE_WALK;
  }
}

// ---------- Leaping Fish ----------
// waits at the spot it spawned at (the water), then leaps up and falls back.
// the height it spawned at is kept in state[i]

static void update_fish(int i, float *y, float *velocity_y, int *state,
                        int *timer) {
  float rest_y = state[i];

  if (y[i] >= rest_y && velocity_y[i] >= 0) {
    y[i] = rest_y;
    velocity_y[i] = 0;

    if (timer[i] > 0) {
      timer[i]--;
    } else {
      velocity_y[i] = -FISH_JUMP_SPEED;
      timer[i] = FISH_LEAP_DELAY;
    }
  } else {
    velocity_y[i] += ENEMY_GRAVITY;
    y[i] += velocity_y[i];

    if (y[i] > rest_y) {
      y[i] = rest_y;
      velocity_y[i] = 0;
    }
  }
}

// ---------- the core functions for the enemies ----------

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

      if (newType == ENEMY_FISH) {
        // the fish remembers the height it spawned at, and each fish starts
        // its leap at a slightly different time
        state[i] = (int)new_y;
        timer[i] = FISH_LEAP_DELAY + i * 13;
      }

      return i;
    }
  }

  return -1;
}

void update_enemies(bool *active, int *type, float *x, float *y,
                    float *velocity_x, float *velocity_y, int *hp,
                    int *direction, int *state, int *timer, bool *bullet_active,
                    float *bullet_x, float *bullet_y, float *bullet_velocity_x,
                    float target_x, float target_y, char **lvl,
                    const int height, const int width, const int cell_size) {
  (void)hp;

  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!active[i])
      continue;

    switch (type[i]) {
    case ENEMY_CRAWLER:
      update_crawler(i, x, y, velocity_x, velocity_y, direction, lvl, height,
                     width, cell_size);
      break;
    case ENEMY_JUMPER:
      update_jumper(i, x, y, velocity_x, velocity_y, direction, state, timer,
                    target_x, lvl, height, width, cell_size);
      break;
    case ENEMY_INSECT:
      update_insect(i, x, y, velocity_x, direction, timer, target_x, target_y,
                    height, width, cell_size);
      break;
    case ENEMY_LEOPARD:
      update_leopard(i, x, y, velocity_x, velocity_y, direction, state, timer,
                     target_x, target_y, lvl, height, width, cell_size);
      break;
    case ENEMY_SHOOTER:
      update_shooter(i, x, y, velocity_y, direction, timer, bullet_active,
                     bullet_x, bullet_y, bullet_velocity_x, target_x, target_y,
                     lvl, height, width, cell_size);
      break;
    case ENEMY_TRIBAL:
      update_tribal(i, x, y, velocity_x, velocity_y, direction, state, timer,
                    target_x, target_y, lvl, height, width, cell_size);
      break;
    case ENEMY_FISH:
      update_fish(i, y, velocity_y, state, timer);
      break;
    default:
      break;
    }
  }
}

// draws every live enemy using the texture of its own type. the picture is
// scaled to fit the enemy's hitbox, and flipped when it faces left (the art is
// expected to face right)
void draw_enemies(RenderWindow &window, Sprite &enemySprite,
                  Texture *enemy_textures, bool *active, int *type, float *x,
                  float *y, int *direction) {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!active[i])
      continue;

    // safety check so a bad type number can never read outside the array
    if (type[i] < 0 || type[i] > ENEMY_FISH)
      continue;

    Texture &tex = enemy_textures[type[i]];
    enemySprite.setTexture(tex, true);

    int w = enemy_width(type[i]);
    int h = enemy_height(type[i]);
    float scale_x = (float)w / tex.getSize().x;
    float scale_y = (float)h / tex.getSize().y;

    // the fish always keeps its starting direction, so it never gets flipped
    bool face_left = (direction[i] < 0 && type[i] != ENEMY_FISH);

    if (face_left) {
      enemySprite.setScale(-scale_x, scale_y);
      enemySprite.setPosition(x[i] + w, y[i]);
    } else {
      enemySprite.setScale(scale_x, scale_y);
      enemySprite.setPosition(x[i], y[i]);
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
