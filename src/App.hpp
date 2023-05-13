#pragma once

#include <SFML/Graphics/RenderWindow.hpp>

class App {
public:
    App();
    ~App();
    void run();

private:
    void updateUI();

    bool m_failedExport { false };
    sf::RenderWindow m_renderWindow;
};
