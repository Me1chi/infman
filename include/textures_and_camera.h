#include "raylib.h"

// Button macros
#define BUTTONWIDTH 150.0
#define BUTTONHEIGHT 75.0

// Color macros
#define INFMANBLUE1 (Color){0, 136, 252, 255}
#define INFMANBLUE2 (Color){1, 248, 252, 255}

#define TILESIZE 16
#define DEFAULTZOOM 3.2

typedef struct {

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

typedef struct {

    Texture2D background;
    Texture2D floor;
    Texture2D obstacle;
    Texture2D level_ending_totem;

} ScenarioTextures;

typedef struct {

    Texture2D idle;
    Texture2D running;
    Texture2D running_shoot;
    Texture2D jumping;
    Texture2D teleport;
    Texture2D heart;

} PlayerTextures;

typedef struct {

    Texture2D enemy;

} EnemyTextures;

typedef struct {

    // Actual projectiles
    Texture2D laser_bullet;
    Texture2D enemy_laser_bullet;
    Texture2D bazooka_bullet;

    // Projectiles Icons
    Texture2D laser_ammo;
    Texture2D bazooka_ammo;

} ProjectileTextures;

typedef struct {

    MenuTextures menu;
    ScenarioTextures scenario;
    PlayerTextures player;
    EnemyTextures enemy;
    ProjectileTextures projectile;

} GameTextures;

typedef struct {

    GameTextures textures;
    Camera2D camera;

} TexturesCamera;

void load_textures_testing(GameTextures *textures);
void unload_textures_testing(GameTextures *textures);

void init_camera(Camera2D* camera, float screen_width, float map_height);
