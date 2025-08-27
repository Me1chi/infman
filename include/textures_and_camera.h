#pragma once
#include "map.h"
#include "projectiles.h"
#include "raylib.h"
#include "top_scores.h"
#include "utils.h"

//screen, menus & button macros
#define SCREENWIDTH 1200.0
#define SCREENHEIGHT 600.0
#define FONTSIZE 20.0

// Button macros
#define BUTTONWIDTH 150.0
#define BUTTONHEIGHT 75.0

// Color macros
#define INFMANBLUE1 (Color){0, 136, 252, 255}
#define INFMANBLUE2 (Color){1, 248, 252, 255}

#define TILESIZE 16
#define DEFAULTZOOM 3.2

typedef struct MenuTextures {

    // Main menu textures
    Texture2D infman_logo;
    Texture2D play_button;
    Texture2D leaderboard_button;
    Texture2D exit_button;
    Texture2D leaderboard_table;
    Texture2D leaderboard_main_menu;

    // Pause menu
    Texture2D resume_button;
    Texture2D main_menu;

} MenuTextures;

typedef struct ScenarioTextures {

    Texture2D background;
    Texture2D floor;
    Texture2D obstacle;
    Texture2D level_ending_totem;

} ScenarioTextures;

typedef struct PlayerTextures {

    Texture2D idle;
    Texture2D running;
    Texture2D running_shoot;
    Texture2D jumping;
    Texture2D teleport;
    Texture2D heart;

} PlayerTextures;

typedef struct EnemyTextures {

    Texture2D enemy;

} EnemyTextures;

typedef struct ProjectileTextures {

    // Actual projectiles
    Texture2D laser_bullet;
    Texture2D enemy_laser_bullet;
    Texture2D bazooka_bullet;

    // Projectiles Icons
    Texture2D laser_ammo;
    Texture2D bazooka_ammo;

} ProjectileTextures;

typedef struct GameTextures{

    MenuTextures menu;
    ScenarioTextures scenario;
    PlayerTextures player;
    EnemyTextures enemy;
    ProjectileTextures projectile;

} GameTextures;

typedef struct TexturesCamera {

    GameTextures textures;
    Camera2D camera;

} TexturesCamera;

struct Player;
struct ProjVector;
struct GameContext;

void load_textures(GameTextures *textures);
void unload_textures(GameTextures *textures);

void init_camera(Camera2D* camera, float screen_width, float map_height);
void camera_update(Player *player, TexturesCamera *txt_cam, struct GameContext *ctx);

void draw_map(SmartMap map, TexturesCamera *txt_cam, Color filter);
void draw_background(SmartMap map, TexturesCamera *txt_cam, Color filter);
void draw_player_hearts_ammo(Player *player, TexturesCamera *txt_cam, Color filter);
void draw_projectiles(ProjVector projs, TexturesCamera *txt_cam, Color filter);
void draw_level_ending_subtitles(Player *player, SmartMap map, TexturesCamera *txt_cam, Color filter);
void draw_enemies(DynVector enemies, TexturesCamera *txt_cam, Color filter);
void draw_player(Player *player, TexturesCamera *txt_cam, Color filter);


//displays the pause menu with the options: RESUME and MAIN MENU
int pause_display(Player *player, DynVector enemies, SmartMap map, ProjVector projs, TexturesCamera *txt_cam);
void leaderboard_display(TexturesCamera *txt_cam, TopPlayer top_players[]); //displays the leaderboard (the top5 players)
int main_menu_display(Player *player,
                      DynVector *enemies,
                      ProjVector *projs,
                      SmartMap map,
                      TexturesCamera *txt_cam,
                      TopPlayer top_players[],
                      struct GameContext *ctx
                      );
 //displays the main menu with the options: PLAY, LEADERBOARD and EXIT

