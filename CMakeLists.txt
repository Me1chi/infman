cmake_minimum_required(VERSION 3.7)
project(infman C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -g")

# Put the executable file in the bin folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Export compile commands for the language server
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include FetchContent to make raylib work
include(FetchContent)

# Prevents unnecessary downloads
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

# Makes FetchContent able to reach Raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)

# Makes Raylib available
FetchContent_MakeAvailable(raylib)

# Add the include directory to the compiler's search path
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(infman
    src/main.c
    src/file_handler.c
    src/textures_and_camera.c
    src/top_scores.c
    src/player.c
    src/projectiles.c
    src/utils.c
    src/map.c
    src/enemies.c
    src/game.c
)

target_link_libraries(infman PRIVATE raylib)
