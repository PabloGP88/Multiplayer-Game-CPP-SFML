cmake_minimum_required(VERSION 3.20)
project(tank_game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML 3
find_package(SFML 3 COMPONENTS Graphics Network REQUIRED)

add_executable(tank_game
        game/main.cpp
        client/client_main.cpp
        server/game_server.cpp
        game/Tank.cpp
        game/game.cpp
        game/bullet.cpp
        game/collision_manager.cpp
        game/obstacle.cpp
        game/decorations.cpp
        game/gameUI.cpp
        game/pickUp.cpp
        game/ammoBox.cpp
        game/healthKit.cpp
)

# Link SFML 3 targets - Use SFML:: namespace (this includes headers automatically)
target_link_libraries(tank_game
        PRIVATE SFML::Graphics SFML::Network
)

# Copy Assets folder to build directory
add_custom_command(TARGET tank_game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Assets
        $<TARGET_FILE_DIR:tank_game>/Assets
)