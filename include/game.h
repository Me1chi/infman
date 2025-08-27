#pragma once
#include <stdbool.h>
#include "map.h"
#include "projectiles.h"
#include "top_scores.h"
#include "utils.h"
#include "textures_and_camera.h"

typedef enum Screen {
    gaming = 0,
    paused = 1,
    menu = 2,
    win = 3,
} Screen;

typedef struct GameContext {
    Screen curr_screen;
    bool running;
} GameContext;

void main_menu_test(Player *player,
                 DynVector *enemies,
                 ProjVector *projs,
                 SmartMap *map,
                 TopPlayer *top_players,
                 GameContext *ctx,
                 TexturesCamera *txt_cam);

void pause(Player *player, DynVector enemies, SmartMap map, ProjVector projs, TexturesCamera *txt_cam, GameContext *ctx);

void gaming_test(Player *player,
                 DynVector *enemies,
                 ProjVector *projs,
                 SmartMap *map,
                 TopPlayer *top_players,
                 GameContext *ctx,
                 TexturesCamera *txt_cam); //tests if the game must start
    //tests if the game must run
