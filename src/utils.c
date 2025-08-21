#include "utils.h"
#include "raylib.h"
#include "math.h"

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

