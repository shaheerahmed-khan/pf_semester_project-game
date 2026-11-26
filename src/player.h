#ifndef player
#define player .h
#include <SFML/Graphics.hpp>

#include "config.h"
// update the player position and velocity based on input and physics
void update_player(float& player_x, float& player_y, char** lvl, float& velocity_x,
                   float& velocity_y, bool on_Ground, int& facing, float speed_Multiplier,
                   int height, int width);
// reset the player position and velocity to the starting position
void respawn_player(float& player_x, float& player_y, float& velocity_x, float& velocity_y,
                    float start_x, float start_y);
// check if the player has fallen off the map (y position is greater than the level height)
bool player_fell_of_map(float y, int height);
// draw the player sprite at the current position and facing direction
void draw_player(sf ::RenderWindow& window, sf ::Sprite& sprite, float player_x, float player_y,
                 int facing);

#endif
