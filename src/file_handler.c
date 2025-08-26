#include <stdio.h>
#include <stdlib.h>
#include "file_handler.h"

TopPlayer *top_players_from_bin() {
    FILE *fileptr;

    TopPlayer *top_players = calloc(TOPLAYERS, sizeof(TopPlayer));

    if ((fileptr = fopen("../config/top_scores.bin", "r+")) != NULL) {
        fread(top_players, sizeof(TopPlayer), TOPLAYERS, fileptr);
    } else {
        TopPlayer default_top_player = {
            "Player",
            0,
        };

        for (int i = 0; i < TOPLAYERS; i++) {
            top_players[i] = default_top_player;
        }
    }

    fclose(fileptr);

    return top_players;
}
void top_players_to_bin(TopPlayer *top_players) {
    FILE *fileptr;

    if ((fileptr = fopen("../config/top_scores.bin", "wb"))) {
        fwrite(top_players, sizeof(TopPlayer), TOPLAYERS, fileptr);
        fclose(fileptr);
    }

    free(top_players);
}
