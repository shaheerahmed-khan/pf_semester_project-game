#ifndef player
#define player .h
#include <SFML/Graphics.hpp>

#include "config.h"

void update_player(float& player_x, float& player_y, char** lvl, float& velocity_x,
                   float& velocity_y, bool on_Ground, int& facing, float speed_Multiplier,
                   int height, int width);
void respawn_player(float& player_x, float& player_y, float& velocity_x, float& velocity_y,
                    float start_x, float start_y);
bool player_fell_of_map(float y, int height);
void draw_player(sf ::RenderWindow& window, sf ::Sprite& sprite, float player_x, float player_y,
                 int facing);

#endif
