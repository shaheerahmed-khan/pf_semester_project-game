#ifndef enemies_h
#define enemies_h

#include "config.h"
#include <SFML/Graphics.hpp>


void init_enemies(bool *active, int *type, float *x, float *y,
                  float *velocity_x, float *velocity_y, int *health,
                  int *direction, int *state, int *timer);

int spawn_enemy(bool *active, int *type, float *x, float *y, float *velocity_x,
                float *velocity_y, int *health, int *direction, int *state,
                int *timer, int newType, float new_x, float new_y);

void update_enemies(bool *active, int *type, float *x, float *y,
                    float *velocity_x, float *velocity_y, int *health,
                    int *direction, int *state, int *timer, char **lvl,
                    const int height, const int width, const int cell_size);

void draw_enemies(sf::RenderWindow &window, bool *active, int *type, float *x,
                  float *y);

bool damage_enemy(bool *active, int *health, int index, int amount);

int enemy_damage_on_contact(bool *active, int *type, float *x, float *y,
                            float player_x, float player_y, float player_w,
                            float player_h);

#endif
