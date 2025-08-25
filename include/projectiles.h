#pragma once
#include "raylib.h"
#include "textures_and_camera.h"
#include <stddef.h>
#include <stdlib.h>

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

ProjVector init_proj_vector() {
    ProjVector new;
    new.arr = malloc(MAXPROJECTILES*sizeof(Projectile));
    new.counter = 0;
    new.max = MAXPROJECTILES;

    for (int i = 0; i < new.max; i++) {
        new.arr[i].bullet.y = -SCREENWIDTH;
    }

    return new;
}

void drop_proj_vector(ProjVector *projs) {
    free(projs->arr);
    projs->counter = 0;
    projs->max = 0;
}

