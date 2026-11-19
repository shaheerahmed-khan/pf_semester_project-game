#include <SFML/Graphics.hpp>

int main() {
    // 1. VideoMode now takes a Vector2u ({width, height})
    sf::RenderWindow window(sf::VideoMode({400, 400}), "SFML on WSL");

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    while (window.isOpen()) {
        // 2. pollEvent() now returns an std::optional<sf::Event>
        while (const std::optional event = window.pollEvent()) {
            // 3. Use is<sf::Event::Closed>() instead of event.type
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear();
        window.draw(shape);
        window.display();
    }

    return 0;
}
