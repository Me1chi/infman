#include "utils.h"
#include "raylib.h"
#include "math.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "textures_and_camera.h"

static const size_t VECTORMINSIZE = 3;

DynVector vector_new(size_t element_size) {
    DynVector new_vector;

    new_vector.ptr = malloc(element_size*VECTORMINSIZE);
    new_vector.len = 0;
    new_vector.size = 3;
    new_vector.element_size = element_size;

    return new_vector;
}

void vector_drop(DynVector *vector) {
    if (vector->ptr != NULL) {
        free(vector->ptr);
    }

    vector->len = 0;
    vector->size = 0;
    vector->element_size = 0;
}

void vector_push_back(DynVector *vector, void *item) {
    // Size testing and size increasing
    if (vector->len >= vector->size) {
        vector->size *= 2;
        vector->ptr = realloc(vector->ptr, vector->size * vector->element_size);
    }

    memcpy(
        (int8_t *)vector->ptr + vector->len * vector->element_size,
        item,
        vector->element_size
    );
    vector->len++;
}

// If NULL, out of bounds
void *vector_get(DynVector vector, size_t index) {
    if (index < vector.len) {
        return ((int8_t *)vector.ptr + index * vector.element_size);
    } else {
        return NULL;
    }
}

// Again, if NULL, out of bound
void *matrix_get(DynVector vector, size_t i, size_t j) {
    DynVector *row;
    void *element;

    row = vector_get(vector, i);
    if (row == NULL) {
        return NULL;
    }

    return vector_get(*row, j);
}

size_t matrix_row_len(DynVector vector, size_t row) {
    DynVector *rowptr;

    rowptr = vector_get(vector, row);
    if (rowptr == NULL) {
        return 0;
    }

    return vector_len(*rowptr);
}

size_t vector_len(DynVector vector) {
    return vector.len;
}

// I WILL NOT IMPLEMENT THE REMOVE FUNCTION BC IT IS NOT NECESSARY FOR THIS APLICATION

int roll_a_dice(int max_number) {
    if (max_number == 0)
        return 0;
    else
        return 1 + (rand() % max_number);
}

void time_actions_update_bool(bool *update_var, float *timer, float goal_time) {
    if (*timer >= goal_time) {
        *update_var = 1;
        *timer = 0;
    } else if (!(*update_var)) {
        *timer += GetFrameTime();
    } else
        *timer = 0.0f;
}

void get_tile_on_matrix(int *hor_tile, int *ver_tile, Rectangle object) {
    *hor_tile = floorf(object.x / TILESIZE);
    *ver_tile = floorf(object.y / TILESIZE);
}

bool check_map_tiles_collision(Rectangle **map_tiles, int limit_x, int limit_y, Rectangle object) {
    int testing = 0;

    for (int i = 0; i < limit_y; i++) {
        for (int j = 0; j < limit_x; j++) {

            testing += CheckCollisionRecs(map_tiles[i][j], object);

        }
    }

    return testing;
}



