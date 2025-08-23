#include "map.h"
#include "utils.h"
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

void drop_map(SmartMap *smart_map) {
    for (int i = 0; i < smart_map->len; i++) {
        vector_drop((DynVector *)vector_get(*smart_map, i));
    }

    vector_drop(smart_map);
}
