#ifndef physics
#define physics .h
#include <SFML/Graphics.hpp>

#include "config.h"
// make thes player fall faster each frame
void apply_gravity(float& vertical_speed);

bool move_with_collison(char** level_grid, int level_height_in_tiles, int level_with_in_tiles,
                        float box_x, float box_y, float box_width, float box_height,
                        float& velocity_x, float& velocity_y, bool& on_Ground);
void move_with_collision(char** level_grid, int level_height_in_tiles, int level_width_in_tiles,
                         float& box_x, float& box_y, float box_width, float box_height,
                         float& horizontal_speed, float& vertical_speed, bool& is_on_ground)

    bool rectangles_overlap(float x1, float y1, float width1, float height1, float x2, float y2,
                            float width2, float height2);

#endif
