#include "physics.h"

#include "level.h"

#include <cmath>

void apply_gravity(float &vertical_speed, float gravity, float terminal_velocity) {
    vertical_speed += gravity;
    if (vertical_speed > terminal_velocity) {
        vertical_speed = terminal_velocity;
    }
}

bool collides_with_solid(char **level_grid, int level_height, int level_width,
                         float box_x, float box_y, float box_width,
                         float box_height, int cell_size, float collision_eps) {
    int left_column = (int)std::floor(box_x / cell_size);
    int right_column = (int)std::floor((box_x + box_width - collision_eps) / cell_size);
    int top_row = (int)std::floor(box_y / cell_size);
    int bottom_row = (int)std::floor((box_y + box_height - collision_eps) / cell_size);

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
            int wall_column = (int)std::floor((box_x + box_width) / cell_size);
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
            int floor_row = (int)std::floor((box_y + box_height) / cell_size);
            box_y = floor_row * cell_size - box_height - collision_eps;
            is_on_ground = true;
        } else if (vertical_speed < 0) {
            int ceiling_row = (int)std::floor(box_y / cell_size);
            box_y = (ceiling_row + 1) * cell_size + collision_eps;
        }

        vertical_speed = 0;
    }
}
