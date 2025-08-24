#pragma once
#include "raylib.h"
#include <stddef.h>

#define MAXPROJECTILES 96
#define PROJECTILESIZE 8
#define LASERPROJECTILESPEED 3.0
#define BAZOOKAPROJECTILESPEED 1.5

typedef struct {
    Rectangle bullet;
    Vector2 direction;
    int projectile_type; //0 for laser, 1 for bazooka, 2 for enemy shot and so on...

} Projectile;

typedef struct {
    Projectile *arr;
    size_t counter;
    size_t max;
} ProjVector;

