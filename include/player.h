#pragma once
#include "map.h"
#include "raylib.h"
#include "top_scores.h"
#include "utils.h"

//player macros
#define PLAYERSIZE 24
#define PLAYERSIZESHOOTJUMP 32
#define PLAYERBLINKTIME 3 //every x seconds player blinks if idle
#define SPRITEFRAMETIME 0.2 //in seconds
#define PLAYERHEARTS 5
#define PLAYERHEARTSMAX 15
#define PLAYERSHOOTTIME 0.5
#define PLAYERAMMO1 37
#define PLAYERAMMO2 11
#define PLAYERLUCK 2 //it is the number of faces of the dice the player is rolling to do something
#define WALKSPEED 2.5
#define JUMPSPEED 2.75
#define GRAVITY 0.1
#define MAXFALLINGSPEED 3.0
#define INVINCIBILITYTIME 0.5
#define DAMAGESPEEDRETARDING 1.5

typedef struct PlayerTimers {

    float invincibility;
    float shoot;
    float sprite;

} PlayerTimers;

typedef struct Player {
    //vector quantities
    Vector2 position;
    Vector2 speed;
    Vector2 size;

    //scalar quantities
    int ammo_laser;
    int ammo_bazooka;
    int hearts;
    int score;
    int sprite_counter;

    //state flags
    bool shooting;
    bool on_floor;
    bool on_ceiling;
    bool blocked_right;
    bool blocked_left;
    bool vulnerable;

    PlayerTimers timer;

} Player;

struct ProjVector;
struct GameContext;
struct Projectile;

Vector2 find_player_spawn(SmartMap map);
Player init_player(SmartMap map); //initializes all the important information of the player, enemies and scenario
void is_player_blocked(Player *player, SmartMap map);
void is_player_on_map(Player *player, SmartMap map); //doesn't let the player to flee from the fight
void player_movement(Player *player);
void player_jump(Player *player, float scale);

void player_laser(Player *player, struct ProjVector *projs);
void player_bazooka(Player *player, struct ProjVector *projs);

void vulnerability_update(Player *player);
void player_damage(Player *player, char source);
void spike_damage(Player *player, SmartMap map);
bool projectile_player_hit_test(Player *player, struct Projectile proj); //tests if the player was hit by some projectile

void player_death(Player *player, DynVector enemies, struct TexturesCamera *txt_cam, SmartMap map, struct GameContext *ctx, struct ProjVector projs);
void player_death_test(Player *player, DynVector enemies, struct TexturesCamera *txt_cam, SmartMap map, struct GameContext *ctx, struct ProjVector projs);
bool is_player_on_ending_platform(Player *player, SmartMap map);

