#ifndef level_h
#define level_h

#include <SFML/Graphics.hpp>

char** load_level(const char* path, int& height, int& width, float& start_x, float& start_y);

void free_level(char** lvl, int height);

bool is_solid(char** lvl, int height, int width, int row, int col);

void draw_level(sf::RenderWindow &window, char** lvl, sf::Sprite& bgSprite, sf::Sprite &blockSprite, int height, int width, int cell_size);









#endif
