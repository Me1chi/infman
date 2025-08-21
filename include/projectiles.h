#pragma once
#include "raylib.h"

typedef struct {
    Rectangle bullet;
    Vector2 direction;
    int projectile_type; //0 for laser, 1 for bazooka, 2 for enemy shot and so on...

} Projectile;

