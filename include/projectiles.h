#pragma once
#include "map.h"
#include "player.h"
#include "raylib.h"
#include "utils.h"
#include <stddef.h>
#include <stdlib.h>

#define MAXPROJECTILES 96
#define PROJECTILESIZE 8
#define LASERPROJECTILESPEED 3.0
#define BAZOOKAPROJECTILESPEED 1.5

typedef struct Projectile {
    Rectangle bullet;
    Vector2 direction;
    int projectile_type; //0 for laser, 1 for bazooka, 2 for enemy shot and so on...

} Projectile;

typedef struct ProjVector {
    Projectile *arr;
    size_t counter;
    size_t max;
} ProjVector;

ProjVector init_proj_vector();
void drop_proj_vector(ProjVector *projs);
void laser_shoot(Player *player, ProjVector *projs, DynVector *enemies, SmartMap *map);
void bazooka_update(Player *player, ProjVector *projs, DynVector *enemies, SmartMap *map);

