#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include "utils.h"
#include "../client/client_main.h"
#include "../server/game_server.h"

void RunServer() {
    Utils::printMsg("=== STARTING  SERVER ===", success);

    unsigned short port = 53000;

    try {
        game_server server(port);
        server.Update();
    }
    catch (const std::exception& e) {
        Utils::printMsg("Server error: " + std::string(e.what()), error);
    }
}

void RunClient() {
    Utils::printMsg("=== TANK GAME CLIENT ===", success);

    // Use local IP - TODO: Talk this to teacher to see if is really what I need to do
    std::optional<sf::IpAddress> serverIP = sf::IpAddress::getLocalAddress();

    if (!serverIP.has_value()) {
        Utils::printMsg("Failed to get local IP address", error);
        return;
    }

    unsigned short serverPort = 53000;

    Utils::printMsg("Server IP: " + serverIP.value().toString(), info);

    // Create client
    client_main client(serverIP.value(), serverPort);

    // Connect to server
    Utils::printMsg("Connecting to " + serverIP.value().toString() + ":" + std::to_string(serverPort) + "...", info);
    if (!client.Connect()) {
        Utils::printMsg("Failed to connect to server", error);
        return;
    }

    // Create window
    sf::RenderWindow window(sf::VideoMode({480, 360}), "Tank Game - Client");
    window.setFramerateLimit(60);

    Utils::printMsg("=== GAME STARTED ===", success);

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        // Handle events
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                client.Disconnect();
                window.close();
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                    client.Disconnect();
                    window.close();
                }
            }

            // Pass input to game
            if (client.game) {
                client.game->HandleEvents(event, client.GetPlayerId());
            }
        }

        // Update game locally
        if (client.game) {
            client.game->Update(dt);
        }

        // Network update
        client.Update();

        // Render
        window.clear();
        if (client.game) {
            client.game->Render(window);
        }
        window.display();
    }

    Utils::printMsg("=== GAME ENDED ===", warning);
}

int main() {
    Utils::printMsg("=== TANK GAME ===", success);

    std::string choice;
    std::cout << "Launch as:" << std::endl;
    std::cout << "  [1] Server" << std::endl;
    std::cout << "  [2] Client" << std::endl;
    std::cout << "Choice: ";
    std::getline(std::cin, choice);

    if (choice == "1") {
        RunServer();
    } else if (choice == "2") {
        RunClient();
    } else {
        Utils::printMsg("Invalid choice!", error);
        return 1;
    }

    return 0;
}