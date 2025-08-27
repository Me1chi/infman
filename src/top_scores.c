#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "top_scores.h"
#include "game.h"
#include "textures_and_camera.h"

void infman_dance(Player *player, TexturesCamera *txt_cam, Vector2 position, Color filter) {
    player->timer.sprite += GetFrameTime();
    if (player->timer.sprite >= SPRITEFRAMETIME) {
        player->sprite_counter = (rand() % (3));
        player->timer.sprite = 0;
    }

    Rectangle source_rectangle = {
        0,
        0,
        PLAYERSIZESHOOTJUMP,
        PLAYERSIZE
    };

    source_rectangle.x = player->sprite_counter * PLAYERSIZESHOOTJUMP;
    if (time(NULL) % 2 == 0)
        source_rectangle.y = PLAYERSIZE;

    DrawTextureRec(txt_cam->textures.player.running_shoot, source_rectangle, position, filter);

}

void applies_array_modifications(TopPlayer pl, TopPlayer *top_players) {
    int index = 0;

    index = compare_scores_array(pl, top_players);

    for (int i = TOPLAYERS - 1; i > index; i--) {
        top_players[i] = top_players[i - 1];
    }

    top_players[index] = pl;
}

int compare_scores_array(TopPlayer pl, TopPlayer *top_players) {
    int index = 0;

    while (index < TOPLAYERS) {
        if (top_players[index].score < pl.score)
            break;
        index++;
    }

    return index;
}

bool input_array_writer(char *array, int *position) {
    char letter_input;

    letter_input = GetKeyPressed();

    if (letter_input >= 32 && letter_input <= 125 && *position < MAXNAME && letter_input != KEY_EQUAL) {

        array[*position] = tolower(letter_input);

        array[*position+1] = '\0';

        (*position)++;
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (*position != 0)
            (*position)--;
        array[*position] = '\0';
    }


    return (letter_input == KEY_EQUAL);
}

TopPlayer get_user_name_score(Player *player, TexturesCamera *txt_cam, GameContext *ctx) {

    EndMode2D();

    EndDrawing();

    char input_buffer[MAXNAME + 1] = {0};
    char player_score_display[50] = "YOUR SCORE WAS -> ";
    char player_score_string[10];

    sprintf(player_score_string, "%d", player->score);

    strcat(player_score_display, player_score_string);

    int name_confirmed = 0;

    int letter_position = 0;

    TopPlayer player_to_be_on_top;

    while (!name_confirmed) {

        BeginDrawing();

        ClearBackground(INFMANBLUE1);

        DrawText("YOU WIN!", SCREENWIDTH/3, SCREENHEIGHT/10, 2*FONTSIZE, BLACK);
        DrawText(player_score_display, SCREENWIDTH/3, SCREENHEIGHT/10 + 3*FONTSIZE, 2*FONTSIZE, BLACK);
        DrawText("INSERT YOU NAME: ", SCREENWIDTH/3, SCREENHEIGHT/10 + 6*FONTSIZE, 2*FONTSIZE, BLACK);
        DrawText(input_buffer, SCREENWIDTH/3, SCREENHEIGHT/10 + 9*FONTSIZE, 2*FONTSIZE, DARKGRAY);

        infman_dance(player, txt_cam, (Vector2){SCREENWIDTH/3, 2*SCREENHEIGHT/3}, WHITE);

        infman_dance(player, txt_cam, (Vector2){SCREENWIDTH/3, 2*SCREENHEIGHT/3 + 3*PLAYERSIZE}, WHITE);

        infman_dance(player, txt_cam, (Vector2){SCREENWIDTH/3, 2*SCREENHEIGHT/3 + 6*PLAYERSIZE}, WHITE);

        infman_dance(player, txt_cam, (Vector2){2*SCREENWIDTH/3, 2*SCREENHEIGHT/3}, WHITE);

        infman_dance(player, txt_cam, (Vector2){2*SCREENWIDTH/3, 2*SCREENHEIGHT/3 + 3*PLAYERSIZE}, WHITE);

        infman_dance(player, txt_cam, (Vector2){2*SCREENWIDTH/3, 2*SCREENHEIGHT/3 + 6*PLAYERSIZE}, WHITE);

        EndDrawing();

        if (input_array_writer(input_buffer, &letter_position)) {
            name_confirmed = 1;
            ctx->curr_screen = 2;
        }
    }

    strcpy(player_to_be_on_top.name, input_buffer);
    player_to_be_on_top.score = player->score;

    return player_to_be_on_top;
}


