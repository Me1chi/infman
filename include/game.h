#pragma once

#include <stdbool.h>
#include "player.h"
#include "projectiles.h"
#include "raylib.h"
#include "textures_and_camera.h"
#include "utils.h"

typedef enum {
    gaming = 0,
    paused = 1,
    menu = 2,
} Screen;

typedef struct {
    Screen curr_screen;
    bool running;
} GameContext;




void game(TexturesCamera *txt_cam,
          GameContext *ctx
          );

void main_menu_test(void); //tests if the player should be in the main menu and calls main_menu_test() if it does (MUST BE THE FIRST FUNCTION TO RUN IN THE MAIN LOOP!!!

void pause(Player *player, DynVector enemies, SmartMap map, ProjVector projs, TexturesCamera *txt_cam, GameContext *ctx);






void pause(Player *player, DynVector enemies, SmartMap map, ProjVector projs, TexturesCamera *txt_cam, GameContext *ctx) {
    //tests if the pause menu must open
    if (IsKeyPressed(KEY_P) && !ctx->curr_screen)
        ctx->curr_screen = 1;

    if (ctx->curr_screen == 1)
        ctx->curr_screen = pause_display(player, enemies, map, projs, txt_cam);
}

void player_win(Camera2D player_camera, Color filter) {
    if (is_player_on_ending_platform()) {
        draw_level_ending_subtitles(player_camera, filter);
        if(IsKeyPressed(KEY_L))
            current_screen = 3;
    }

    PLAYER_ON_TOP player_on_top = {0, 0};

    while (current_screen == 3) {
        player_on_top = get_user_name_score(player_camera);
    }

    applies_array_modifications(player_on_top);

}

void gaming_test(TexturesCamera *textures_and_camera) { //tests if the game must start
    //tests if the game must run
    if (current_screen == 0)
        gaming_calls(player_camera);
}

int gaming_calls(Player *player,
                 DynVector *enemies,
                 ProjVector *projs,
                 SmartMap *map,
                 TexturesCamera *txt_cam) { //all of the gaming functions run inside it

    is_player_blocked(player, *map);

    is_player_on_map(player, *map);

    BeginDrawing();

    ClearBackground(RAYWHITE);

    BeginMode2D(txt_cam->camera);

    draw_background(*map, txt_cam, WHITE);
    draw_map(*map, txt_cam, WHITE);
    draw_projectiles(*projs, txt_cam, WHITE);
    draw_player(player, txt_cam, WHITE);
    draw_enemies(*enemies, txt_cam, WHITE);
    draw_player_hearts_ammo(player, txt_cam, WHITE);






    player_win(*player_camera, WHITE);

    laser_shoot();

    bazooka_update();

    EndMode2D();

    EndDrawing();

    player_movement();

    player_laser();

    player_bazooka();

    enemy_movement();

    enemies_laser();

    enemies_drop_manager();

    camera_update(player_camera);

    player_death_test(*player_camera);

    spike_damage();

    vulnerability_update();

    return 0;
}

