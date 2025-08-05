#include "textures_and_camera.h"
#include "file_handler.h"
#include "raylib.h"

void load_textures_testing(GameTextures *textures) {

    // Menu
    textures->menu.infman_logo = LoadTexture(FROM_RESOURCES("map/inf_man_logo.png"));
    textures->menu.play_button = LoadTexture(FROM_RESOURCES("map/button0.png"));
    textures->menu.leaderboard_button = LoadTexture(FROM_RESOURCES("map/button0.png"));
    textures->menu.exit_button = LoadTexture(FROM_RESOURCES("map/button0.png"));

    Image infmanblueImage = GenImageColor(BUTTONWIDTH*2, BUTTONHEIGHT*4, INFMANBLUE1);
    textures->menu.leaderboard_table = LoadTextureFromImage(infmanblueImage);
    UnloadImage(infmanblueImage);

    textures->menu.leaderboard_main_menu = LoadTexture(FROM_RESOURCES("map/button0.png"));
    textures->menu.resume_button = LoadTexture(FROM_RESOURCES("map/button1_esp.png"));
    textures->menu.main_menu = LoadTexture(FROM_RESOURCES("map/button1_esp.png"));

}

