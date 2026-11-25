#ifndef level_h
#define level_h


char** load_level(const char* path, int& height, int& width, float& start_x, float& start_y);

void free_level(char** lvl, int height);













#endif
