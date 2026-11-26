#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>

// Updates the player from the two permitted inputs (Right and Space) and the
// world. Every number comes from main, so there are no hidden constants here.
void update_player(float &player_x, float &player_y, float &velocity_x,
                   float &velocity_y, bool &on_ground, int &facing,
                   bool move_right, bool jump_pressed, float speed,
                   float jump_speed, int player_width, int player_height,
                   char **lvl, int height, int width, int cell_size,
                   float gravity, float terminal_velocity,
                   float collision_eps);

// Resets the player to the level start.
void respawn_player(float &player_x, float &player_y, float &velocity_x,
                    float &velocity_y, bool &on_ground, float start_x,
                    float start_y);

// True when the player has fallen below the bottom of the level.
bool player_fell_off_map(float y, int height, int cell_size);

// Draws the player, mirroring the sprite when it faces left.
void draw_player(sf::RenderWindow &window, sf::Sprite &sprite, float player_x,
                 float player_y, int facing, int player_scale,
                 int player_width);

#endif
