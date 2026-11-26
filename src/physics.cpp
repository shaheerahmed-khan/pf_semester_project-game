#include "physics.h"

#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

#include "config.h"
#include "level.h"
void apply_gravity(float& vertical_speed) {
    vertical_speed += GRAVITY;
    if (vertical_speed > TERMINAL_VELOCITY) {
        vertical_speed = TERMINAL_VELOCITY;
    }
}

// true when two rectangles overlap
bool rects_overlap(float first_x, float first_y, float first_width, float first_height,
                   float second_x, float second_y, float second_width, float second_height) {
    return first_x < second_x + second_width && first_x + first_width > second_x &&
           first_y < second_y + second_height && first_y + first_height > second_y;
}

// true when the box is inside any solid tile
bool collides_with_solid(char** level_grid, int level_height_in_tiles, int level_width_in_tiles,
                         float box_x, float box_y, float box_width, float box_height) {
    int left_column = (int)std::floor(box_x / CELL_SIZE);
    int right_column = (int)std::floor((box_x + box_width - COLLISION_EPS) / CELL_SIZE);
    int top_row = (int)std::floor(box_y / CELL_SIZE);
    int bottom_row = (int)std::floor((box_y + box_height - COLLISION_EPS) / CELL_SIZE);

    for (int row = top_row; row <= bottom_row; row++) {
        for (int col = left_column; col <= right_column; col++) {
            if (is_solid(level_grid, level_height_in_tiles, level_width_in_tiles, row, col)) {
                return true;
            }
        }
    }
    return false;
}

void move_with_collision(char** level_grid, int level_height_in_tiles, int level_width_in_tiles,
                         float& box_x, float& box_y, float box_width, float box_height,
                         float& horizontal_speed, float& vertical_speed, bool& is_on_ground) {
    is_on_ground = false;

    // ----- sideways -----
    box_x += horizontal_speed;

    if (collides_with_solid(level_grid, level_height_in_tiles, level_width_in_tiles, box_x, box_y,
                            box_width, box_height)) {
        if (horizontal_speed > 0) {
            int wall_column = (int)std::floor((box_x + box_width) / CELL_SIZE);
            box_x = wall_column * CELL_SIZE - box_width - COLLISION_EPS;
        } else if (horizontal_speed < 0) {
            int wall_column = (int)std::floor(box_x / CELL_SIZE);
            box_x = (wall_column + 1) * CELL_SIZE + COLLISION_EPS;
        }

        horizontal_speed = 0;
    }

    // ----- up and down -----
    box_y += vertical_speed;

    if (collides_with_solid(level_grid, level_height_in_tiles, level_width_in_tiles, box_x, box_y,
                            box_width, box_height)) {
        if (vertical_speed > 0) {
            int floor_row = (int)std::floor((box_y + box_height) / CELL_SIZE);
            box_y = floor_row * CELL_SIZE - box_height - COLLISION_EPS;
            is_on_ground = true;
        } else if (vertical_speed < 0) {
            int ceiling_row = (int)std::floor(box_y / CELL_SIZE);
            box_y = (ceiling_row + 1) * CELL_SIZE + COLLISION_EPS;
        }

        vertical_speed = 0;
    }
}
