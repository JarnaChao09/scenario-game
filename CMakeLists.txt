cmake_minimum_required(VERSION 3.21)
project(scenario-game)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(/opt/homebrew/include)

add_library(scenario SHARED src/scenario.c)
add_library(calc STATIC src/calc.c)

add_executable(game src/gui.c)
target_link_libraries(game scenario SDL2 SDL2_ttf SDL2_image)
add_executable(game-cli src/terminal.c)
target_link_libraries(game-cli scenario)

