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

void time_actions_update_bool(bool *update_var, float *timer, float goal_time);

void get_tile_on_matrix(int *hor_tile, int *ver_tile, Rectangle object);
