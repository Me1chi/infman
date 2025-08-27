#include "projectiles.h"
#include "enemies.h"
#include "map.h"
#include "player.h"
#include "textures_and_camera.h"
#include "utils.h"

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

void laser_shoot(Player *player, ProjVector *projs, DynVector *enemies, SmartMap *map) {

    Vector2 extremes = get_map_extremes(*map);
    int extreme_x = extremes.x;
    int extreme_y = extremes.y;

    Rectangle map_tiles_data[extreme_y][extreme_x];

    Rectangle *map_tiles[extreme_y];

    for (int i = 0; i < extreme_y; i++) {
        map_tiles[i] = map_tiles_data[i];
    }

    get_map_tiles_matrix(map_tiles, *map, true);

    for (int i = 0; i < projs->max; i++) {

        if (projs->arr[i].projectile_type == 0 || projs->arr[i].projectile_type == 2) {

            projs->arr[i].bullet.x += LASERPROJECTILESPEED*projs->arr[i].direction.x;
            projs->arr[i].bullet.y += LASERPROJECTILESPEED*projs->arr[i].direction.y;

            if (projectile_player_hit_test(player, projs->arr[i]) &&
                projs->arr[i].projectile_type == 2) {

                projs->arr[i].bullet.y = -SCREENWIDTH;
                player_damage(player, 'L');

            }


            for (int j = 0; j < vector_len(*enemies); j++) {

                Enemy *enptr = vector_get(*enemies, j);

                if (projectile_enemy_hit_test(&projs->arr[i], enptr) &&
                    projs->arr[i].projectile_type == 0) {

                    Enemy *enptr = vector_get(*enemies, j);
                    enptr->hearts--;

                    projs->arr[i].bullet.y = -SCREENHEIGHT;
                }
            }
            

            if (check_map_tiles_collision(map_tiles, extreme_x, extreme_y, projs->arr[i].bullet) &&
                projs->arr[i].projectile_type == 0) {
                projs->arr[i].bullet.y = -SCREENHEIGHT;
            }
        }
    }
}

void destroy_tile(SmartMap map, int ver_tile, int hor_tile) {
    char *charptr = matrix_get(map, ver_tile, hor_tile);

    if (charptr != NULL) {
        *charptr = ' ';
    }
}

void bazooka_update(Player *player, ProjVector *projs, DynVector *enemies, SmartMap *map) {

    Vector2 extremes = get_map_extremes(*map);
    int extreme_x = extremes.x;
    int extreme_y = extremes.y;

    Rectangle map_tiles_data[extreme_y][extreme_x];

    Rectangle *map_tiles[extreme_y];

    for (int i = 0; i < extreme_y; i++) {
        map_tiles[i] = map_tiles_data[i];
    }

    get_map_tiles_matrix(map_tiles, *map, true);

    for (int i = 0; i < projs->max; i++) {

        if (projs->arr[i].projectile_type == 1) {

            //bullet movement
            projs->arr[i].bullet.x += BAZOOKAPROJECTILESPEED*projs->arr[i].direction.x;

            projs->arr[i].bullet.y += BAZOOKAPROJECTILESPEED*projs->arr[i].direction.y;

            //enemy collision check
            for (int j = 0; j < vector_len(*enemies); j++) {

                Enemy *enptr = vector_get(*enemies, j);

                if (projectile_enemy_hit_test(&projs->arr[i], enptr) &&
                    projs->arr[i].projectile_type == 1) {

                    Enemy *enptr = vector_get(*enemies, j);
                    enptr->hearts = 0;

                    projs->arr[i].bullet.y = -SCREENHEIGHT;
                }
            }

            //map tiles collision
            if (check_map_tiles_collision(map_tiles, extreme_x, extreme_y, projs->arr[i].bullet) &&
                projs->arr[i].projectile_type == 1) {

                int horizontal_exp_tile = 0;
                int vertical_exp_tile = 0;


                projs->arr[i].bullet.x += PROJECTILESIZE*2;

                get_tile_on_matrix(&horizontal_exp_tile, &vertical_exp_tile, projs->arr[i].bullet);

                //explosion tile
                destroy_tile(*map, vertical_exp_tile, horizontal_exp_tile);

                //adjacent tiles
                destroy_tile(*map, vertical_exp_tile, horizontal_exp_tile + 1);
                destroy_tile(*map, vertical_exp_tile, horizontal_exp_tile - 1);

                //above tile
                destroy_tile(*map, vertical_exp_tile - 1, horizontal_exp_tile);

                if (roll_a_dice(PLAYERLUCK) == 1) {
                    destroy_tile(*map, vertical_exp_tile - 1, horizontal_exp_tile + 1);
                }
                if (roll_a_dice(PLAYERLUCK) == 1 ) {
                    destroy_tile(*map, vertical_exp_tile - 1, horizontal_exp_tile - 1);
                }

                //below tile
                destroy_tile(*map, vertical_exp_tile + 1, horizontal_exp_tile);

                if (roll_a_dice(PLAYERLUCK) == 1) {
                    destroy_tile(*map, vertical_exp_tile + 1, horizontal_exp_tile + 1);

                }
                if (roll_a_dice(PLAYERLUCK) == 1) {
                    destroy_tile(*map, vertical_exp_tile + 1, horizontal_exp_tile - 1);

                }

                projs->arr[i].bullet.y = -SCREENHEIGHT;

            }
        }
    }
}
