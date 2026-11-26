#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>

#include "enemies.h"
#include "level.h"
#include "physics.h"
#include "player.h"
#include "powerups.h"
#include "scores.h"
// #include "weapons.h"

using namespace sf;
using namespace std;

int main() {

  // ---------- all the game numbers ----------
  // global variables AND global constants are banned, so every number the
  // game uses lives here in main and gets passed to the functions that need it

  // window
  const int screen_x = 1136;
  const int screen_y = 896;

  // world
  const int cell_size = 64;
  const float gravity = 1;
  const float terminal_velocity = 20;
  const float collision_eps = 0.01f;

  // player (the picture is 18 x 32 pixels, drawn at scale 3)
  const int playerStartHealth = 100;
  const int playerVulnFrames = 60;
  const float playerSpeed = 5;
  const float jumpSpeed = 20;
  const int playerScale = 3;
  const int playerWidth = 54;
  const int playerHeight = 96;

  // enemies. the type numbers are the row numbers of the table below, and
  // they are also the order of the enemy pictures
  const int maxEnemies = 20;
  const int maxEnemyBullets = 10;

  const int ENEMY_CRAWLER = 0;
  const int ENEMY_JUMPER = 1;
  const int ENEMY_INSECT = 2;
  const int ENEMY_LEOPARD = 3;
  const int ENEMY_SHOOTER = 4;
  const int ENEMY_TRIBAL = 5;
  const int ENEMY_FISH = 6;

  // enemy gravity is the same as the player's, so it uses the numbers above.
  // one row per enemy type, the columns are explained in enemies.h:
  //   width, height, hp, damage, start speed, score, then the columns that
  //   only some types use (unused ones are 0)
  float enemyCfg[ENEMY_FISH + 1][14] = {
      // crawler
      {56, 40, 20, 10, 2, 100, 0, 0, 0, 0, 0, 0, 0, 0},
      // jumper: speed 4, jump speed 16, jump delay 60, sight range 500
      {48, 64, 30, 10, 0, 150, 4, 16, 60, 500, 0, 0, 0, 0},
      // insect: speed 2.5, vertical speed 1.5, wave speed 0.1, wave size 3
      {40, 32, 10, 5, 0, 100, 2.5f, 1.5f, 0.1f, 3, 0, 0, 0, 0},
      // leopard: patrol speed 1.5, charge speed 7, sight 400 x 64,
      // charge 45 frames, recover 60 frames
      {80, 48, 40, 20, 1.5f, 300, 7, 400, 64, 45, 60, 0, 0, 0},
      // shooter: sight 500 x 96, shoots every 90 frames
      {48, 80, 20, 10, 0, 200, 500, 96, 90, 0, 0, 0, 0, 0},
      // tribal: walk 1.5, lunge 6, sight 350 x 96, attack range 90,
      // windup 30, lunge 15, cooldown 60 frames
      {56, 96, 50, 20, 0, 400, 1.5f, 6, 350, 96, 90, 30, 15, 60},
      // fish: jump speed 20, leap delay 90 frames
      {32, 40, 10, 15, 0, 150, 20, 90, 0, 0, 0, 0, 0, 0}};

  // shooter bullets: speed, width, height, damage
  float enemyBulletCfg[4] = {6, 16, 8, 10};

  RenderWindow window(VideoMode(screen_x, screen_y), "VICE: Project Doom",
                      Style::Resize);
  window.setFramerateLimit(60);

  // ---------- level ----------
  // the level comes from the text file. if the file can't be opened we fall
  // back to a flat test floor so the game still runs
  int height = 14;
  int width = 18;
  float start_x = 780;
  float start_y = 60;

  char **lvl = load_level("../levels/level1.txt", height, width, start_x,
                          start_y, cell_size, playerHeight);

  if (lvl == nullptr) {
    cout << "Could not load ../levels/level1.txt, using the test floor" << endl;

    height = 14;
    width = 18;
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
  }

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

  // ---------- music ----------
  Music lvlMusic;

  if (lvlMusic.openFromFile("Data/mus.ogg")) {
    lvlMusic.setVolume(20);
    lvlMusic.setLoop(true);
    lvlMusic.play();
  } else {
    cout << "Could not load Data/mus.ogg" << endl;
  }

  // ---------- player ----------
  float player_x = start_x;
  float player_y = start_y;
  float velocityX = 0;
  float velocityY = 0;
  bool onGround = false;
  int facing = 1;

  Texture PlayerTexture;
  Sprite PlayerSprite;

  if (!PlayerTexture.loadFromFile("Data/player.png")) {
    cout << "Could not load Data/player.png" << endl;
    return 1;
  }
  PlayerSprite.setTexture(PlayerTexture);

  int playerHealth = playerStartHealth;
  int invulnTimer = 0;

  // ---------- enemy pictures ----------
  // one texture per enemy type, stored at the index of its ENEMY_* value so
  // the draw code can find it with just the type number.
  // paths are relative to the data folder, same as bg.png and player.png
  const char *enemyFiles[ENEMY_FISH + 1];
  enemyFiles[ENEMY_CRAWLER] = "Data/sprites/enemies/spiked_crawler.png";
  enemyFiles[ENEMY_JUMPER] = "Data/sprites/enemies/blue_jumper.png";
  enemyFiles[ENEMY_INSECT] = "Data/sprites/enemies/flying_insect.png";
  enemyFiles[ENEMY_LEOPARD] = "Data/sprites/enemies/leopard.png";
  enemyFiles[ENEMY_SHOOTER] = "Data/sprites/enemies/green_shooter.png";
  enemyFiles[ENEMY_TRIBAL] = "Data/sprites/enemies/tribal_warrior.png";
  enemyFiles[ENEMY_FISH] = "Data/sprites/enemies/leaping_fish.png";

  Texture enemyTextures[ENEMY_FISH + 1];
  for (int i = 0; i <= ENEMY_FISH; i++) {
    if (!enemyTextures[i].loadFromFile(enemyFiles[i])) {
      cout << "Could not load " << enemyFiles[i] << endl;
      return 1;
    }
  }

  // the shooter's bullet picture
  Texture bulletTexture;
  if (!bulletTexture.loadFromFile("Data/sprites/fx/enemy_bolt.png")) {
    cout << "Could not load Data/sprites/fx/enemy_bolt.png" << endl;
    return 1;
  }

  // one sprite is reused for every enemy and bullet, the draw functions swap
  // its texture each time
  Sprite enemySprite;

  // ---------- enemy data ----------
  bool enemyActive[maxEnemies];
  int enemyType[maxEnemies];
  float enemyX[maxEnemies];
  float enemyY[maxEnemies];
  float enemyVX[maxEnemies];
  float enemyVY[maxEnemies];
  int enemyHP[maxEnemies];
  int enemyDir[maxEnemies];
  int enemyState[maxEnemies];
  int enemyTimer[maxEnemies];

  init_enemies(maxEnemies, enemyActive, enemyType, enemyX, enemyY, enemyVX,
               enemyVY, enemyHP, enemyDir, enemyState, enemyTimer);

  // shooter bullet data
  bool bulletActive[maxEnemyBullets];
  float bulletX[maxEnemyBullets];
  float bulletY[maxEnemyBullets];
  float bulletVX[maxEnemyBullets];

  init_enemy_projectiles(maxEnemyBullets, bulletActive, bulletX, bulletY,
                         bulletVX);

  // one crawler to start with
  spawn_enemy(maxEnemies, enemyCfg, enemyActive, enemyType, enemyX, enemyY,
              enemyVX, enemyVY, enemyHP, enemyDir, enemyState, enemyTimer,
              ENEMY_CRAWLER, 100, 100);

  Event ev;
  // ---------- main loop ----------
  while (window.isOpen()) {

    while (window.pollEvent(ev)) {
      if (ev.type == Event::Closed) {
        window.close();
      }

      // pressing escape to close
      if (ev.type == Event::KeyPressed && ev.key.code == Keyboard::Escape) {
        window.close();
      }

      // testing only: press 1 to 7 to spawn an enemy (remove before submitting,
      // key presses through events are not on the allowed SFML list):
      // 1 Crawler, 2 Jumper, 3 Insect, 4 Leopard, 5 Shooter, 6 Tribal, 7 Fish
      if (ev.type == Event::KeyPressed && ev.key.code >= Keyboard::Num1 &&
          ev.key.code <= Keyboard::Num7) {
        int newType = ev.key.code - Keyboard::Num1;
        float spawn_x = 100;
        float spawn_y = 100;

        if (newType == ENEMY_INSECT)
          spawn_y = 300;
        if (newType == ENEMY_FISH) {
          spawn_x = 600;
          spawn_y = 700;
        }

        spawn_enemy(maxEnemies, enemyCfg, enemyActive, enemyType, enemyX,
                    enemyY, enemyVX, enemyVY, enemyHP, enemyDir, enemyState,
                    enemyTimer, newType, spawn_x, spawn_y);
      }
    }

    // player input, gravity and collisions all happen in here
    update_player(player_x, player_y, velocityX, velocityY, onGround, facing,
                  1.0f, lvl, height, width, playerSpeed, jumpSpeed, playerWidth,
                  playerHeight, gravity, terminal_velocity, cell_size,
                  collision_eps);

    // fell into a pit, back to the start
    if (player_fell_off_map(player_y, height, cell_size)) {
      respawn_player(player_x, player_y, velocityX, velocityY, start_x,
                     start_y);
    }

    // the enemies aim at the middle of the player
    float target_x = player_x + playerWidth / 2.0f;
    float target_y = player_y + playerHeight / 2.0f;

    update_enemies(maxEnemies, enemyCfg, maxEnemyBullets, enemyBulletCfg,
                   enemyActive, enemyType, enemyX, enemyY, enemyVX, enemyVY,
                   enemyHP, enemyDir, enemyState, enemyTimer, bulletActive,
                   bulletX, bulletY, bulletVX, target_x, target_y, lvl, height,
                   width, cell_size, gravity, terminal_velocity);
    update_enemy_projectiles(maxEnemyBullets, enemyBulletCfg, bulletActive,
                             bulletX, bulletY, bulletVX, lvl, height, width,
                             cell_size);

    // enemy or enemy bullet touching the player
    if (invulnTimer > 0) {
      invulnTimer--;
    }
    int touchDamage = enemy_damage_on_contact(
        maxEnemies, enemyCfg, enemyActive, enemyType, enemyX, enemyY, player_x,
        player_y, playerWidth, playerHeight);
    if (touchDamage == 0) {
      touchDamage = enemy_projectile_damage_on_contact(
          maxEnemyBullets, enemyBulletCfg, bulletActive, bulletX, bulletY,
          player_x, player_y, playerWidth, playerHeight);
    }
    if (touchDamage > 0 && invulnTimer == 0) {
      playerHealth -= touchDamage;
      invulnTimer = playerVulnFrames;
      cout << "Player health: " << playerHealth << endl;
      if (playerHealth <= 0) {
        cout << "Player died, health reset" << endl;
        playerHealth = playerStartHealth;
        respawn_player(player_x, player_y, velocityX, velocityY, start_x,
                       start_y);
      }
    }

    // ---------- drawing ----------
    window.clear();

    draw_level(window, lvl, bgSprite, blockSprite, height, width, cell_size);
    draw_enemies(window, enemySprite, enemyTextures, maxEnemies, enemyCfg,
                 enemyActive, enemyType, enemyX, enemyY, enemyDir);
    draw_enemy_projectiles(window, enemySprite, bulletTexture, maxEnemyBullets,
                           enemyBulletCfg, bulletActive, bulletX, bulletY,
                           bulletVX);
    draw_player(window, PlayerSprite, player_x, player_y, facing, playerScale,
                playerWidth);

    window.display();
  }

  // stopping music and freeing the level array
  lvlMusic.stop();
  free_level(lvl, height);

  return 0;
}
