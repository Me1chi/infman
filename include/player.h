#pragma once
#include "raylib.h"
#include "projectiles.h"

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

typedef struct {
    //vector quantities
    Vector2 position;
    Vector2 speed;
    Vector2 size;

    //scalar quantities
    int ammo_laser;
    int ammo_bazooka;
    int hearts;
    int score;

    //state flags
    bool shooting;
    bool on_floor;
    bool on_ceiling;
    bool blocked_right;
    bool blocked_left;
    bool vulnerable;

} Player;

void is_player_blocked(Player *player); //checks player collision
void is_player_on_map(Player *player); //doesn't let the player to flee from the fight
void player_movement(Player *player, float invincibility_timer);
void player_jump(Player *player, float scale);

void player_laser(void); //TODO!
void player_bazooka(void);

void vulnerability_update(Player *player, float invincibility_timer); //makes the player vulnerable again after certain time

void player_damage(char source);
void spike_damage(void);
bool projectile_player_hit_test(Projectile proj); //tests if the player was hit by some projectile
void player_death_test(Camera2D player_camera); //tests if the game must end the death screen
void player_death(Camera2D player_camera); //death screen with an option to return to main menu
bool is_player_on_ending_platform(void);
