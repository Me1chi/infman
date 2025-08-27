#pragma once
#include "player.h"
#include "projectiles.h"
#include "raylib.h"
#include "map.h"
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

typedef struct Enemy {
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

struct ProjVector;

//enemies mechanics/update functions Enemy enemy_new(int tile_x, int tile_y); void init_enemies(DynVector *enemies, SmartMap map);
void init_enemies(DynVector *enemies, SmartMap map);
void enemies_meet_player(DynVector enemies, Player *player);
bool enemy_looking_at_player(Enemy *en, Player *player);
void enemy_movement(DynVector enemies, Player *player, SmartMap smart_map);
bool enemy_should_shoot(Enemy *en); //use probability to return if the enemy should shoot
void enemies_laser(DynVector enemies, ProjVector *projs);
bool projectile_enemy_hit_test(Projectile *proj, Enemy *en);
void enemies_drop_manager(DynVector enemies, Player *player);

