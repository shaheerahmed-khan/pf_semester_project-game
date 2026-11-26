#include "enemies.h"

#include <cmath>

using namespace sf;

// ---------- shared tile helpers ----------

static char enemy_tile_at(char **lvl, int row, int col, int height, int width) {
    if (row < 0) {
        return '.';
    }
    if (row >= height || col < 0 || col >= width) {
        return '#';
    }
    return lvl[row][col];
}

// -1 if the target is on the left, 1 if it is on the right.
static int direction_to(float from_x, float to_x) {
    if (to_x < from_x) {
        return -1;
    }
    return 1;
}

// Pulls an enemy down and lands it on solid tiles. Returns true when the enemy
// is standing on the ground.
static bool apply_enemy_gravity(int i, float *x, float *y, float *velocity_y,
                                int w, int h, char **lvl, int height,
                                int width, int cell_size, float gravity,
                                float terminal_velocity) {
    velocity_y[i] += gravity;
    if (velocity_y[i] > terminal_velocity) {
        velocity_y[i] = terminal_velocity;
    }

    float next_y = y[i] + velocity_y[i];
    int left_col = (int)x[i] / cell_size;
    int right_col = (int)(x[i] + w - 1) / cell_size;

    if (velocity_y[i] >= 0) {
        int feet_row = (int)(next_y + h) / cell_size;
        char left_tile = enemy_tile_at(lvl, feet_row, left_col, height, width);
        char right_tile = enemy_tile_at(lvl, feet_row, right_col, height, width);

        if (left_tile == '#' || right_tile == '#') {
            y[i] = (float)(feet_row * cell_size - h);
            velocity_y[i] = 0;
            return true;
        }
    } else {
        int head_row = -1;
        if (next_y >= 0) {
            head_row = (int)next_y / cell_size;
        }

        char left_tile = enemy_tile_at(lvl, head_row, left_col, height, width);
        char right_tile = enemy_tile_at(lvl, head_row, right_col, height, width);

        if (left_tile == '#' || right_tile == '#') {
            y[i] = (float)((head_row + 1) * cell_size);
            velocity_y[i] = 0;
            return false;
        }
    }

    y[i] = next_y;
    return false;
}

static int front_column(float next_x, bool moving_right, int w, int cell_size) {
    if (moving_right) {
        return (int)(next_x + w - 1) / cell_size;
    }
    return (int)next_x / cell_size;
}

// True if a solid block or the screen edge is in front of the body.
static bool wall_ahead(float next_x, float y, bool moving_right, int w, int h,
                       char **lvl, int height, int width, int cell_size) {
    if (next_x < 0 || next_x + w > width * cell_size) {
        return true;
    }

    int col = front_column(next_x, moving_right, w, cell_size);
    int body_row = (int)(y + h - 1) / cell_size;

    return enemy_tile_at(lvl, body_row, col, height, width) == '#';
}

// True if there is no floor in front of the feet.
static bool ledge_ahead(float next_x, float y, bool moving_right, int w, int h,
                        char **lvl, int height, int width, int cell_size) {
    int col = front_column(next_x, moving_right, w, cell_size);
    int below_row = (int)(y + h) / cell_size;

    return enemy_tile_at(lvl, below_row, col, height, width) != '#';
}

// Moves sideways by the current velocity, refusing walls and ledges.
static bool try_walk(int i, float *x, float *y, float *velocity_x, int w, int h,
                     char **lvl, int height, int width, int cell_size) {
    float next_x = x[i] + velocity_x[i];
    bool moving_right = velocity_x[i] > 0;

    if (wall_ahead(next_x, y[i], moving_right, w, h, lvl, height, width,
                   cell_size)) {
        return false;
    }
    if (ledge_ahead(next_x, y[i], moving_right, w, h, lvl, height, width,
                    cell_size)) {
        return false;
    }

    x[i] = next_x;
    return true;
}

// ---------- shooter bullets ----------

void init_enemy_projectiles(int max_bullets, bool *bullet_active,
                            float *bullet_x, float *bullet_y,
                            float *bullet_velocity_x) {
    for (int i = 0; i < max_bullets; i++) {
        bullet_active[i] = false;
        bullet_x[i] = 0;
        bullet_y[i] = 0;
        bullet_velocity_x[i] = 0;
    }
}

static void spawn_enemy_projectile(int max_bullets, bool *bullet_active,
                                   float *bullet_x, float *bullet_y,
                                   float *bullet_velocity_x, float new_x,
                                   float new_y, float new_velocity_x) {
    for (int i = 0; i < max_bullets; i++) {
        if (!bullet_active[i]) {
            bullet_active[i] = true;
            bullet_x[i] = new_x;
            bullet_y[i] = new_y;
            bullet_velocity_x[i] = new_velocity_x;
            return;
        }
    }
}

void update_enemy_projectiles(int max_bullets, float bullet_cfg[4],
                              bool *bullet_active, float *bullet_x,
                              float *bullet_y, float *bullet_velocity_x,
                              char **lvl, int height, int width,
                              int cell_size) {
    int bullet_w = (int)bullet_cfg[1];
    int bullet_h = (int)bullet_cfg[2];

    for (int i = 0; i < max_bullets; i++) {
        if (!bullet_active[i]) {
            continue;
        }

        bullet_x[i] += bullet_velocity_x[i];

        if (bullet_x[i] < 0 ||
            bullet_x[i] + bullet_w > width * cell_size) {
            bullet_active[i] = false;
            continue;
        }

        int col = (int)(bullet_x[i] + bullet_w / 2) / cell_size;
        int row = (int)(bullet_y[i] + bullet_h / 2) / cell_size;

        if (enemy_tile_at(lvl, row, col, height, width) == '#') {
            bullet_active[i] = false;
        }
    }
}

void draw_enemy_projectiles(RenderWindow &window, Sprite &enemy_sprite,
                            Texture &bullet_texture, int max_bullets,
                            float bullet_cfg[4], bool *bullet_active,
                            float *bullet_x, float *bullet_y,
                            float *bullet_velocity_x) {
    int bullet_w = (int)bullet_cfg[1];
    int bullet_h = (int)bullet_cfg[2];

    // setTexture does not reset the crop, so reset it explicitly.
    enemy_sprite.setTexture(bullet_texture);
    enemy_sprite.setTextureRect(IntRect(0, 0,
                                       (int)bullet_texture.getSize().x,
                                       (int)bullet_texture.getSize().y));

    float scale_x = (float)bullet_w / bullet_texture.getSize().x;
    float scale_y = (float)bullet_h / bullet_texture.getSize().y;

    for (int i = 0; i < max_bullets; i++) {
        if (!bullet_active[i]) {
            continue;
        }

        if (bullet_velocity_x[i] < 0) {
            enemy_sprite.setScale(-scale_x, scale_y);
            enemy_sprite.setPosition(bullet_x[i] + bullet_w, bullet_y[i]);
        } else {
            enemy_sprite.setScale(scale_x, scale_y);
            enemy_sprite.setPosition(bullet_x[i], bullet_y[i]);
        }

        window.draw(enemy_sprite);
    }
}

int enemy_projectile_damage_on_contact(int max_bullets, float bullet_cfg[4],
                                       bool *bullet_active, float *bullet_x,
                                       float *bullet_y, float player_x,
                                       float player_y, float player_w,
                                       float player_h) {
    int bullet_w = (int)bullet_cfg[1];
    int bullet_h = (int)bullet_cfg[2];
    int bullet_damage = (int)bullet_cfg[3];

    for (int i = 0; i < max_bullets; i++) {
        if (!bullet_active[i]) {
            continue;
        }

        bool overlap = ((bullet_x[i] < player_x + player_w) &&
                        (bullet_x[i] + bullet_w > player_x) &&
                        (bullet_y[i] < player_y + player_h) &&
                        (bullet_y[i] + bullet_h > player_y));

        if (overlap) {
            bullet_active[i] = false;
            return bullet_damage;
        }
    }

    return 0;
}

// ---------- Spiked Crawler ----------
// Walks back and forth, turning at walls and ledges.
static void update_crawler(int i, float *x, float *y, float *velocity_x,
                           float *velocity_y, int *direction, float *cfg,
                           char **lvl, int height, int width, int cell_size,
                           float gravity, float terminal_velocity) {
    int w = (int)cfg[0];
    int h = (int)cfg[1];

    if (!apply_enemy_gravity(i, x, y, velocity_y, w, h, lvl, height, width,
                             cell_size, gravity, terminal_velocity)) {
        return;
    }

    if (!try_walk(i, x, y, velocity_x, w, h, lvl, height, width, cell_size)) {
        velocity_x[i] = -velocity_x[i];
        direction[i] = -direction[i];
    }
}

// ---------- Blue Jumping Enemy ----------
// Waits on the ground, then jumps toward the player.
static void update_jumper(int i, float *x, float *y, float *velocity_x,
                          float *velocity_y, int *direction, int *state,
                          int *timer, float *cfg, float target_x, char **lvl,
                          int height, int width, int cell_size, float gravity,
                          float terminal_velocity) {
    int w = (int)cfg[0];
    int h = (int)cfg[1];
    float speed = cfg[6];
    float jump_speed = cfg[7];
    int jump_delay = (int)cfg[8];
    float sight_range = cfg[9];

    bool landed = apply_enemy_gravity(i, x, y, velocity_y, w, h, lvl, height,
                                      width, cell_size, gravity,
                                      terminal_velocity);

    if (landed) {
        if (state[i] == 1) {
            state[i] = 0;
            timer[i] = jump_delay;
            velocity_x[i] = 0;
        }

        float center_x = x[i] + w / 2.0f;
        direction[i] = direction_to(center_x, target_x);

        if (timer[i] > 0) {
            timer[i]--;
        } else if (fabs(target_x - center_x) < sight_range) {
            velocity_x[i] = direction[i] * speed;
            velocity_y[i] = -jump_speed;
            state[i] = 1;
        }
    } else {
        float next_x = x[i] + velocity_x[i];
        bool moving_right = velocity_x[i] > 0;

        if (wall_ahead(next_x, y[i], moving_right, w, h, lvl, height, width,
                       cell_size)) {
            velocity_x[i] = 0;
        } else {
            x[i] = next_x;
        }
    }
}

// ---------- Flying Insect ----------
// Flies toward the player, bobbing up and down.
static void update_insect(int i, float *x, float *y, float *velocity_x,
                          int *direction, int *timer, float *cfg,
                          float target_x, float target_y, int height,
                          int width, int cell_size) {
    int w = (int)cfg[0];
    int h = (int)cfg[1];
    float speed = cfg[6];
    float vertical_speed = cfg[7];
    float wave_speed = cfg[8];
    float wave_size = cfg[9];

    timer[i]++;

    float center_x = x[i] + w / 2.0f;
    float center_y = y[i] + h / 2.0f;

    direction[i] = direction_to(center_x, target_x);

    if (fabs(target_x - center_x) < speed) {
        velocity_x[i] = 0;
    } else {
        velocity_x[i] = direction[i] * speed;
    }

    x[i] += velocity_x[i];

    float drift = 0;
    if (target_y > center_y + vertical_speed) {
        drift = vertical_speed;
    } else if (target_y < center_y - vertical_speed) {
        drift = -vertical_speed;
    }

    y[i] += drift + sin(timer[i] * wave_speed) * wave_size;

    if (x[i] < 0) {
        x[i] = 0;
    }
    if (x[i] + w > width * cell_size) {
        x[i] = (float)(width * cell_size - w);
    }
    if (y[i] < 0) {
        y[i] = 0;
    }
    if (y[i] + h > height * cell_size) {
        y[i] = (float)(height * cell_size - h);
    }
}

// ---------- Leopard ----------
// Patrols, charges when it sees the player, then rests.
static void update_leopard(int i, float *x, float *y, float *velocity_x,
                           float *velocity_y, int *direction, int *state,
                           int *timer, float *cfg, float target_x,
                           float target_y, char **lvl, int height, int width,
                           int cell_size, float gravity,
                           float terminal_velocity) {
    int w = (int)cfg[0];
    int h = (int)cfg[1];
    float patrol_speed = cfg[4];
    float charge_speed = cfg[6];
    float sight_range = cfg[7];
    float sight_height = cfg[8];
    int charge_frames = (int)cfg[9];
    int recover_frames = (int)cfg[10];

    if (!apply_enemy_gravity(i, x, y, velocity_y, w, h, lvl, height, width,
                             cell_size, gravity, terminal_velocity)) {
        return;
    }

    float center_x = x[i] + w / 2.0f;
    float center_y = y[i] + h / 2.0f;

    if (state[i] == 0) {
        if (!try_walk(i, x, y, velocity_x, w, h, lvl, height, width,
                      cell_size)) {
            velocity_x[i] = -velocity_x[i];
            direction[i] = -direction[i];
        }

        bool sees_player = (fabs(target_x - center_x) < sight_range &&
                            fabs(target_y - center_y) < sight_height);

        if (sees_player) {
            direction[i] = direction_to(center_x, target_x);
            velocity_x[i] = direction[i] * charge_speed;
            state[i] = 1;
            timer[i] = charge_frames;
        }
    } else if (state[i] == 1) {
        bool moved = try_walk(i, x, y, velocity_x, w, h, lvl, height, width,
                              cell_size);
        timer[i]--;

        if (!moved || timer[i] <= 0) {
            velocity_x[i] = 0;
            state[i] = 2;
            timer[i] = recover_frames;
        }
    } else {
        timer[i]--;

        if (timer[i] <= 0) {
            state[i] = 0;
            velocity_x[i] = direction[i] * patrol_speed;
        }
    }
}

// ---------- Green Shooter ----------
// Stands still, faces the player, and shoots when the player is in sight.
static void update_shooter(int i, float *x, float *y, float *velocity_y,
                           int *direction, int *timer, float *cfg,
                           int max_bullets, float bullet_cfg[4],
                           bool *bullet_active, float *bullet_x,
                           float *bullet_y, float *bullet_velocity_x,
                           float target_x, float target_y, char **lvl,
                           int height, int width, int cell_size, float gravity,
                           float terminal_velocity) {
    int w = (int)cfg[0];
    int h = (int)cfg[1];
    float sight_range = cfg[6];
    float sight_height = cfg[7];
    int shoot_delay = (int)cfg[8];

    float bullet_speed = bullet_cfg[0];
    int bullet_w = (int)bullet_cfg[1];

    if (!apply_enemy_gravity(i, x, y, velocity_y, w, h, lvl, height, width,
                             cell_size, gravity, terminal_velocity)) {
        return;
    }

    float center_x = x[i] + w / 2.0f;
    float center_y = y[i] + h / 2.0f;

    direction[i] = direction_to(center_x, target_x);

    if (timer[i] > 0) {
        timer[i]--;
    }

    bool sees_player = (fabs(target_x - center_x) < sight_range &&
                        fabs(target_y - center_y) < sight_height);

    if (sees_player && timer[i] == 0) {
        float start_x = x[i] + w;
        if (direction[i] < 0) {
            start_x = x[i] - bullet_w;
        }

        spawn_enemy_projectile(max_bullets, bullet_active, bullet_x, bullet_y,
                               bullet_velocity_x, start_x, y[i] + h / 3.0f,
                               direction[i] * bullet_speed);
        timer[i] = shoot_delay;
    }
}

// ---------- Tribal Warrior ----------
// Walks toward the player, winds up, lunges, then rests.
static void update_tribal(int i, float *x, float *y, float *velocity_x,
                          float *velocity_y, int *direction, int *state,
                          int *timer, float *cfg, float target_x,
                          float target_y, char **lvl, int height, int width,
                          int cell_size, float gravity,
                          float terminal_velocity) {
    int w = (int)cfg[0];
    int h = (int)cfg[1];
    float walk_speed = cfg[6];
    float lunge_speed = cfg[7];
    float sight_range = cfg[8];
    float sight_height = cfg[9];
    float attack_range = cfg[10];
    int windup_frames = (int)cfg[11];
    int lunge_frames = (int)cfg[12];
    int cooldown_frames = (int)cfg[13];

    if (!apply_enemy_gravity(i, x, y, velocity_y, w, h, lvl, height, width,
                             cell_size, gravity, terminal_velocity)) {
        return;
    }

    float center_x = x[i] + w / 2.0f;
    float center_y = y[i] + h / 2.0f;

    if (state[i] == 0) {
        float distance = fabs(target_x - center_x);
        bool sees_player = (distance < sight_range &&
                            fabs(target_y - center_y) < sight_height);

        if (!sees_player) {
            velocity_x[i] = 0;
            return;
        }

        direction[i] = direction_to(center_x, target_x);

        if (distance < attack_range) {
            velocity_x[i] = 0;
            state[i] = 1;
            timer[i] = windup_frames;
        } else {
            velocity_x[i] = direction[i] * walk_speed;
            try_walk(i, x, y, velocity_x, w, h, lvl, height, width, cell_size);
        }
    } else if (state[i] == 1) {
        timer[i]--;

        if (timer[i] <= 0) {
            velocity_x[i] = direction[i] * lunge_speed;
            state[i] = 2;
            timer[i] = lunge_frames;
        }
    } else if (state[i] == 2) {
        bool moved = try_walk(i, x, y, velocity_x, w, h, lvl, height, width,
                              cell_size);
        timer[i]--;

        if (!moved || timer[i] <= 0) {
            velocity_x[i] = 0;
            state[i] = 3;
            timer[i] = cooldown_frames;
        }
    } else {
        timer[i]--;

        if (timer[i] <= 0) {
            state[i] = 0;
        }
    }
}

// ---------- Leaping Fish ----------
// Waits at the height it spawned at (the water), then leaps and falls back.
// The spawn height is stored in state[i].
static void update_fish(int i, float *y, float *velocity_y, int *state,
                        int *timer, float *cfg, float gravity,
                        float terminal_velocity) {
    float rest_y = (float)state[i];
    float jump_speed = cfg[6];
    int leap_delay = (int)cfg[7];

    if (y[i] >= rest_y && velocity_y[i] >= 0) {
        y[i] = rest_y;
        velocity_y[i] = 0;

        if (timer[i] > 0) {
            timer[i]--;
        } else {
            velocity_y[i] = -jump_speed;
            timer[i] = leap_delay;
        }
    } else {
        velocity_y[i] += gravity;
        if (velocity_y[i] > terminal_velocity) {
            velocity_y[i] = terminal_velocity;
        }
        y[i] += velocity_y[i];

        if (y[i] > rest_y) {
            y[i] = rest_y;
            velocity_y[i] = 0;
        }
    }
}

// ---------- core enemy functions ----------

void init_enemies(int max_enemies, bool *active, int *type, float *x, float *y,
                  float *velocity_x, float *velocity_y, int *hp,
                  int *direction, int *state, int *timer) {
    for (int i = 0; i < max_enemies; i++) {
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

int spawn_enemy(int max_enemies, int enemy_type_count, float cfg[][14],
                bool *active, int *type, float *x, float *y,
                float *velocity_x, float *velocity_y, int *hp, int *direction,
                int *state, int *timer, int new_type, float new_x,
                float new_y) {
    if (new_type < 0 || new_type >= enemy_type_count) {
        return -1;
    }

    float *row = cfg[new_type];

    for (int i = 0; i < max_enemies; i++) {
        if (!active[i]) {
            active[i] = true;
            type[i] = new_type;
            x[i] = new_x;
            y[i] = new_y;
            direction[i] = -1;
            velocity_x[i] = direction[i] * row[4];
            velocity_y[i] = 0;
            hp[i] = (int)row[2];
            state[i] = 0;
            timer[i] = 0;

            // Type 6 is the fish: remember its water height and stagger leaps.
            if (new_type == 6) {
                state[i] = (int)new_y;
                timer[i] = (int)row[7] + i * 13;
            }

            return i;
        }
    }

    return -1;
}

void update_enemies(int max_enemies, int enemy_type_count, float cfg[][14],
                    int max_bullets, float bullet_cfg[4], bool *active,
                    int *type, float *x, float *y, float *velocity_x,
                    float *velocity_y, int *direction, int *state, int *timer,
                    bool *bullet_active, float *bullet_x, float *bullet_y,
                    float *bullet_velocity_x, float target_x, float target_y,
                    char **lvl, int height, int width, int cell_size,
                    float gravity, float terminal_velocity) {
    for (int i = 0; i < max_enemies; i++) {
        if (!active[i]) {
            continue;
        }
        if (type[i] < 0 || type[i] >= enemy_type_count) {
            continue;
        }

        float *row = cfg[type[i]];

        switch (type[i]) {
        case 0: // crawler
            update_crawler(i, x, y, velocity_x, velocity_y, direction, row, lvl,
                           height, width, cell_size, gravity,
                           terminal_velocity);
            break;
        case 1: // jumper
            update_jumper(i, x, y, velocity_x, velocity_y, direction, state,
                          timer, row, target_x, lvl, height, width, cell_size,
                          gravity, terminal_velocity);
            break;
        case 2: // insect
            update_insect(i, x, y, velocity_x, direction, timer, row, target_x,
                          target_y, height, width, cell_size);
            break;
        case 3: // leopard
            update_leopard(i, x, y, velocity_x, velocity_y, direction, state,
                           timer, row, target_x, target_y, lvl, height, width,
                           cell_size, gravity, terminal_velocity);
            break;
        case 4: // shooter
            update_shooter(i, x, y, velocity_y, direction, timer, row,
                           max_bullets, bullet_cfg, bullet_active, bullet_x,
                           bullet_y, bullet_velocity_x, target_x, target_y,
                           lvl, height, width, cell_size, gravity,
                           terminal_velocity);
            break;
        case 5: // tribal warrior
            update_tribal(i, x, y, velocity_x, velocity_y, direction, state,
                          timer, row, target_x, target_y, lvl, height, width,
                          cell_size, gravity, terminal_velocity);
            break;
        case 6: // leaping fish
            update_fish(i, y, velocity_y, state, timer, row, gravity,
                        terminal_velocity);
            break;
        default:
            break;
        }
    }
}

void draw_enemies(RenderWindow &window, Sprite &enemy_sprite,
                  Texture *enemy_textures, int max_enemies,
                  int enemy_type_count, float cfg[][14], bool *active,
                  int *type, float *x, float *y, int *direction) {
    for (int i = 0; i < max_enemies; i++) {
        if (!active[i]) {
            continue;
        }
        if (type[i] < 0 || type[i] >= enemy_type_count) {
            continue;
        }

        float *row = cfg[type[i]];
        int w = (int)row[0];
        int h = (int)row[1];

        Texture &tex = enemy_textures[type[i]];

        enemy_sprite.setTexture(tex);
        enemy_sprite.setTextureRect(IntRect(0, 0, (int)tex.getSize().x,
                                            (int)tex.getSize().y));

        float scale_x = (float)w / tex.getSize().x;
        float scale_y = (float)h / tex.getSize().y;

        // The fish always keeps its starting direction.
        bool face_left = (direction[i] < 0 && type[i] != 6);

        if (face_left) {
            enemy_sprite.setScale(-scale_x, scale_y);
            enemy_sprite.setPosition(x[i] + w, y[i]);
        } else {
            enemy_sprite.setScale(scale_x, scale_y);
            enemy_sprite.setPosition(x[i], y[i]);
        }

        window.draw(enemy_sprite);
    }
}

bool damage_enemy(int max_enemies, bool *active, int *hp, int index,
                  int amount) {
    if (index < 0 || index >= max_enemies || !active[index]) {
        return false;
    }

    hp[index] -= amount;

    if (hp[index] <= 0) {
        active[index] = false;
        return true;
    }

    return false;
}

int enemy_damage_on_contact(int max_enemies, int enemy_type_count,
                            float cfg[][14], bool *active, int *type,
                            float *x, float *y, float player_x,
                            float player_y, float player_w, float player_h) {
    for (int i = 0; i < max_enemies; i++) {
        if (!active[i]) {
            continue;
        }
        if (type[i] < 0 || type[i] >= enemy_type_count) {
            continue;
        }

        float *row = cfg[type[i]];
        float enemy_w = row[0];
        float enemy_h = row[1];

        bool overlap = ((x[i] < player_x + player_w) &&
                        (x[i] + enemy_w > player_x) &&
                        (y[i] < player_y + player_h) &&
                        (y[i] + enemy_h > player_y));

        if (overlap) {
            return (int)row[3];
        }
    }

    return 0;
}
