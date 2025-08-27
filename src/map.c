#include "map.h"
#include "raylib.h"
#include "utils.h"
#include "textures_and_camera.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

SmartMap fetch_map() {

    SmartMap vector = vector_new(sizeof(DynVector));

    FILE *fileptr;

    if ((fileptr = fopen("../config/terrain.txt", "r")) != NULL) {

        char read = fgetc(fileptr);
        if (read != EOF) {

            DynVector row = vector_new(sizeof(char));

            do {
                if (read == '\n') {
                    vector_push_back(&vector, &row);
                    row = vector_new(sizeof(char));
                } else {
                    vector_push_back(&row, &read);
                }
            } while ((read = fgetc(fileptr)) != EOF);

            vector_push_back(&vector, &row);

        }
        fclose(fileptr);
    } else {
        // HERE IT IS POSSIBLE TO IMPLEMENT A DEFAULT MAP 
        // WHEN THE TERRAIN.TXT FILE DOES NOT EXIST
    }

    return vector;
}

void map_drop(SmartMap *smart_map) {
    for (int i = 0; i < smart_map->len; i++) {
        vector_drop((DynVector *)vector_get(*smart_map, i));
    }

    vector_drop(smart_map);
}

Vector2 get_map_extremes(SmartMap smart_map) {
    Vector2 lens;

    int greatest = 0;
    int test = 0;
    // Here I grab the largest row
    for (int i = 0; i < vector_len(smart_map); i++) {
        if ((test = vector_len(*(SmartMap *)vector_get(smart_map, i))) > greatest) {
            greatest = test;
        }
    }

    lens.x = greatest;
    lens.y = vector_len(smart_map);

    return lens;
}

void get_map_tiles_matrix(Rectangle **map_tiles, SmartMap map, int do_spikes_matter) {

    size_t map_rows = vector_len(map);

    for (int i = 0; i < map_rows; i++) {

        DynVector row = *(DynVector *)vector_get(map, i);
        size_t len = vector_len(row);

        for (int j = 0; j < len; j++) {

            char element = *(char *)vector_get(row, j);

            if (element == 'O' || element == 'S'*do_spikes_matter)
                map_tiles[i][j] = (Rectangle){
                    j * TILESIZE,
                    i * TILESIZE,
                    TILESIZE,
                    TILESIZE
                };
            else
                map_tiles[i][j] = (Rectangle){
                    0,
                    0,
                    0,
                    0
                };
        }
    }
}

