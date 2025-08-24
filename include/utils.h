#pragma once
#include "raylib.h"
#include <stdio.h>

typedef struct {
    void *ptr;
    size_t len;
    size_t size;
    size_t element_size;
} DynVector;

DynVector vector_new(size_t element_size);
void vector_drop(DynVector *vector);
void vector_push_back(DynVector *vector, void *item);
void *vector_get(DynVector vector, size_t index);
size_t vector_len(DynVector vector);
void *matrix_get(DynVector vector, size_t i, size_t j);

void time_actions_update_bool(bool *update_var, float *timer, float goal_time);
int roll_a_dice(int max_number);

void get_tile_on_matrix(int *hor_tile, int *ver_tile, Rectangle object);
bool check_map_tiles_collision(Rectangle **map_tiles, int limit_x, int limit_y, Rectangle object); //check all the blocks in the map tiles matrix. Searching for collision
