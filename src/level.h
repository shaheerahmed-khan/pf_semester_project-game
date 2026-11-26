#ifndef LEVEL_H
#define LEVEL_H

#include <SFML/Graphics.hpp>

// Loads the ASCII map with plain C file I/O and fixed char buffers, so the
// loader does not depend on std::string or std::ifstream. On success, height,
// width, start_x and start_y are filled in; on failure the function returns
// nullptr.
char **load_level(const char *path, int &height, int &width, float &start_x,
                  float &start_y, int cell_size, int player_height);

void free_level(char **lvl, int height);

bool is_solid(char **lvl, int height, int width, int row, int col);

void draw_level(sf::RenderWindow &window, char **lvl, sf::Sprite &bgSprite,
                sf::Sprite &blockSprite, int height, int width, int cell_size);

#endif
