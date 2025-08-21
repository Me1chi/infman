#pragma once
#include "raylib.h"

void time_actions_update_bool(bool *update_var, float *timer, float goal_time);

void get_tile_on_matrix(int *hor_tile, int *ver_tile, Rectangle object);
