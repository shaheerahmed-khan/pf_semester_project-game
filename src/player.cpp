#include "player.h"

#include "physics.h"

void update_player(float& player_x, float& player_y, float& velocity_x, float& velocity_y,
                   bool& on_Ground, int& facing, float speed_Multiplier, char** lvl, int height,
                   int width) {
    velocity_x = 0;
    float speed = PLAYER_SPEED * speed_Multiplier;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        velocity_x = -speed;
        facing = -1;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        velocity_x = speed;
        facing = 1;
    }

    bool jump_key = sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

    if (jump_key && on_Ground) {
        velocity_y = -JUMP_SPEED;  // -20: shoots upward
    }

    apply_gravity(velocity_y);
    move_with_collision(lvl, height, width, player_x, player_y, PLAYER_WIDTH, PLAYER_HEIGHT,
                        velocity_x, velocity_y, on_Ground);
}

void respawn_player(float& player_x, float& player_y, float& velocity_x, float& velocity_y,
                    float start_x, float start_y) {
    player_x = start_x;
    player_y = start_y;
    velocity_x = 0;
    velocity_y = 0;
}

bool player_fell_off_map(float y, int height) {
    return y > height * CELL_SIZE;
}

void draw_player(sf::RenderWindow& window, sf::Sprite& sprite, float x, float y, int facing) {
    // no usable picture -> draw a green box so you can still test
    if (sprite.getTexture() == nullptr || sprite.getTexture()->getSize().x == 0) {
        sf::RectangleShape box(sf::Vector2f(PLAYER_WIDTH, PLAYER_HEIGHT));
        box.setFillColor(sf::Color::Green);
        box.setPosition(x, y);
        window.draw(box);
        return;
    }

    if (facing == 1) {
        sprite.setScale(PLAYER_SCALE, PLAYER_SCALE);
        sprite.setPosition(x, y);
    } else {
        sprite.setScale(-PLAYER_SCALE, PLAYER_SCALE);  // negative = flipped
        sprite.setPosition(x + PLAYER_WIDTH, y);
    }
    window.draw(sprite);
}
