#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include "utils.h"
#include "../client/client_main.h"
#include "../server/game_server.h"
#include "../config.h"


void RunServer() {
    Utils::printMsg("Started as SERVER", success);

    unsigned short port = Config::getServerPort();

    try {
        game_server server(port);
        server.Update();
    }
    catch (const std::exception& e) {
        Utils::printMsg("Server error: " + std::string(e.what()), error);
    }
}

void RunClient() {
    Utils::printMsg("Started as CLIENT", success);

    std::optional<sf::IpAddress> serverIP = sf::IpAddress::resolve(Config::getServerIP());
    unsigned short serverPort = Config::getServerPort();


    if (!serverIP.has_value()) {
        Utils::printMsg("Failed to get local IP address", error);
        return;
    }


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

    Utils::printMsg("------- Game Created ------- ", success);

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        // Handle events just as in the labs
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

            // Pass input to game, done like the labs
            if (client.game) {
                client.game->HandleEvents(event, client.GetPlayerId());
            }
        }

        if (client.game) {
            client.game->Update(dt);
        }

        client.Update();

        window.clear();
        if (client.game) {
            client.game->Render(window);
        }
        window.display();
    }

}

int main() {
    Utils::printMsg(" CMP501 – Tank Network Game - Pablo Gonzalez", success);

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
        Utils::printMsg("Invalid choice", error);
        return 1;
    }

    return 0;
}