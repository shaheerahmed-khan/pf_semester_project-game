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
    step 2: making a 2d array and filling it with empty cells
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

    /*
    Step 3: filling the 2d array with the level data from the file
    copy each character into the array, restarting
    */
   //reset the "reach the end" flag
    file.clear();
    //move back to beginning of file
    file.seekg(0);

    for (int i=0; i<height; i++)
    {
            getline(file, line);
            for (int j=0; j<width && j<(int)line.length(); j++)
            {
                    level[i][j]=line[j];
                    if (line[j]=='P')
                        {
                          start_x=j*64;
                          start_y=(i+1)*64-96;
                          level[i][j]='.';
                        }
            }
    }





      return level; 
}




/*
step4: freeing the 2d array when the level is no longer needed
new (heap)--->delete (new char height and new char width)
simpler words: loadlevel uses new to create arrays in two layers:
one block for number of rows, and one block for each row (columns)
since we need to clean memory after using it in heap, we use delete
the order matters here very much, as first we delete each row first,
then we delete the array of row pointers itself. 
if we do it in reverse order, we will lose the pointer to each row and will not be able to delete them, causing memory leak.
*/

void free_level(char** lvl, int height)
{
    for (int i=0; i<height; i++)
    {
        //delete each row
        delete[] lvl[i];
    }
    //delete the entire array of row pointers itself
    delete[] lvl;
}



bool is_solid(char** lvl, int height, int width, int row, int col)
{
        //preventing reading outside the map
        //left and right count as walls, hence true
        //top and bottom do not, hence false (they fall into pit or die or whatever)
        if (col<0 || col>=width)
        {
                return true;
        }
        if (row<0 || row>=height)
        {
                return false;
        }
        
        //checking whether a particular tile (1 tile=64x64 pixel) is solid or not
        if (lvl[row][col]=='#')
        {
                return true;
        }
        else
        {
                return false;
        }

}


void draw_level(RenderWindow &window, char** lvl, Sprite& bgSprite, Sprite &blockSprite, int height, int width, int cell_size)
{
        window.draw(bgSprite);

        for (int i = 0; i < height; i += 1) 
        {
                for (int j = 0; j < width; j += 1) 
                {

                        if (lvl[i][j] == '#') 
                        {
                                blockSprite.setPosition(j*cell_size, i*cell_size);
                                window.draw(blockSprite);
                        }
                }
        }
}        

