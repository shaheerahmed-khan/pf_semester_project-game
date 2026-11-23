#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>

#include "config.h"
#include "enemies.h"
#include "level.h"
// #include "physics.h"
#include "player.h"
#include "powerups.h"
#include "scores.h"
// #include "weapons.h"

using namespace sf;
using namespace std;

int screen_x = 1136;
int screen_y = 896;

void display_level(RenderWindow &window, char **lvl, Sprite &bgSprite,
                   Sprite &blockSprite, const int height, const int width,
                   const int cell_size);
char tile_at(char **lvl, int row, int col, const int height, const int width);
void player_gravity(char **lvl, float &velocityY, bool &onGround,
                    const float gravity, const float terminal_Velocity,
                    float &player_x, float &player_y, const int cell_size,
                    const int Pheight, const int Pwidth, const int height,
                    const int width);

int main() {

  RenderWindow window(VideoMode(screen_x, screen_y), "VICE: Project Doom",
                      Style::Resize);
  window.setFramerateLimit(60);

  // level specifics
  const int cell_size = 64;
  const int height = 14;
  const int width = 18;
  char **lvl;

  // level and background textures and sprites
  Texture bgTex;
  Sprite bgSprite;
  Texture blockTexture;
  Sprite blockSprite;

  if (!bgTex.loadFromFile("Data/bg.png")) {
    cout << "Could not load Data/bg.png" << endl;
    return 1;
  }
  bgSprite.setTexture(bgTex);
  bgSprite.setPosition(0, 0);

  if (!blockTexture.loadFromFile("Data/block1.png")) {
    cout << "Could not load Data/block1.png" << endl;
    return 1;
  }
  blockSprite.setTexture(blockTexture);

  // Music initialisation
  Music lvlMusic;

  if (lvlMusic.openFromFile("Data/mus.ogg")) {
    lvlMusic.setVolume(20);
    lvlMusic.setLoop(true);
    lvlMusic.play();
  } else {
    cout << "Could not load Data/mus.ogg" << endl;
  }

  // player data
  float player_x = 780;
  float player_y = 60;

  const float gravity = 1;
  const float terminal_Velocity = 20;

  Texture PlayerTexture;
  Sprite PlayerSprite;

  bool onGround = false;
  float velocityY = 0;

  const int PlayerHeight = 96;
  const int PlayerWidth = 54;

  if (!PlayerTexture.loadFromFile("Data/player.png")) {
    cout << "Could not load Data/player.png" << endl;
    return 1;
  }
  PlayerSprite.setTexture(PlayerTexture);
  PlayerSprite.setScale(3, 3);
  PlayerSprite.setPosition(player_x, player_y);

  // enemy picture (a plain white pixel for now, replace with a real sprite
  // later)
  Image enemyImage;
  Texture enemyTexture;
  Sprite enemySprite;

  enemyImage.create(1, 1, Color::White);
  enemyTexture.loadFromImage(enemyImage);
  enemySprite.setTexture(enemyTexture);

  // enemy data
  bool enemyActive[MAX_ENEMIES];
  int enemyType[MAX_ENEMIES];
  float enemyX[MAX_ENEMIES];
  float enemyY[MAX_ENEMIES];
  float enemyVX[MAX_ENEMIES];
  float enemyVY[MAX_ENEMIES];
  int enemyHP[MAX_ENEMIES];
  int enemyDir[MAX_ENEMIES];
  int enemyState[MAX_ENEMIES];
  int enemyTimer[MAX_ENEMIES];

  init_enemies(enemyActive, enemyType, enemyX, enemyY, enemyVX, enemyVY,
               enemyHP, enemyDir, enemyState, enemyTimer);

  int playerHealth = PLAYER_START_HEALTH;
  int invulnTimer = 0;

  // creating level array
  lvl = new char *[height];
  for (int i = 0; i < height; i += 1) {
    lvl[i] = new char[width];
    for (int j = 0; j < width; j++) {
      lvl[i][j] = '.';
    }
  }

  for (int j = 0; j < width; j += 1) {
    lvl[13][j] = '#';
  }

  // one crawler to start with
  spawn_enemy(enemyActive, enemyType, enemyX, enemyY, enemyVX, enemyVY, enemyHP,
              enemyDir, enemyState, enemyTimer, ENEMY_CRAWLER, 100, 100);

  Event ev;
  // main loop
  while (window.isOpen()) {

    while (window.pollEvent(ev)) {
      if (ev.type == Event::Closed) {
        window.close();
      }

      // pressing escape to close
      if (ev.type == Event::KeyPressed && ev.key.code == Keyboard::Escape) {
        window.close();
      }

      // press 1 to spawn a Spiked Crawler
      if (ev.type == Event::KeyPressed && ev.key.code == Keyboard::Num1) {
        spawn_enemy(enemyActive, enemyType, enemyX, enemyY, enemyVX, enemyVY,
                    enemyHP, enemyDir, enemyState, enemyTimer, ENEMY_CRAWLER,
                    100, 100);
      }
    }

    window.clear();

    display_level(window, lvl, bgSprite, blockSprite, height, width, cell_size);
    update_enemies(enemyActive, enemyType, enemyX, enemyY, enemyVX, enemyVY,
                   enemyHP, enemyDir, enemyState, enemyTimer, lvl, height,
                   width, cell_size);
    draw_enemies(window, enemySprite, enemyActive, enemyType, enemyX, enemyY);
    player_gravity(lvl, velocityY, onGround, gravity, terminal_Velocity,
                   player_x, player_y, cell_size, PlayerHeight, PlayerWidth,
                   height, width);
    PlayerSprite.setPosition(player_x, player_y);
    window.draw(PlayerSprite);

    // enemy touching the player
    if (invulnTimer > 0) {
      invulnTimer--;
    }
    int touchDamage =
        enemy_damage_on_contact(enemyActive, enemyType, enemyX, enemyY,
                                player_x, player_y, PlayerWidth, PlayerHeight);
    if (touchDamage > 0 && invulnTimer == 0) {
      playerHealth -= touchDamage;
      invulnTimer = PLAYER_VULN_FRAMES;
      cout << "Player health: " << playerHealth << endl;
      if (playerHealth <= 0) {
        cout << "Player died, health reset" << endl;
        playerHealth = PLAYER_START_HEALTH;
      }
    }

    window.display();
  }

  // stopping music and deleting level array
  lvlMusic.stop();
  for (int i = 0; i < height; i++) {
    delete[] lvl[i];
  }
  delete[] lvl;

  return 0;
}

void display_level(RenderWindow &window, char **lvl, Sprite &bgSprite,
                   Sprite &blockSprite, const int height, const int width,
                   const int cell_size) {
  window.draw(bgSprite);

  for (int i = 0; i < height; i += 1) {
    for (int j = 0; j < width; j += 1) {

      if (lvl[i][j] == '#') {
        blockSprite.setPosition(j * cell_size, i * cell_size);
        window.draw(blockSprite);
      }
    }
  }
}

// safe way to read the level, never goes outside the array
char tile_at(char **lvl, int row, int col, const int height, const int width) {
  if (row < 0) {
    return '.';
  }
  if (row >= height || col < 0 || col >= width) {
    return '#';
  }
  return lvl[row][col];
}

void player_gravity(char **lvl, float &velocityY, bool &onGround,
                    const float gravity, const float terminal_Velocity,
                    float &player_x, float &player_y, const int cell_size,
                    const int Pheight, const int Pwidth, const int height,
                    const int width) {
  float next_y = player_y + velocityY;

  int feet_row = (int)(next_y + Pheight) / cell_size;
  int left_col = (int)(player_x) / cell_size;
  int mid_col = (int)(player_x + Pwidth / 2) / cell_size;
  int right_col = (int)(player_x + Pwidth) / cell_size;

  char bottom_left_down = tile_at(lvl, feet_row, left_col, height, width);
  char bottom_mid_down = tile_at(lvl, feet_row, mid_col, height, width);
  char bottom_right_down = tile_at(lvl, feet_row, right_col, height, width);

  if (bottom_left_down == '#' || bottom_mid_down == '#' ||
      bottom_right_down == '#') {
    // stand exactly on top of the block
    player_y = feet_row * cell_size - Pheight;
    velocityY = 0;
    onGround = true;
  } else {
    player_y = next_y;
    onGround = false;
    velocityY += gravity;
    if (velocityY >= terminal_Velocity) {
      velocityY = terminal_Velocity;
    }
  }
}
