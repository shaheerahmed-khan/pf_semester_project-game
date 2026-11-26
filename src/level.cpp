#include "level.h"

#include <cstdio>
#include <cstring>

using namespace sf;

char **load_level(const char *path, int &height, int &width, float &start_x,
                  float &start_y, int cell_size, int player_height) {
    FILE *file = fopen(path, "r");
    if (file == nullptr) {
        return nullptr;
    }

    // First pass: count rows and find the widest row.
    char line[512];
    int rows = 0;
    int columns = 0;

    while (fgets(line, (int)sizeof(line), file) != nullptr) {
        int length = (int)strlen(line);
        while (length > 0 &&
               (line[length - 1] == '\n' || line[length - 1] == '\r')) {
            length--;
        }

        if (length > columns) {
            columns = length;
        }
        rows++;
    }

    if (rows <= 0 || columns <= 0) {
        fclose(file);
        return nullptr;
    }

    // Second pass: allocate the grid and copy the characters in.
    char **level = new char *[rows];
    for (int i = 0; i < rows; i++) {
        level[i] = new char[columns];
        for (int j = 0; j < columns; j++) {
            level[i][j] = '.';
        }
    }

    rewind(file);

    for (int i = 0; i < rows; i++) {
        if (fgets(line, (int)sizeof(line), file) == nullptr) {
            line[0] = '\0';
        }

        int length = (int)strlen(line);
        while (length > 0 &&
               (line[length - 1] == '\n' || line[length - 1] == '\r')) {
            length--;
        }

        for (int j = 0; j < columns; j++) {
            if (j >= length) {
                continue;
            }

            level[i][j] = line[j];

            if (line[j] == 'P') {
                start_x = (float)(j * cell_size);
                start_y = (float)((i + 1) * cell_size - player_height);
                level[i][j] = '.';
            }
        }
    }

    fclose(file);

    height = rows;
    width = columns;
    return level;
}

void free_level(char **lvl, int height) {
    if (lvl == nullptr) {
        return;
    }

    for (int i = 0; i < height; i++) {
        delete[] lvl[i];
    }
    delete[] lvl;
}

bool is_solid(char **lvl, int height, int width, int row, int col) {
    // Left and right screen edges are walls; above and below the map are not.
    if (col < 0 || col >= width) {
        return true;
    }
    if (row < 0 || row >= height) {
        return false;
    }

    return lvl[row][col] == '#';
}

void draw_level(RenderWindow &window, char **lvl, Sprite &bgSprite,
                Sprite &blockSprite, int height, int width, int cell_size) {
    window.draw(bgSprite);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (lvl[i][j] == '#') {
                blockSprite.setPosition((float)(j * cell_size),
                                        (float)(i * cell_size));
                window.draw(blockSprite);
            }
        }
    }
}
