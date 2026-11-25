#ifndef CONFIG_H
#define CONFIG_H

// COMMON PROPERTIES
const int MAX_ENEMIES = 20;
const int MAX_ENEMY_PROJECTILES = 10;

const int ENEMY_CRAWLER = 0;
const int ENEMY_JUMPER = 1;
const int ENEMY_INSECT = 2;
const int ENEMY_LEOPARD = 3;
const int ENEMY_SHOOTER = 4;
const int ENEMY_TRIBAL = 5;
const int ENEMY_FISH = 6;

const float ENEMY_GRAVITY = 1;
const float ENEMY_TERMINAL_VELOCITY = 20;

// CRAWLERS
const int CRAWLER_HP = 20;
const float CRAWLER_SPEED = 2;
const int CRAWLER_WIDTH = 56;
const int CRAWLER_HEIGHT = 40;
const int CRAWLER_DAMAGE = 10;
const int CRAWLER_SCORE = 100;

// BLUE JUMPERS
const int JUMPER_HP = 30;
const int JUMPER_SPEED = 4;
const int JUMPER_JUMP_SPEED = 16;
const int JUMPER_WIDTH = 48;
const int JUMPER_HEIGHT = 64;
const int JUMPER_DAMAGE = 10;
const int JUMPER_SCORE = 150;
const int JUMPER_JUMP_DELAY = 60;
const float JUMPER_SIGHT_RANGE = 500;
const int JUMPER_STATE_GROUND = 0;
const int JUMPER_STATE_AIR = 1;

// FLYING INSECT
const int INSECT_HP = 10;
const float INSECT_SPEED = 2.5;
const float INSECT_VERTICAL_SPEED = 1.5;
const float INSECT_WAVE_SPEED = 0.1;
const float INSECT_WAVE_SIZE = 3;
const int INSECT_WIDTH = 40;
const int INSECT_HEIGHT = 32;
const int INSECT_DAMAGE = 5;
const int INSECT_SCORE = 100;

// LEOPARD
const int LEOPARD_HP = 40;
const float LEOPARD_PATROL_SPEED = 1.5;
const float LEOPARD_CHARGE_SPEED = 7;
const int LEOPARD_WIDTH = 80;
const int LEOPARD_HEIGHT = 48;
const int LEOPARD_DAMAGE = 20;
const int LEOPARD_SCORE = 300;
const float LEOPARD_SIGHT_RANGE = 400;
const float LEOPARD_SIGHT_HEIGHT = 64;
const int LEOPARD_CHARGE_FRAMES = 45;
const int LEOPARD_RECOVER_FRAMES = 60;
const int LEOPARD_STATE_PATROL = 0;
const int LEOPARD_STATE_CHARGE = 1;
const int LEOPARD_STATE_RECOVER = 2;

// GREEN SHOOTER
const int SHOOTER_HP = 20;
const int SHOOTER_WIDTH = 48;
const int SHOOTER_HEIGHT = 80;
const int SHOOTER_DAMAGE = 10;
const int SHOOTER_SCORE = 200;
const float SHOOTER_SIGHT_RANGE = 500;
const float SHOOTER_SIGHT_HEIGHT = 96;
const int SHOOTER_SHOOT_DELAY = 90;

// SHOOTER BULLETS
const float ENEMY_BULLET_SPEED = 6;
const int ENEMY_BULLET_WIDTH = 16;
const int ENEMY_BULLET_HEIGHT = 8;
const int ENEMY_BULLET_DAMAGE = 10;

// TRIBAL WARRIOR
const int TRIBAL_HP = 50;
const float TRIBAL_SPEED = 1.5;
const float TRIBAL_LUNGE_SPEED = 6;
const int TRIBAL_WIDTH = 56;
const int TRIBAL_HEIGHT = 96;
const int TRIBAL_DAMAGE = 20;
const int TRIBAL_SCORE = 400;
const float TRIBAL_SIGHT_RANGE = 350;
const float TRIBAL_SIGHT_HEIGHT = 96;
const float TRIBAL_ATTACK_RANGE = 90;
const int TRIBAL_WINDUP_FRAMES = 30;
const int TRIBAL_LUNGE_FRAMES = 15;
const int TRIBAL_COOLDOWN_FRAMES = 60;
const int TRIBAL_STATE_WALK = 0;
const int TRIBAL_STATE_WINDUP = 1;
const int TRIBAL_STATE_LUNGE = 2;
const int TRIBAL_STATE_COOLDOWN = 3;

// LEAPING FISH
const int FISH_HP = 10;
const float FISH_JUMP_SPEED = 20;
const int FISH_WIDTH = 32;
const int FISH_HEIGHT = 40;
const int FISH_DAMAGE = 15;
const int FISH_SCORE = 150;
const int FISH_LEAP_DELAY = 90;

// PLAYER
const int PLAYER_START_HEALTH = 100;
const int PLAYER_VULN_FRAMES = 60;

#endif
