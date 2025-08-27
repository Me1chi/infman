#include "game.h"
#include "enemies.h"
#include "projectiles.h"
#include "textures_and_camera.h"

void pause(Player *player, DynVector enemies, SmartMap map, ProjVector projs, TexturesCamera *txt_cam, GameContext *ctx) {
    //tests if the pause menu must open
    if (IsKeyPressed(KEY_P) && !ctx->curr_screen)
        ctx->curr_screen = 1;

    if (ctx->curr_screen == 1)
        ctx->curr_screen = pause_display(player, enemies, map, projs, txt_cam);
}

void main_menu_test(Player *player,
                 DynVector *enemies,
                 ProjVector *projs,
                 SmartMap *map,
                 TopPlayer *top_players,
                 GameContext *ctx,
                 TexturesCamera *txt_cam) {
    //tests if it must enter the main menu
    if (ctx->curr_screen == 2) {
        ctx->curr_screen = main_menu_display(player, enemies, projs, *map, txt_cam, top_players, ctx);
    }
}

void player_win(Player *player, TexturesCamera *txt_cam, SmartMap map, GameContext *ctx, TopPlayer *top_players, Color filter) {
    if (is_player_on_ending_platform(player, map)) {
        draw_level_ending_subtitles(player, map, txt_cam, filter);
        if(IsKeyPressed(KEY_L))
            ctx->curr_screen = 3;
    }

    TopPlayer player_on_top = {0, 0};

    while (ctx->curr_screen == 3) {
        player_on_top = get_user_name_score(player, txt_cam, ctx);
    }

    applies_array_modifications(player_on_top, top_players);

}

int gaming_calls(Player *player,
                 DynVector *enemies,
                 ProjVector *projs,
                 SmartMap *map,
                 TopPlayer *top_players,
                 GameContext *ctx,
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

    player_win(player, txt_cam, *map, ctx, top_players, WHITE);

    laser_shoot(player, projs, enemies, map);

    bazooka_update(player, projs, enemies, map);

    EndMode2D();

    EndDrawing();

    player_movement(player);

    player_laser(player, projs);

    player_bazooka(player, projs);

    enemy_movement(*enemies, player, *map);

    enemies_laser(*enemies, projs);

    enemies_drop_manager(*enemies, player);

    camera_update(player, txt_cam, ctx);

    player_death_test(player, *enemies, txt_cam, *map, ctx, *projs);

    spike_damage(player, *map);

    vulnerability_update(player);

    return 0;
}

void gaming_test(Player *player,
                 DynVector *enemies,
                 ProjVector *projs,
                 SmartMap *map,
                 TopPlayer *top_players,
                 GameContext *ctx,
                 TexturesCamera *txt_cam) { //tests if the game must start
    //tests if the game must run
    if (ctx->curr_screen == 0)
        gaming_calls(player, enemies, projs, map, top_players, ctx, txt_cam);
}


