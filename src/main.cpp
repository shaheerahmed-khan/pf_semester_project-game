#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>

#include "enemies.h"
#include "level.h"
#include "player.h"

using namespace sf;
using namespace std;

int main() {
    // ---------- every tunable number lives here ----------
    // No global variables and no global constants: main owns the numbers and
    // passes them to the functions that need them.

    // window and world
    const int screen_x = 1136;
    const int screen_y = 896;
    const int cell_size = 64;
    const float gravity = 1.0f;
    const float terminal_velocity = 20.0f;
    const float collision_eps = 0.01f;
    const int frame_ms = 16;

    // player (the picture is 18 x 32 pixels, drawn at scale 3)
    const int player_start_health = 100;
    const int player_vuln_frames = 60;
    const float player_speed = 5.0f;
    const float jump_speed = 20.0f;
    const int player_scale = 3;
    const int player_width = 54;
    const int player_height = 96;

    // enemies: the type numbers are the row numbers of the table below and
    // the order of the enemy pictures.
    const int max_enemies = 20;
    const int max_enemy_bullets = 10;
    const int enemy_type_count = 7;

    const int enemy_crawler = 0;
    const int enemy_jumper = 1;
    const int enemy_insect = 2;
    const int enemy_leopard = 3;
    const int enemy_shooter = 4;
    const int enemy_tribal = 5;
    const int enemy_fish = 6;

    // One row per enemy type. Columns:
    //   0 width, 1 height, 2 hp, 3 contact damage, 4 walk speed, 5 score,
    //   6..13 type-specific values (unused ones are 0).
    float enemy_cfg[enemy_type_count][14] = {
        // crawler
        {56, 40, 20, 10, 2, 100, 0, 0, 0, 0, 0, 0, 0, 0},
        // jumper: speed 4, jump speed 16, jump delay 60, sight range 500
        {48, 64, 30, 10, 0, 150, 4, 16, 60, 500, 0, 0, 0, 0},
        // insect: speed 2.5, vertical speed 1.5, wave speed 0.1, wave size 3
        {40, 32, 10, 5, 0, 100, 2.5f, 1.5f, 0.1f, 3, 0, 0, 0, 0},
        // leopard: patrol 1.5, charge 7, sight 400 x 64,
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
    float enemy_bullet_cfg[4] = {6, 16, 8, 10};

    RenderWindow window(VideoMode(screen_x, screen_y), "VICE: Project Doom");

    // ---------- level ----------
    int height = 14;
    int width = 18;
    float start_x = (float)cell_size;
    float start_y = (float)(13 * cell_size - player_height);

    char **lvl = load_level("levels/level1.txt", height, width, start_x,
                            start_y, cell_size, player_height);

    if (lvl == nullptr) {
        cout << "Could not load levels/level1.txt, using the test floor"
             << endl;

        height = 14;
        width = 18;
        lvl = new char *[height];
        for (int i = 0; i < height; i++) {
            lvl[i] = new char[width];
            for (int j = 0; j < width; j++) {
                lvl[i][j] = '.';
            }
        }
        for (int j = 0; j < width; j++) {
            lvl[height - 1][j] = '#';
        }

        start_x = (float)cell_size;
        start_y = (float)((height - 1) * cell_size - player_height);
    }

    // ---------- background and tiles ----------
    Texture bg_texture;
    Sprite bg_sprite;
    Texture block_texture;
    Sprite block_sprite;

    if (!bg_texture.loadFromFile("assets/backgrounds/level1.png")) {
        cout << "Could not load assets/backgrounds/level1.png" << endl;
        return 1;
    }
    bg_sprite.setTexture(bg_texture);
    bg_sprite.setPosition(0, 0);

    if (!block_texture.loadFromFile("assets/tiles/block1.png")) {
        cout << "Could not load assets/tiles/block1.png" << endl;
        return 1;
    }
    block_sprite.setTexture(block_texture);

    // ---------- player ----------
    float player_x = start_x;
    float player_y = start_y;
    float player_velocity_x = 0;
    float player_velocity_y = 0;
    bool player_on_ground = false;
    int player_facing = 1;
    int player_health = player_start_health;
    int invuln_timer = 0;

    Texture player_texture;
    Sprite player_sprite;

    if (!player_texture.loadFromFile("assets/sprites/player/hart_sprite.png")) {
        cout << "Could not load assets/sprites/player/hart_sprite.png" << endl;
        return 1;
    }
    player_sprite.setTexture(player_texture);

    // ---------- enemy pictures ----------
    const char *enemy_files[enemy_type_count];
    enemy_files[enemy_crawler] =
        "assets/sprites/enemies/spiked_crawler.png";
    enemy_files[enemy_jumper] = "assets/sprites/enemies/blue_jumper.png";
    enemy_files[enemy_insect] = "assets/sprites/enemies/flying_insect.png";
    enemy_files[enemy_leopard] = "assets/sprites/enemies/leopard.png";
    enemy_files[enemy_shooter] = "assets/sprites/enemies/green_shooter.png";
    enemy_files[enemy_tribal] =
        "assets/sprites/enemies/tribal_warrior.png";
    enemy_files[enemy_fish] = "assets/sprites/enemies/leaping_fish.png";

    Texture enemy_textures[enemy_type_count];
    for (int i = 0; i < enemy_type_count; i++) {
        if (!enemy_textures[i].loadFromFile(enemy_files[i])) {
            cout << "Could not load " << enemy_files[i] << endl;
            return 1;
        }
    }

    Texture bullet_texture;
    if (!bullet_texture.loadFromFile("assets/sprites/fx/enemy_bolt.png")) {
        cout << "Could not load assets/sprites/fx/enemy_bolt.png" << endl;
        return 1;
    }

    // One sprite is reused for every enemy and bullet.
    Sprite enemy_sprite;

    // ---------- enemy data ----------
    bool enemy_active[max_enemies];
    int enemy_type[max_enemies];
    float enemy_x[max_enemies];
    float enemy_y[max_enemies];
    float enemy_velocity_x[max_enemies];
    float enemy_velocity_y[max_enemies];
    int enemy_hp[max_enemies];
    int enemy_direction[max_enemies];
    int enemy_state[max_enemies];
    int enemy_timer[max_enemies];

    init_enemies(max_enemies, enemy_active, enemy_type, enemy_x, enemy_y,
                 enemy_velocity_x, enemy_velocity_y, enemy_hp, enemy_direction,
                 enemy_state, enemy_timer);

    // ---------- shooter bullet data ----------
    bool bullet_active[max_enemy_bullets];
    float bullet_x[max_enemy_bullets];
    float bullet_y[max_enemy_bullets];
    float bullet_velocity_x[max_enemy_bullets];

    init_enemy_projectiles(max_enemy_bullets, bullet_active, bullet_x,
                           bullet_y, bullet_velocity_x);

    // One crawler to start with.
    spawn_enemy(max_enemies, enemy_type_count, enemy_cfg, enemy_active,
                enemy_type, enemy_x, enemy_y, enemy_velocity_x,
                enemy_velocity_y, enemy_hp, enemy_direction, enemy_state,
                enemy_timer, enemy_crawler, 100, 100);

    Event ev;
    Clock frame_clock;

    // ---------- main loop ----------
    while (window.isOpen()) {
        while (window.pollEvent(ev)) {
            if (ev.type == Event::Closed) {
                window.close();
            }
        }

        // The only permitted controls: Right moves, Space jumps.
        bool move_right = Keyboard::isKeyPressed(Keyboard::Right);
        bool jump_pressed = Keyboard::isKeyPressed(Keyboard::Space);

        update_player(player_x, player_y, player_velocity_x, player_velocity_y,
                      player_on_ground, player_facing, move_right,
                      jump_pressed, player_speed, jump_speed, player_width,
                      player_height, lvl, height, width, cell_size, gravity,
                      terminal_velocity, collision_eps);

        if (player_fell_off_map(player_y, height, cell_size)) {
            respawn_player(player_x, player_y, player_velocity_x,
                           player_velocity_y, player_on_ground, start_x,
                           start_y);
        }

        // Enemies aim at the middle of the player.
        float target_x = player_x + player_width / 2.0f;
        float target_y = player_y + player_height / 2.0f;

        update_enemies(
            max_enemies, enemy_type_count, enemy_cfg, max_enemy_bullets,
            enemy_bullet_cfg, enemy_active, enemy_type, enemy_x, enemy_y,
            enemy_velocity_x, enemy_velocity_y, enemy_direction, enemy_state,
            enemy_timer, bullet_active, bullet_x, bullet_y,
            bullet_velocity_x, target_x, target_y, lvl, height, width,
            cell_size, gravity, terminal_velocity);

        update_enemy_projectiles(max_enemy_bullets, enemy_bullet_cfg,
                                 bullet_active, bullet_x, bullet_y,
                                 bullet_velocity_x, lvl, height, width,
                                 cell_size);

        if (invuln_timer > 0) {
            invuln_timer--;
        }

        int touch_damage = enemy_damage_on_contact(
            max_enemies, enemy_type_count, enemy_cfg, enemy_active, enemy_type,
            enemy_x, enemy_y, player_x, player_y, player_width,
            player_height);

        if (touch_damage == 0) {
            touch_damage = enemy_projectile_damage_on_contact(
                max_enemy_bullets, enemy_bullet_cfg, bullet_active, bullet_x,
                bullet_y, player_x, player_y, player_width, player_height);
        }

        if (touch_damage > 0 && invuln_timer == 0) {
            player_health -= touch_damage;
            invuln_timer = player_vuln_frames;
            cout << "Player health: " << player_health << endl;

            if (player_health <= 0) {
                cout << "Player died, health reset" << endl;
                player_health = player_start_health;
                invuln_timer = player_vuln_frames;
                respawn_player(player_x, player_y, player_velocity_x,
                               player_velocity_y, player_on_ground, start_x,
                               start_y);
            }
        }

        // ---------- drawing ----------
        window.clear();

        draw_level(window, lvl, bg_sprite, block_sprite, height, width,
                   cell_size);
        draw_enemies(window, enemy_sprite, enemy_textures, max_enemies,
                     enemy_type_count, enemy_cfg, enemy_active, enemy_type,
                     enemy_x, enemy_y, enemy_direction);
        draw_enemy_projectiles(window, enemy_sprite, bullet_texture,
                               max_enemy_bullets, enemy_bullet_cfg,
                               bullet_active, bullet_x, bullet_y,
                               bullet_velocity_x);
        draw_player(window, player_sprite, player_x, player_y, player_facing,
                    player_scale, player_width);

        window.display();

        // Clock-based cap instead of setFramerateLimit.
        while (frame_clock.getElapsedTime().asMilliseconds() < frame_ms) {
        }
        frame_clock.restart();
    }

    free_level(lvl, height);

    return 0;
}
