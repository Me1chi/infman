#include "textures_and_camera.h"
#include "file_handler.h"
#include "raylib.h"

void load_textures(GameTextures *textures) {

    // Menu
    textures->menu.infman_logo = LoadTexture(FROM_RESOURCES("map/inf_man_logo.png"));
    textures->menu.play_button = LoadTexture(FROM_RESOURCES("map/button0.png"));
    textures->menu.leaderboard_button = LoadTexture(FROM_RESOURCES("map/button0.png"));
    textures->menu.exit_button = LoadTexture(FROM_RESOURCES("map/button0.png"));

    Image infmanblueImage = GenImageColor(BUTTONWIDTH*2, BUTTONHEIGHT*4, INFMANBLUE1);
    textures->menu.leaderboard_table = LoadTextureFromImage(infmanblueImage);
    UnloadImage(infmanblueImage);

    textures->menu.leaderboard_main_menu = LoadTexture(FROM_RESOURCES("map/button0.png"));
    textures->menu.resume_button = LoadTexture(FROM_RESOURCES("map/button1_esp.png"));
    textures->menu.main_menu = LoadTexture(FROM_RESOURCES("map/button1_esp.png"));

    // Scenario
    textures->scenario.background = LoadTexture(FROM_RESOURCES("map/background.png"));
    textures->scenario.floor = LoadTexture(FROM_RESOURCES("map/tile.png"));
    textures->scenario.obstacle = LoadTexture(FROM_RESOURCES("map/spike.png"));
    textures->scenario.level_ending_totem = LoadTexture(FROM_RESOURCES("map/level_ending_toten.png"));

    // Player
    textures->player.idle = LoadTexture(FROM_RESOURCES("player/infman_idle.png"));
    textures->player.running = LoadTexture(FROM_RESOURCES("player/infman_running.png"));
    textures->player.running_shoot = LoadTexture(FROM_RESOURCES("player/infman_running_shooting.png"));
    textures->player.jumping = LoadTexture(FROM_RESOURCES("player/infman_jumping.png"));
    textures->player.teleport = textures->player.jumping; // Here I don't have the texture ;(
    textures->player.heart = LoadTexture(FROM_RESOURCES("player/infman-heart.png"));

    // Enemy
    textures->enemy.enemy = LoadTexture(FROM_RESOURCES("mobs/enemies.png"));

    // Projectile
    textures->projectile.laser_ammo = LoadTexture(FROM_RESOURCES("player/laser_ammo.png"));
    textures->projectile.enemy_laser_bullet = LoadTexture(FROM_RESOURCES("mobs/enemies_laser_bullet.png"));
    textures->projectile.bazooka_ammo = LoadTexture(FROM_RESOURCES("player/bazooka_bullet.png"));
    textures->projectile.laser_ammo = LoadTexture(FROM_RESOURCES("player/laser_ammo.png"));
    textures->projectile.bazooka_ammo = LoadTexture(FROM_RESOURCES("player/bazooka_ammo.png"));

}

void unload_textures(GameTextures* textures) {

    // Menu
    UnloadTexture(textures->menu.infman_logo);
    UnloadTexture(textures->menu.play_button);
    UnloadTexture(textures->menu.leaderboard_button);
    UnloadTexture(textures->menu.exit_button);
    UnloadTexture(textures->menu.leaderboard_table);
    UnloadTexture(textures->menu.leaderboard_main_menu);
    UnloadTexture(textures->menu.resume_button);
    UnloadTexture(textures->menu.main_menu);


    // Scenario
    UnloadTexture(textures->scenario.background);
    UnloadTexture(textures->scenario.floor);
    UnloadTexture(textures->scenario.obstacle);
    UnloadTexture(textures->scenario.level_ending_totem);

    // Player
    UnloadTexture(textures->player.idle);
    UnloadTexture(textures->player.running);
    UnloadTexture(textures->player.running_shoot);
    UnloadTexture(textures->player.jumping);
    UnloadTexture(textures->player.teleport);
    UnloadTexture(textures->player.heart);

    // Enemy
    UnloadTexture(textures->enemy.enemy);

    // Projectile
    UnloadTexture(textures->projectile.laser_ammo);
    UnloadTexture(textures->projectile.enemy_laser_bullet);
    UnloadTexture(textures->projectile.bazooka_ammo);
    UnloadTexture(textures->projectile.laser_ammo);
    UnloadTexture(textures->projectile.bazooka_ammo);

}

void init_camera(Camera2D* camera, float screen_width, float map_height) {
    camera->target.y = map_height;
    camera->offset = (Vector2){screen_width / 6.0f, 0};
    camera->zoom = DEFAULTZOOM;
}

