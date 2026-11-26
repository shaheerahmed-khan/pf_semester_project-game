#ifndef PHYSICS_H
#define PHYSICS_H

// Pushes a vertical speed toward terminal velocity. The gravity and terminal
// values are supplied by main so this file owns no global constants.
void apply_gravity(float &vertical_speed, float gravity, float terminal_velocity);

// True when the box overlaps any solid tile in the level grid.
bool collides_with_solid(char **level_grid, int level_height, int level_width,
                         float box_x, float box_y, float box_width,
                         float box_height, int cell_size,
                         float collision_eps);

// Moves the box one step, stopping it at solid tiles. Sets is_on_ground when a
// downward move lands on a floor.
void move_with_collision(char **level_grid, int level_height, int level_width,
                         float &box_x, float &box_y, float box_width,
                         float box_height, float &horizontal_speed,
                         float &vertical_speed, bool &is_on_ground,
                         int cell_size, float collision_eps);

#endif
