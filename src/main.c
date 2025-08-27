#include "enemies.h"
#include "game.h"
#include "projectiles.h"
#include "raylib.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "file_handler.h"
#include "textures_and_camera.h"
#include "top_scores.h"
#include "map.h"
#include "game.h"
#include "utils.h"

//DROP ammo[ENEMIES] = {0};

//main function
int main() {

    //initializes the game window
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "INFman");

    // Top players leaderboard
    TopPlayer *top_players = top_players_from_bin();

    // Map from the config file
    SmartMap map = fetch_map();
    Vector2 map_extremes_float = get_map_extremes(map);
    int map_extreme_x = (int)map_extremes_float.x;
    int map_extreme_y = (int)map_extremes_float.y;

    Player player;
    ProjVector projs;
    DynVector enemies = vector_new(sizeof(Enemy));

    // Camera and textures from the files
    TexturesCamera textures_and_camera;
    load_textures(&textures_and_camera.textures);
    init_camera(&textures_and_camera.camera, SCREENWIDTH, map_extreme_y);

    //initializes the audio devide (if using -> FIX THE XCODE SOUND ISSUE)
    InitAudioDevice();

    printf("Hello, World!\n");

    //defines as 60 the target fps
    SetTargetFPS(60);

    //random seed initialization
    long current_time = time(NULL);
    unsigned int current_time_reduced = (unsigned int)current_time;
    srand(current_time_reduced);

    GameContext game_context = {
        .curr_screen = menu,
        .running = true,
    };

    while (!WindowShouldClose() && game_context.running) {
        main_menu_test(&player, &enemies, &projs, &map, top_players, &game_context, &textures_and_camera);
        pause(&player, enemies, map, projs, &textures_and_camera, &game_context);
        gaming_test(&player, &enemies, &projs, &map, top_players, &game_context, &textures_and_camera);
    }

    //textures unloading
    unload_textures(&textures_and_camera.textures);

    //closes the audio device
    CloseAudioDevice();

    // Drop dynamic vectors and matrix
    map_drop(&map);
    vector_drop(&enemies);
    free(projs.arr);

    //closes the window
    printf("AQUI DEVERIA FECHAR A JANELA");
    CloseWindow();

    return 0;
}


