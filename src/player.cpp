#include "player.h"

#include "physics.h"

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

void draw_player(sf::RenderWindow &window, sf::Sprite &sprite, float player_x,
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
