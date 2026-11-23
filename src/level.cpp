#include <fstream>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>
#include "config.h"
#include "level.h"

using namespace std;
using namespace sf;


char** load_level(const char* path, int& height, int& width, float& start_x, float& start_y)
{
        ifstream file(path);

        if (!file.is_open())
        {
                return nullptr;
        }





    /*
    Step-1: measuring the map
    this level is stored in a text file. each line of the code is a row of the level
    each character is a coloumn (or a tile)
    before we can create the 2D array, we need to know its size
    so we read through the file once and count:
    height=no of lines (rows)
    width=no of characters in a line (columns)
    this happens only once, when the level is loaded for the first time.
    all rows in a map file must be the same length, 
    so last line's length is used as width 
    */
    height=0;
    width=0;
    string line;

    while (getline(file, line))
    {
            height++;
            width=line.length();
    }
    


    /*
    Now that we know the size, we create a 2d array 
    and fill every cell with '.' (empty)
    We dont leave cells uninitialized becuse then theyll hold garbage values
    */

    char** level=new char*[height];
    for (int i=0; i<height; i++)
    {
            level[i]=new char[width];
            for (int j=0; j<width; j++)
            {
                    level[i][j]='.';        
            }
    }            

    
    








      return nullptr; 
}
