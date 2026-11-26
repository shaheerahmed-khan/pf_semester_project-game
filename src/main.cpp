#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>

using namespace sf;
using namespace std;

// ===========================================================================
// Level grid
// ===========================================================================

bool is_solid(char **lvl, int height, int width, int row, int col) {
    // Left and right screen edges are walls; above and below the map are not.
    if (col < 0 || col >= width) {
        return true;
    }
    if (row < 0 || row >= height) {
        return false;
    }

    return lvl[row][col] == '#';
}

// Loads the ASCII map with plain C file I/O and fixed char buffers.
char **load_level(const char *path, int &height, int &width, float &start_x,
                  float &start_y, int cell_size, int player_height) {
    FILE *file = fopen(path, "r");
    if (file == nullptr) {
        return nullptr;
    }

    char line[512];
    int rows = 0;
    int columns = 0;

    while (fgets(line, (int)sizeof(line), file) != nullptr) {
        int length = (int)strlen(line);
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

    char **level = new char *[rows];
    for (int i = 0; i < rows; i++) {
        level[i] = new char[columns];
        for (int j = 0; j < columns; j++) {
            level[i][j] = '.';
        }
    }

    rewind(file);

    for (int i = 0; i < rows; i++) {
        if (fgets(line, (int)sizeof(line), file) == nullptr) {
            line[0] = '\0';
        }

        int length = (int)strlen(line);
        while (length > 0 &&
               (line[length - 1] == '\n' || line[length - 1] == '\r')) {
            length--;
        }

        for (int j = 0; j < columns; j++) {
            if (j >= length) {
                continue;
            }

            level[i][j] = line[j];

            if (line[j] == 'P') {
                start_x = (float)(j * cell_size);
                start_y = (float)((i + 1) * cell_size - player_height);
                level[i][j] = '.';
            }
        }
    }

    fclose(file);

    height = rows;
    width = columns;
    return level;
}

void free_level(char **lvl, int height) {
    if (lvl == nullptr) {
        return;
    }

    for (int i = 0; i < height; i++) {
        delete[] lvl[i];
    }
    delete[] lvl;
}

void draw_level(RenderWindow &window, char **lvl, Sprite &bgSprite,
                Sprite &blockSprite, int height, int width, int cell_size) {
    window.draw(bgSprite);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (lvl[i][j] == '#') {
                blockSprite.setPosition((float)(j * cell_size),
                                        (float)(i * cell_size));
                window.draw(blockSprite);
            }
        }
    }
}

// ===========================================================================
// Physics
// ===========================================================================

void apply_gravity(float &vertical_speed, float gravity,
                   float terminal_velocity) {
    vertical_speed += gravity;
    if (vertical_speed > terminal_velocity) {
        vertical_speed = terminal_velocity;
    }
}

bool collides_with_solid(char **level_grid, int level_height, int level_width,
                         float box_x, float box_y, float box_width,
                         float box_height, int cell_size,
                         float collision_eps) {
    int left_column = (int)std::floor(box_x / cell_size);
    int right_column =
        (int)std::floor((box_x + box_width - collision_eps) / cell_size);
    int top_row = (int)std::floor(box_y / cell_size);
    int bottom_row =
        (int)std::floor((box_y + box_height - collision_eps) / cell_size);

    for (int row = top_row; row <= bottom_row; row++) {
        for (int col = left_column; col <= right_column; col++) {
            if (is_solid(level_grid, level_height, level_width, row, col)) {
                return true;
            }
        }
    }
    return false;
}

void move_with_collision(char **level_grid, int level_height, int level_width,
                         float &box_x, float &box_y, float box_width,
                         float box_height, float &horizontal_speed,
                         float &vertical_speed, bool &is_on_ground,
                         int cell_size, float collision_eps) {
    is_on_ground = false;

    // Sideways movement.
    box_x += horizontal_speed;

    if (collides_with_solid(level_grid, level_height, level_width, box_x, box_y,
                            box_width, box_height, cell_size, collision_eps)) {
        if (horizontal_speed > 0) {
            int wall_column =
                (int)std::floor((box_x + box_width) / cell_size);
            box_x = wall_column * cell_size - box_width - collision_eps;
        } else if (horizontal_speed < 0) {
            int wall_column = (int)std::floor(box_x / cell_size);
            box_x = (wall_column + 1) * cell_size + collision_eps;
        }

        horizontal_speed = 0;
    }

    // Vertical movement.
    box_y += vertical_speed;

    if (collides_with_solid(level_grid, level_height, level_width, box_x, box_y,
                            box_width, box_height, cell_size, collision_eps)) {
        if (vertical_speed > 0) {
            int floor_row =
                (int)std::floor((box_y + box_height) / cell_size);
            box_y = floor_row * cell_size - box_height - collision_eps;
            is_on_ground = true;
        } else if (vertical_speed < 0) {
            int ceiling_row = (int)std::floor(box_y / cell_size);
            box_y = (ceiling_row + 1) * cell_size + collision_eps;
        }

        vertical_speed = 0;
    }
}

// ===========================================================================
// Player
// ===========================================================================

void update_player(float &player_x, float &player_y, float &velocity_x,
                   float &velocity_y, bool &on_ground, int &facing,
                   bool move_right, bool jump_pressed, float speed,
                   float jump_speed, int player_width, int player_height,
                   char **lvl, int height, int width, int cell_size,
                   float gravity, float terminal_velocity,
                   float collision_eps) {
    velocity_x = 0;

    if (move_right) {
        velocity_x = speed;
        facing = 1;
    }

    if (jump_pressed && on_ground) {
        velocity_y = -jump_speed;
    }

    apply_gravity(velocity_y, gravity, terminal_velocity);
    move_with_collision(lvl, height, width, player_x, player_y, player_width,
                        player_height, velocity_x, velocity_y, on_ground,
                        cell_size, collision_eps);
}

void respawn_player(float &player_x, float &player_y, float &velocity_x,
                    float &velocity_y, bool &on_ground, float start_x,
                    float start_y) {
    player_x = start_x;
    player_y = start_y;
    velocity_x = 0;
    velocity_y = 0;
    on_ground = false;
}

bool player_fell_off_map(float y, int height, int cell_size) {
    return y > height * cell_size;
}

void draw_player(RenderWindow &window, Sprite &sprite, float player_x,
                 float player_y, int facing, int player_scale,
                 int player_width) {
    if (facing >= 0) {
        sprite.setScale((float)player_scale, (float)player_scale);
        sprite.setPosition(player_x, player_y);
    } else {
        sprite.setScale((float)-player_scale, (float)player_scale);
        sprite.setPosition(player_x + player_width, player_y);
    }

    window.draw(sprite);
}

// ===========================================================================
// Enemies
// ===========================================================================

char enemy_tile_at(char **lvl, int row, int col, int height, int width) {
    if (row < 0) {
        return '.';
    }
    if (row >= height || col < 0 || col >= width) {
        return '#';
    }
    return lvl[row][col];
}

// -1 if the target is on the left, 1 if it is on the right.
int direction_to(float from_x, float to_x) {
    if (to_x < from_x) {
        return -1;
    }
    return 1;
}

// Pulls an enemy down and lands it on solid tiles.
bool apply_enemy_gravity(int i, float *x, float *y, float *velocity_y, int w,
                         int h, char **lvl, int height, int width,
                         int cell_size, float gravity,
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
        char right_tile =
            enemy_tile_at(lvl, feet_row, right_col, height, width);

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
        char right_tile =
            enemy_tile_at(lvl, head_row, right_col, height, width);

        if (left_tile == '#' || right_tile == '#') {
            y[i] = (float)((head_row + 1) * cell_size);
            velocity_y[i] = 0;
            return false;
        }
    }

    y[i] = next_y;
    return false;
}

int front_column(float next_x, bool moving_right, int w, int cell_size) {
    if (moving_right) {
        return (int)(next_x + w - 1) / cell_size;
    }
    return (int)next_x / cell_size;
}

bool wall_ahead(float next_x, float y, bool moving_right, int w, int h,
                char **lvl, int height, int width, int cell_size) {
    if (next_x < 0 || next_x + w > width * cell_size) {
        return true;
    }

    int col = front_column(next_x, moving_right, w, cell_size);
    int body_row = (int)(y + h - 1) / cell_size;

    return enemy_tile_at(lvl, body_row, col, height, width) == '#';
}

bool ledge_ahead(float next_x, float y, bool moving_right, int w, int h,
                 char **lvl, int height, int width, int cell_size) {
    int col = front_column(next_x, moving_right, w, cell_size);
    int below_row = (int)(y + h) / cell_size;

    return enemy_tile_at(lvl, below_row, col, height, width) != '#';
}

bool try_walk(int i, float *x, float *y, float *velocity_x, int w, int h,
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

void spawn_enemy_projectile(int max_bullets, bool *bullet_active,
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

void update_crawler(int i, float *x, float *y, float *velocity_x,
                    float *velocity_y, int *direction, float *cfg, char **lvl,
                    int height, int width, int cell_size, float gravity,
                    float terminal_velocity) {
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

void update_jumper(int i, float *x, float *y, float *velocity_x,
                   float *velocity_y, int *direction, int *state, int *timer,
                   float *cfg, float target_x, char **lvl, int height,
                   int width, int cell_size, float gravity,
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
        } else if (std::fabs(target_x - center_x) < sight_range) {
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

void update_insect(int i, float *x, float *y, float *velocity_x,
                   int *direction, int *timer, float *cfg, float target_x,
                   float target_y, int height, int width, int cell_size) {
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

    if (std::fabs(target_x - center_x) < speed) {
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

    y[i] += drift + std::sin(timer[i] * wave_speed) * wave_size;

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

void update_leopard(int i, float *x, float *y, float *velocity_x,
                    float *velocity_y, int *direction, int *state, int *timer,
                    float *cfg, float target_x, float target_y, char **lvl,
                    int height, int width, int cell_size, float gravity,
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

        bool sees_player = (std::fabs(target_x - center_x) < sight_range &&
                            std::fabs(target_y - center_y) < sight_height);

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

void update_shooter(int i, float *x, float *y, float *velocity_y,
                    int *direction, int *timer, float *cfg, int max_bullets,
                    float bullet_cfg[4], bool *bullet_active, float *bullet_x,
                    float *bullet_y, float *bullet_velocity_x, float target_x,
                    float target_y, char **lvl, int height, int width,
                    int cell_size, float gravity, float terminal_velocity) {
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

    bool sees_player = (std::fabs(target_x - center_x) < sight_range &&
                        std::fabs(target_y - center_y) < sight_height);

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

void update_tribal(int i, float *x, float *y, float *velocity_x,
                   float *velocity_y, int *direction, int *state, int *timer,
                   float *cfg, float target_x, float target_y, char **lvl,
                   int height, int width, int cell_size, float gravity,
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
        float distance = std::fabs(target_x - center_x);
        bool sees_player = (distance < sight_range &&
                            std::fabs(target_y - center_y) < sight_height);

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

void update_fish(int i, float *y, float *velocity_y, int *state, int *timer,
                 float *cfg, float gravity, float terminal_velocity) {
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

// ===========================================================================
// Game
// ===========================================================================

int main() {
    // ---------- every tunable number lives here ----------
    // No global variables and no global constants: main owns the numbers and
    // passes them to the functions that need them.

    const int screen_x = 1136;
    const int screen_y = 896;
    const int cell_size = 64;
    const float gravity = 1.0f;
    const float terminal_velocity = 20.0f;
    const float collision_eps = 0.01f;
    const int frame_ms = 16;

    // player (the picture is 18 x 32 pixels, drawn at scale 3)
    const int player_start_health = 100;
    const int player_vuln_frames = 60;
    const float player_speed = 5.0f;
    const float jump_speed = 20.0f;
    const int player_scale = 3;
    const int player_width = 54;
    const int player_height = 96;

    // enemies: the type numbers are the row numbers of the table below and
    // the order of the enemy pictures.
    const int max_enemies = 20;
    const int max_enemy_bullets = 10;
    const int enemy_type_count = 7;

    const int enemy_crawler = 0;
    const int enemy_jumper = 1;
    const int enemy_insect = 2;
    const int enemy_leopard = 3;
    const int enemy_shooter = 4;
    const int enemy_tribal = 5;
    const int enemy_fish = 6;

    // One row per enemy type. Columns:
    //   0 width, 1 height, 2 hp, 3 contact damage, 4 walk speed, 5 score,
    //   6..13 type-specific values (unused ones are 0).
    float enemy_cfg[enemy_type_count][14] = {
        // crawler
        {56, 40, 20, 10, 2, 100, 0, 0, 0, 0, 0, 0, 0, 0},
        // jumper: speed 4, jump speed 16, jump delay 60, sight range 500
        {48, 64, 30, 10, 0, 150, 4, 16, 60, 500, 0, 0, 0, 0},
        // insect: speed 2.5, vertical speed 1.5, wave speed 0.1, wave size 3
        {40, 32, 10, 5, 0, 100, 2.5f, 1.5f, 0.1f, 3, 0, 0, 0, 0},
        // leopard: patrol 1.5, charge 7, sight 400 x 64,
        // charge 45 frames, recover 60 frames
        {80, 48, 40, 20, 1.5f, 300, 7, 400, 64, 45, 60, 0, 0, 0},
        // shooter: sight 500 x 96, shoots every 90 frames
        {48, 80, 20, 10, 0, 200, 500, 96, 90, 0, 0, 0, 0, 0},
        // tribal: walk 1.5, lunge 6, sight 350 x 96, attack range 90,
        // windup 30, lunge 15, cooldown 60 frames
        {56, 96, 50, 20, 0, 400, 1.5f, 6, 350, 96, 90, 30, 15, 60},
        // fish: jump speed 20, leap delay 90 frames
        {32, 40, 10, 15, 0, 150, 20, 90, 0, 0, 0, 0, 0, 0}};

    // shooter bullets: speed, width, height, damage
    float enemy_bullet_cfg[4] = {6, 16, 8, 10};

    RenderWindow window(VideoMode(screen_x, screen_y), "VICE: Project Doom");

    // ---------- level ----------
    int height = 14;
    int width = 18;
    float start_x = (float)cell_size;
    float start_y = (float)(13 * cell_size - player_height);

    char **lvl = load_level("levels/level1.txt", height, width, start_x,
                            start_y, cell_size, player_height);

    if (lvl == nullptr) {
        cout << "Could not load levels/level1.txt, using the test floor"
             << endl;

        height = 14;
        width = 18;
        lvl = new char *[height];
        for (int i = 0; i < height; i++) {
            lvl[i] = new char[width];
            for (int j = 0; j < width; j++) {
                lvl[i][j] = '.';
            }
        }
        for (int j = 0; j < width; j++) {
            lvl[height - 1][j] = '#';
        }

        start_x = (float)cell_size;
        start_y = (float)((height - 1) * cell_size - player_height);
    }

    // ---------- background and tiles ----------
    Texture bg_texture;
    Sprite bg_sprite;
    Texture block_texture;
    Sprite block_sprite;

    if (!bg_texture.loadFromFile("assets/backgrounds/level1.png")) {
        cout << "Could not load assets/backgrounds/level1.png" << endl;
        return 1;
    }
    bg_sprite.setTexture(bg_texture);
    bg_sprite.setPosition(0, 0);

    if (!block_texture.loadFromFile("assets/tiles/block1.png")) {
        cout << "Could not load assets/tiles/block1.png" << endl;
        return 1;
    }
    block_sprite.setTexture(block_texture);

    // ---------- player ----------
    float player_x = start_x;
    float player_y = start_y;
    float player_velocity_x = 0;
    float player_velocity_y = 0;
    bool player_on_ground = false;
    int player_facing = 1;
    int player_health = player_start_health;
    int invuln_timer = 0;

    Texture player_texture;
    Sprite player_sprite;

    if (!player_texture.loadFromFile(
            "assets/sprites/player/hart_sprite.png")) {
        cout << "Could not load assets/sprites/player/hart_sprite.png" << endl;
        return 1;
    }
    player_sprite.setTexture(player_texture);

    // ---------- enemy pictures ----------
    const char *enemy_files[enemy_type_count];
    enemy_files[enemy_crawler] =
        "assets/sprites/enemies/spiked_crawler.png";
    enemy_files[enemy_jumper] = "assets/sprites/enemies/blue_jumper.png";
    enemy_files[enemy_insect] = "assets/sprites/enemies/flying_insect.png";
    enemy_files[enemy_leopard] = "assets/sprites/enemies/leopard.png";
    enemy_files[enemy_shooter] = "assets/sprites/enemies/green_shooter.png";
    enemy_files[enemy_tribal] =
        "assets/sprites/enemies/tribal_warrior.png";
    enemy_files[enemy_fish] = "assets/sprites/enemies/leaping_fish.png";

    Texture enemy_textures[enemy_type_count];
    for (int i = 0; i < enemy_type_count; i++) {
        if (!enemy_textures[i].loadFromFile(enemy_files[i])) {
            cout << "Could not load " << enemy_files[i] << endl;
            return 1;
        }
    }

    Texture bullet_texture;
    if (!bullet_texture.loadFromFile("assets/sprites/fx/enemy_bolt.png")) {
        cout << "Could not load assets/sprites/fx/enemy_bolt.png" << endl;
        return 1;
    }

    // One sprite is reused for every enemy and bullet.
    Sprite enemy_sprite;

    // ---------- enemy data ----------
    bool enemy_active[max_enemies];
    int enemy_type[max_enemies];
    float enemy_x[max_enemies];
    float enemy_y[max_enemies];
    float enemy_velocity_x[max_enemies];
    float enemy_velocity_y[max_enemies];
    int enemy_hp[max_enemies];
    int enemy_direction[max_enemies];
    int enemy_state[max_enemies];
    int enemy_timer[max_enemies];

    init_enemies(max_enemies, enemy_active, enemy_type, enemy_x, enemy_y,
                 enemy_velocity_x, enemy_velocity_y, enemy_hp, enemy_direction,
                 enemy_state, enemy_timer);

    // ---------- shooter bullet data ----------
    bool bullet_active[max_enemy_bullets];
    float bullet_x[max_enemy_bullets];
    float bullet_y[max_enemy_bullets];
    float bullet_velocity_x[max_enemy_bullets];

    init_enemy_projectiles(max_enemy_bullets, bullet_active, bullet_x,
                           bullet_y, bullet_velocity_x);

    // One crawler to start with.
    spawn_enemy(max_enemies, enemy_type_count, enemy_cfg, enemy_active,
                enemy_type, enemy_x, enemy_y, enemy_velocity_x,
                enemy_velocity_y, enemy_hp, enemy_direction, enemy_state,
                enemy_timer, enemy_crawler, 100, 100);

    Event ev;
    Clock frame_clock;

    // ---------- main loop ----------
    while (window.isOpen()) {
        while (window.pollEvent(ev)) {
            if (ev.type == Event::Closed) {
                window.close();
            }
        }

        // The only permitted controls: Right moves, Space jumps.
        bool move_right = Keyboard::isKeyPressed(Keyboard::Right);
        bool jump_pressed = Keyboard::isKeyPressed(Keyboard::Space);

        update_player(player_x, player_y, player_velocity_x, player_velocity_y,
                      player_on_ground, player_facing, move_right,
                      jump_pressed, player_speed, jump_speed, player_width,
                      player_height, lvl, height, width, cell_size, gravity,
                      terminal_velocity, collision_eps);

        if (player_fell_off_map(player_y, height, cell_size)) {
            respawn_player(player_x, player_y, player_velocity_x,
                           player_velocity_y, player_on_ground, start_x,
                           start_y);
        }

        // Enemies aim at the middle of the player.
        float target_x = player_x + player_width / 2.0f;
        float target_y = player_y + player_height / 2.0f;

        update_enemies(
            max_enemies, enemy_type_count, enemy_cfg, max_enemy_bullets,
            enemy_bullet_cfg, enemy_active, enemy_type, enemy_x, enemy_y,
            enemy_velocity_x, enemy_velocity_y, enemy_direction, enemy_state,
            enemy_timer, bullet_active, bullet_x, bullet_y,
            bullet_velocity_x, target_x, target_y, lvl, height, width,
            cell_size, gravity, terminal_velocity);

        update_enemy_projectiles(max_enemy_bullets, enemy_bullet_cfg,
                                 bullet_active, bullet_x, bullet_y,
                                 bullet_velocity_x, lvl, height, width,
                                 cell_size);

        if (invuln_timer > 0) {
            invuln_timer--;
        }

        int touch_damage = enemy_damage_on_contact(
            max_enemies, enemy_type_count, enemy_cfg, enemy_active, enemy_type,
            enemy_x, enemy_y, player_x, player_y, player_width,
            player_height);

        if (touch_damage == 0) {
            touch_damage = enemy_projectile_damage_on_contact(
                max_enemy_bullets, enemy_bullet_cfg, bullet_active, bullet_x,
                bullet_y, player_x, player_y, player_width, player_height);
        }

        if (touch_damage > 0 && invuln_timer == 0) {
            player_health -= touch_damage;
            invuln_timer = player_vuln_frames;
            cout << "Player health: " << player_health << endl;

            if (player_health <= 0) {
                cout << "Player died, health reset" << endl;
                player_health = player_start_health;
                invuln_timer = player_vuln_frames;
                respawn_player(player_x, player_y, player_velocity_x,
                               player_velocity_y, player_on_ground, start_x,
                               start_y);
            }
        }

        // ---------- drawing ----------
        window.clear();

        draw_level(window, lvl, bg_sprite, block_sprite, height, width,
                   cell_size);
        draw_enemies(window, enemy_sprite, enemy_textures, max_enemies,
                     enemy_type_count, enemy_cfg, enemy_active, enemy_type,
                     enemy_x, enemy_y, enemy_direction);
        draw_enemy_projectiles(window, enemy_sprite, bullet_texture,
                               max_enemy_bullets, enemy_bullet_cfg,
                               bullet_active, bullet_x, bullet_y,
                               bullet_velocity_x);
        draw_player(window, player_sprite, player_x, player_y, player_facing,
                    player_scale, player_width);

        window.display();

        // Clock-based cap instead of setFramerateLimit.
        while (frame_clock.getElapsedTime().asMilliseconds() < frame_ms) {
        }
        frame_clock.restart();
    }

    free_level(lvl, height);

    return 0;
}
