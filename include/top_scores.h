#pragma once
#include "raylib.h"

#define TOPLAYERS 10
#define MAXNAME 9

//this type is meant to be used in the top 10 ranking
typedef struct {
    char name[MAXNAME + 1]; //the + 1 comes from the NULL character
    int score;

} TopPlayer;

//top scores updating functions
void infman_dance(Vector2 position, Color filter); //hehe this is cool
TopPlayer get_user_name_score(Camera2D player_camera); //takes the user input for name  and recording
bool input_array_writer(char *array, int *position); //called by the above one
int compare_scores_array(TopPlayer pl); //make sure that the player is well placed in the top scores array
void applies_array_modifications(TopPlayer pl); //make the modifications on the top players array permanent

