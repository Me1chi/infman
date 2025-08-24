#pragma once
#include "raylib.h"
#include "utils.h"

typedef DynVector SmartMap;

typedef struct {
    Vector2 position;
    int drop_type; //0 for heart, 1 for circuit part and so on...
    int state; //0 for unavailable, 1 for available, 2 for took

} Drop;

SmartMap fetch_map();
void map_drop(SmartMap *smart_map);

Vector2 get_map_extremes(SmartMap smart_map);
void get_map_tiles_matrix(Rectangle **map_tiles, SmartMap map, int do_spikes_matter);
