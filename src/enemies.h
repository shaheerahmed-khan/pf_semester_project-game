#ifndef ENEMIES_H
#define ENEMIES_H

#include <SFML/Graphics.hpp>

// Enemy rows come from main(). Columns:
//   0 width, 1 height, 2 hp, 3 contact damage, 4 walk speed, 5 score,
//   6..13 type-specific values (see the table in main.cpp).
// Bullet columns: 0 speed, 1 width, 2 height, 3 damage.

void init_enemies(int max_enemies, bool *active, int *type, float *x, float *y,
                  float *velocity_x, float *velocity_y, int *hp,
                  int *direction, int *state, int *timer);

int spawn_enemy(int max_enemies, int enemy_type_count, float cfg[][14],
                bool *active, int *type, float *x, float *y,
                float *velocity_x, float *velocity_y, int *hp, int *direction,
                int *state, int *timer, int new_type, float new_x,
                float new_y);

void update_enemies(int max_enemies, int enemy_type_count, float cfg[][14],
                    int max_bullets, float bullet_cfg[4], bool *active,
                    int *type, float *x, float *y, float *velocity_x,
                    float *velocity_y, int *direction, int *state, int *timer,
                    bool *bullet_active, float *bullet_x, float *bullet_y,
                    float *bullet_velocity_x, float target_x, float target_y,
                    char **lvl, int height, int width, int cell_size,
                    float gravity, float terminal_velocity);

// enemy_textures has one texture per enemy type, indexed by the type value.
void draw_enemies(sf::RenderWindow &window, sf::Sprite &enemy_sprite,
                  sf::Texture *enemy_textures, int max_enemies,
                  int enemy_type_count, float cfg[][14], bool *active,
                  int *type, float *x, float *y, int *direction);

bool damage_enemy(int max_enemies, bool *active, int *hp, int index,
                  int amount);

int enemy_damage_on_contact(int max_enemies, int enemy_type_count,
                            float cfg[][14], bool *active, int *type,
                            float *x, float *y, float player_x,
                            float player_y, float player_w, float player_h);

void init_enemy_projectiles(int max_bullets, bool *bullet_active,
                            float *bullet_x, float *bullet_y,
                            float *bullet_velocity_x);

void update_enemy_projectiles(int max_bullets, float bullet_cfg[4],
                              bool *bullet_active, float *bullet_x,
                              float *bullet_y, float *bullet_velocity_x,
                              char **lvl, int height, int width,
                              int cell_size);

void draw_enemy_projectiles(sf::RenderWindow &window,
                            sf::Sprite &enemy_sprite,
                            sf::Texture &bullet_texture, int max_bullets,
                            float bullet_cfg[4], bool *bullet_active,
                            float *bullet_x, float *bullet_y,
                            float *bullet_velocity_x);

int enemy_projectile_damage_on_contact(int max_bullets, float bullet_cfg[4],
                                       bool *bullet_active, float *bullet_x,
                                       float *bullet_y, float player_x,
                                       float player_y, float player_w,
                                       float player_h);

#endif
