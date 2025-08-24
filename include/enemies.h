#pragma once

#include "math.h"
#include "player.h"
#include "projectiles.h"
#include "raylib.h"
#include "map.h"
#include "textures_and_camera.h"
#include "utils.h"

#define ENEMYHEARTS 2
#define ENEMYSIZE1 20
#define ENEMYMINSPEEDX 0.1
#define ENEMYMAXSPEEDX 0.5
#define ENEMYMINSPEEDY 0.01
#define ENEMYMAXSPEEDY 0.05
#define ENEMYMINMOVERANGEX 2
#define ENEMYMAXMOVERANGEX 4
#define ENEMYMINMOVERANGEY 1
#define ENEMYMAXMOVERANGEY 2
#define CHANCEOFSHOOTING 2 //the number here is represented by its inverse! example 6 -> 1/6
#define ENEMYRELOADTIMEMIN 1
#define ENEMYRELOADTIMEMAX 2
#define ENEMYFOV 700.0

typedef struct {
    //vector quantities
    Rectangle position_size;
    Vector2 speed;
    Vector2 pivot;
    Vector2 movement_range;

    //scalar quantities
    int hearts;
    int reload_time;
    double shooting_timer;

    //state testing
    bool blocked_left;
    bool blocked_right;
    bool shooting;
    bool alive;
    bool met_player;

} Enemy;

//enemies mechanics/update functions
void enemies_meet_player(DynVector enemies, Player *player);
bool enemy_looking_at_player(Enemy *en, Player *player);
void enemy_movement(DynVector enemies, Player *player, SmartMap smart_map);
bool enemy_should_shoot(Enemy *en); //use probability to return if the enemy should shoot
void enemies_laser(DynVector enemies, ProjVector *projs);
bool projectile_enemy_hit_test(Projectile *proj, Enemy *en);
void enemies_drop_manager(DynVector enemies, Player *player);

void enemies_meet_player(DynVector enemies, Player *player) {
    for (int i = 0; i < vector_len(enemies); i++) {
        Enemy *enptr = vector_get(enemies, i);

        if (fabsf(player->position.x - enptr->position_size.x) < ENEMYFOV/DEFAULTZOOM)
            enptr->met_player = 1;
    }
}

bool enemy_looking_at_player(Enemy *en, Player *player) {
    bool test = 0;

    test = (en->speed.x < 0 && player->position.x < en->position_size.x) || (en->speed.x > 0 && player->position.x > en->position_size.x);

    return test;
}


void enemy_movement(DynVector enemies, Player *player, SmartMap smart_map) {
    enemies_meet_player(enemies, player);

    Vector2 extremes = get_map_extremes(smart_map);
    int extreme_y = extremes.y;
    int extreme_x = extremes.x;

    Rectangle map_tiles_data[extreme_y][extreme_x];

    Rectangle *map_tiles_pointers[extreme_y];

    for (int i = 0; i < extreme_y; i++) {
        map_tiles_pointers[i] = map_tiles_data[i];
    }

    get_map_tiles_matrix(map_tiles_pointers, smart_map, 0);

    int horizontal_tile = 0;
    int vertical_tile = 0;

    for (int i = 0; i < vector_len(enemies); i++) {

        Enemy *enptr = vector_get(enemies, i);

        get_tile_on_matrix(&horizontal_tile, &vertical_tile, enptr->position_size);

        if (horizontal_tile <= 0 || horizontal_tile >= extreme_x - 1)
            enptr->speed.x = -enptr->speed.x;

        if (fabsf(enptr->position_size.x - enptr->pivot.x) > enptr->movement_range.x)
            enptr->speed.x = -enptr->speed.x;


        if (vertical_tile <= 0 || vertical_tile >= extreme_y) {
            enptr->speed.y = -enptr->speed.y;
            if (vertical_tile <= 4)
                enptr->position_size.y = enptr->pivot.y;
        }

        if (fabsf(enptr->position_size.y - enptr->pivot.y) > enptr->movement_range.y ||
            check_map_tiles_collision(map_tiles_pointers, extreme_x, extreme_y, enptr->position_size))
            enptr->speed.y = -enptr->speed.y;


        if (enptr->hearts <= 0) {
            enptr->pivot = (Vector2){enptr->pivot.x, enptr->pivot.y};
            enptr->position_size.y = -SCREENHEIGHT;
        }

        if (enemy_looking_at_player(enptr, player) &&
            enptr->met_player)
            enptr->shooting = 1;
        else
            enptr->shooting = 0;

        enptr->position_size.x += enptr->speed.x;
        enptr->position_size.y += enptr->speed.y;
    }
}

bool enemy_should_shoot(Enemy *en) {
    int decision;

    int enemy_can_shoot = 0;

    if (en->shooting_timer >= en->reload_time && en->shooting) {
        enemy_can_shoot = 1;
        en->shooting_timer = 0;
    } else if (!enemy_can_shoot) {
        en->shooting_timer += GetFrameTime();
    } else
        en->shooting_timer = 0;

    if (roll_a_dice(CHANCEOFSHOOTING) == 1 && enemy_can_shoot)
        decision = 1;
    else
        decision = 0;

    return decision;
}

void enemies_laser(DynVector enemies, ProjVector *projs) {
    for (int i = 0; i < vector_len(enemies); i++) {

        Enemy *enptr = vector_get(enemies, i);

        if (enemy_should_shoot(enptr)) {

            int current_direction_x;
            Rectangle current_bullet;

            if (projs->counter >= projs->max)
                projs->counter = 0;

            current_direction_x = enptr->speed.x/fabsf(enptr->speed.x);

            if (current_direction_x == 1)
                current_bullet.x = enptr->position_size.x + ENEMYSIZE1;
            else
                current_bullet.x = enptr->position_size.x;

            current_bullet.y = enptr->position_size.y + 12;
            current_bullet.width = PROJECTILESIZE;
            current_bullet.height = PROJECTILESIZE/2.0;

            projs->arr[projs->counter] = (Projectile){current_bullet, (Vector2){current_direction_x, 0.0}, 2};

            projs->counter++;
        }
    }
}

bool projectile_enemy_hit_test(Projectile *proj, Enemy *en) {
    return CheckCollisionRecs(proj->bullet, en->position_size);;
}

void enemies_drop_manager(DynVector enemies, Player *player) {
    for (int i = 0; i < vector_len(enemies); i++) {

        Enemy *enptr = vector_get(enemies, i);

        if (enptr->hearts <= 0 && enptr->alive) {
            if (!enptr->met_player)
                player->score += 15;
            else if (!enemy_looking_at_player(enptr, player))
                player->score += 50;
            else
                player->score += 100;
            //ammo[i].state = 1;
            //ammo[i].position = enptr->pivot;
            enptr->alive = 0;
        }
    }
}

