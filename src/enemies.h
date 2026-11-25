#ifndef enemies_h
#define enemies_h

#include "config.h"
#include <SFML/Graphics.hpp>

// for enemies in general
void init_enemies(bool *active, int *type, float *x, float *y,
                  float *velocity_x, float *velocity_y, int *hp, int *direction,
                  int *state, int *timer);

int spawn_enemy(bool *active, int *type, float *x, float *y, float *velocity_x,
                float *velocity_y, int *hp, int *direction, int *state,
                int *timer, int newType, float new_x, float new_y);

void update_enemies(bool *active, int *type, float *x, float *y,
                    float *velocity_x, float *velocity_y, int *hp,
                    int *direction, int *state, int *timer, bool *bullet_active,
                    float *bullet_x, float *bullet_y, float *bullet_velocity_x,
                    float target_x, float target_y, char **lvl,
                    const int height, const int width, const int cell_size);

void draw_enemies(sf::RenderWindow &window, sf::Sprite &enemySprite,
                  bool *active, int *type, float *x, float *y);

bool damage_enemy(bool *active, int *hp, int index, int amount);

int enemy_damage_on_contact(bool *active, int *type, float *x, float *y,
                            float player_x, float player_y, float player_w,
                            float player_h);

// these are for the shooter bullets
void init_enemy_projectiles(bool *bullet_active, float *bullet_x,
                            float *bullet_y, float *bullet_velocity_x);

void update_enemy_projectiles(bool *bullet_active, float *bullet_x,
                              float *bullet_y, float *bullet_velocity_x,
                              char **lvl, const int height, const int width,
                              const int cell_size);

void draw_enemy_projectiles(sf::RenderWindow &window, sf::Sprite &enemySprite,
                            bool *bullet_active, float *bullet_x,
                            float *bullet_y);

int enemy_projectile_damage_on_contact(bool *bullet_active, float *bullet_x,
                                       float *bullet_y, float player_x,
                                       float player_y, float player_w,
                                       float player_h);

#endif
