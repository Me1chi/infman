#pragma once
#include "player.h"
#include "raylib.h"
#include "textures_and_camera.h"

#define TOPLAYERS 10
#define MAXNAME 9

//this type is meant to be used in the top 10 ranking
typedef struct {
    char name[MAXNAME + 1]; //the + 1 comes from the NULL character
    int score;

} TopPlayer;

//top scores updating functions

void infman_dance(Player *player, TexturesCamera *txt_cam, Vector2 position, Color filter);
//hehe this is cool

void applies_array_modifications(TopPlayer pl, TopPlayer *top_players);
//make the modifications on the top players array permanent

int compare_scores_array(TopPlayer pl, TopPlayer *top_players);
//make sure that the player is well placed in the top scores array

bool input_array_writer(char *array, int *position);
//called by the above one

TopPlayer get_user_name_score(Player *player, TexturesCamera *txt_cam, GameContext *ctx);
//takes the user input for name  and recording

