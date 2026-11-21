#ifndef CONFIG_H
#define CONFIG_H

const int MAX_ENEMIES = 20;

const int ENEMY_CRAWLER = 0;
const int ENEMY_JUMPER = 1;
const int ENEMY_INSECT = 2;
const int ENEMY_LEOPARD = 3;
const int ENEMY_SHOOTER = 4;
const int ENEMY_TRIBAL = 5;
const int ENEMY_FISH = 6;

const float ENEMY_GRAVITY = 1;
const float ENEMY_TERMINAL_VELOCITY = 20;

const int CRAWLER_HP = 20;
const float CRAWLER_SPEED = 2;
const int CRAWLER_WIDTH = 56;
const int CRAWLER_HEIGHT = 40;
const int CRAWLER_DAMAGE = 10;
const int CRAWLER_SCORE = 100;

const int PLAYER_START_HEALTH = 100;
const int PLAYER_VULN_FRAMES = 60;

#endif
