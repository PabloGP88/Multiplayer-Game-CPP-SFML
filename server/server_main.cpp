//
// Created by Pablo Gonzalez Poblette on 17/11/25.
//

#include "game_server.h"
#include "../game/utils.h"
#include <iostream>

int main() {

    unsigned short port = 53000;

    try {
        game_server server(port);
        server.Update();
    }
    catch (const std::exception& e) {
        Utils::printMsg("Server error: " + std::string(e.what()), error);
        return 1;
    }

    return 0;
}