#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include "Utils.h"
#include "Game.h"
#include "tank_message.h"

int main() {
    Utils::printMsg("Game startup...");

    // Tank choice
    std::string tankChoice;
    unsigned short myPort;
    unsigned short remotePort;
    std::string tankColor;
    int localId = 0;   // local player ID
    int remoteId = 1;  // remote player ID

    Utils::printMsg("Choose your tank:");
    Utils::printMsg("  [1] Tank A (Blue)  - Port 53000");
    Utils::printMsg("  [2] Tank B (Red)   - Port 53001");
    std::cout << "Choice: ";
    std::getline(std::cin, tankChoice);

    if (tankChoice == "1") {
        myPort = 53000;
        remotePort = 53001;
        tankColor = "blue";
        localId = 0;
        remoteId = 1;
        Utils::printMsg("You are Tank A (Blue)", success);
    } else if (tankChoice == "2") {
        myPort = 53001;
        remotePort = 53000;
        tankColor = "red";
        localId = 1;
        remoteId = 0;
        Utils::printMsg("You are Tank B (Red)", success);
    } else {
        Utils::printMsg("Invalid choice!", error);
        return 1;
    }

    // Window
    sf::RenderWindow window(sf::VideoMode({640, 480}), "Tank Game - " + tankColor);
    window.setFramerateLimit(60);

    // Networking
    sf::UdpSocket socket;
    sf::IpAddress remoteIP = sf::IpAddress::LocalHost;

    if (socket.bind(myPort) != sf::Socket::Status::Done) {
        Utils::printMsg("Error binding socket!", error);
        return 1;
    }
    socket.setBlocking(false);

    // Game object
    Game game;
    // Add local tank
    game.AddTank(localId, tankColor);
    // Add remote tank (placeholder color)
    game.AddTank(remoteId, (tankColor == "blue" ? "red" : "blue"));

    Utils::printMsg("=== GAME STARTED ===", success);
    Utils::printMsg("Sending to: " + remoteIP.toString() + ":" + std::to_string(remotePort));

    sf::Clock clock;
    float send_rate = 0.1f;
    float send_timer = 0;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        send_timer += dt;

        // Events
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                socket.unbind();
            }
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                    window.close();
                    socket.unbind();
                }
            }
            game.HandleEvents(event, localId); // pass localId
        }

        // Update world
        game.Update(dt);

        // Send local tank state
        if (send_timer >= send_rate) {
            TankMessage message = game.GetNetworkUpdate(localId);
            message.playerId = localId; // add playerId field in TankMessage
            sf::Packet packet;
            packet << message;

            socket.send(packet, remoteIP, remotePort);
            send_timer = 0;
        }

        // Receive remote tank state
        sf::Packet incomingPacket;
        sf::IpAddress senderIP;

        unsigned short senderPort;
        if (socket.receive(incomingPacket, senderIP, senderPort) == sf::Socket::Status::Done) {
            TankMessage receivedMessage;
            if (incomingPacket >> receivedMessage) {
                game.NetworkUpdate(dt, receivedMessage.playerId, receivedMessage);
            }
        }

        // Render
        window.clear();
        game.Render(window); // render all tanks, UI only for local
        window.display();
    }

    Utils::printMsg("=== GAME ENDED ===", warning);
    return 0;
}
