#include "game.h"
#include "raylib.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "file_handler.h"
#include "textures_and_camera.h"
#include "top_scores.h"
#include "map.h"

//DROP ammo[ENEMIES] = {0};


//miscellaneous
void laser_shoot(void); //these two are used to update the projectiles and its interactions
void bazooka_update(void);

//main function
int main() {

    //initializes the game window
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "INFman");

    // Top players leaderboard
    TopPlayer *top_players = top_players_from_bin();

    // Map from the config file
    SmartMap map = fetch_map();
    Vector2 map_extremes_float = get_map_extremes(map);
    int map_extreme_x = (int)map_extremes_float.x;
    int map_extreme_y = (int)map_extremes_float.y;

    // Camera and textures from the files
    TexturesCamera textures_and_camera;
    load_textures(&textures_and_camera.textures);
    init_camera(&textures_and_camera.camera, SCREENWIDTH, map_extreme_y);

    //initializes the audio devide (if using -> FIX THE XCODE SOUND ISSUE)
    InitAudioDevice();

    printf("Hello, World!\n");

    //defines as 60 the target fps
    SetTargetFPS(60);

    //random seed initialization
    long current_time = time(NULL);
    unsigned int current_time_reduced = (unsigned int)current_time;
    srand(current_time_reduced);

    GameContext game_context = {
        .curr_screen = menu,
        .running = true,
    };

    while (!WindowShouldClose() && game_context.running) {
        main_menu_test();
        pause(&textures_and_camera);
        gaming_test(&textures_and_camera);
    }

    //textures unloading
    unload_textures(&textures_and_camera.textures);

    //closes the audio device
    CloseAudioDevice();

    // Drop dyn allocated map
    map_drop(&map);

    // Saves the leaderboard binary file
    top_players_to_bin(top_players);

    //closes the window
    CloseWindow();

    return 0;
}

int pause_display(Camera2D player_camera) {

    int option = 1; //the "current_screen" value, changed by the player's action
    int resume_hovering, main_menu_hovering; //hovering variables

    //declares a variable to know where the user will click (duh!)
    Vector2 mouse_pointer = {0.0f, 0.0f};

    //build the rectangles representing the buttons logically
    Rectangle resume = {player.position.x + SCREENWIDTH/12, MAPHEIGHT*TILESIZE/2.0 - BUTTONHEIGHT/2/20, BUTTONWIDTH/DEFAULTZOOM, BUTTONHEIGHT/DEFAULTZOOM};
    Rectangle main_menu = {player.position.x + SCREENWIDTH/12, MAPHEIGHT*TILESIZE/2.0 + BUTTONHEIGHT/2, BUTTONWIDTH/DEFAULTZOOM, BUTTONHEIGHT/DEFAULTZOOM};

    //declaring the variables to store the position of the text drawn in the buttons
    Vector2 resume_text_position = {resume.x + 32.5/DEFAULTZOOM, resume.y + 29/DEFAULTZOOM};
    Vector2 main_menu_text_position = {main_menu.x + 16/DEFAULTZOOM, main_menu.y + 27.5/DEFAULTZOOM};

    //pause menu loop that waits for the player's action
    while (option == 1) {
        mouse_pointer = GetMousePosition();
        //automatic adjust to the mouse pointer, using the camera mode
        mouse_pointer = GetScreenToWorld2D(GetMousePosition(), player_camera);
        resume_hovering = 0;
        main_menu_hovering = 0;

        //hovering and click tests
        if (CheckCollisionPointRec(mouse_pointer, resume)) {
            resume_hovering = 1;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                option--;
        }

        if (CheckCollisionPointRec(mouse_pointer, main_menu)) {
            main_menu_hovering = 1;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                option++;
        }

        //graphical part of the pause menu
        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode2D(player_camera);

        draw_background(DARKGRAY);

        draw_map(DARKGRAY);

        draw_player(DARKGRAY);

        draw_enemies(DARKGRAY);

        draw_projectiles(bazooka_projectiles, DARKGRAY);

        draw_projectiles(laser_projectiles, DARKGRAY);

        draw_player_hearts_ammo(player.hearts, &player_camera, DARKGRAY);

        draw_level_ending_subtitles(player_camera, DARKGRAY);

        //draw resume buttons
        if (resume_hovering == 1)
            DrawTexture(resume_texture, resume.x, resume.y, GRAY);
        else
            DrawTexture(resume_texture, resume.x, resume.y, WHITE);

        //draw main menu buttons
        if (main_menu_hovering == 1)
            DrawTexture(main_menu_texture, main_menu.x, main_menu.y, GRAY);
        else
            DrawTexture(main_menu_texture, main_menu.x, main_menu.y, WHITE);

        //add text labels for the buttons
        DrawTextEx(GetFontDefault(), "RESUME", resume_text_position, FONTSIZE/DEFAULTZOOM/1.15, 1, BLACK);

        DrawTextEx(GetFontDefault(), "MAIN MENU", main_menu_text_position, FONTSIZE/DEFAULTZOOM/1.15, 1, BLACK);

        //NOTE: DrawTextEx must be used here bc DrawText smallest font would be larger than preferred

        EndMode2D();

        EndDrawing();

    }
    return option;
}

void main_menu_test(void) {
    //tests if it must enter the main menu
    if (current_screen == 2) {
        current_screen = main_menu_display();
    }
}


Vector2 txt_to_map(void) {
    int player_exists = 0;
    char read;
    FILE *fileptr;
    Vector2 player_position = {0, 0};
    enemies_counter = 0;

    if ((fileptr = fopen("../config/terrain.txt", "r")) != NULL) {
        for (int i = 0; i < MAPHEIGHT; i++) {
            for (int j = 0; j < MAPLENGTH; j++) {
                fscanf(fileptr, "%c", &read);
                if (read == '\n')
                    fscanf(fileptr, "%c", &read);
                else if (read == 'M') {
                    enemies[enemies_counter].pivot = (Vector2){j*TILESIZE, i*TILESIZE};
                    enemies[enemies_counter].position_size = (Rectangle){j*TILESIZE, i*TILESIZE, ENEMYSIZE1, ENEMYSIZE1};
                    enemies_counter++;
                }
                else if (read == 'P') {
                    player_position = (Vector2){j*TILESIZE, i*TILESIZE};
                    player_exists = 1;
                }
                map[i][j] = read;
            }
        }
        fclose(fileptr);
    } else
        do_not_exit = 0;

    if (!player_exists)
        do_not_exit = 0;

    if (enemies_counter != ENEMIES)
        do_not_exit = 0;

    return player_position;
}

void player_death_test(Camera2D player_camera) {
    if (player.hearts <= 0 && !current_screen)
        player_death(player_camera);
}

void laser_shoot(void) {

    Rectangle map_tiles[MAPHEIGHT][MAPLENGTH];

    get_map_tiles_matrix(map_tiles, true);

    for (int i = 0; i < MAXPROJECTILES; i++) {
        laser_projectiles[i].bullet.x += LASERPROJECTILESPEED*laser_projectiles[i].direction.x;
        laser_projectiles[i].bullet.y += LASERPROJECTILESPEED*laser_projectiles[i].direction.y;

        if (projectile_player_hit_test(laser_projectiles[i]) &&
            laser_projectiles[i].projectile_type == 2) {

            laser_projectiles[i].bullet.y = -SCREENWIDTH;
            player_damage('L');

        }


        for (int j = 0; j < ENEMIES; j++) {
            if (projectile_enemy_hit_test(laser_projectiles[i], enemies[j]) &&

                laser_projectiles[i].projectile_type == 0) {

                laser_projectiles[i].bullet.y = -SCREENHEIGHT;
                enemies[j].hearts--;
            }
        }

        if (check_map_tiles_collision(map_tiles, laser_projectiles[i].bullet) &&
            laser_projectiles[i].projectile_type == 0) {
            laser_projectiles[i].bullet.y = -SCREENHEIGHT;
        }

    }
}

void player_laser(void) {

    if (player.ammo_laser > 0 && IsKeyPressed(KEY_SPACE)) {
        int current_direction;
        Rectangle current_bullet;

        if (laser_projectiles_counter >= MAXPROJECTILES)
            laser_projectiles_counter = 0;

        if (player.speed.x == 0)
            current_direction = 1;
        else
            current_direction = player.speed.x/fabsf(player.speed.x);

        if (current_direction == 1) {
            current_bullet.x = player.position.x + PLAYERSIZE;
        } else
            current_bullet.x = player.position.x;

        current_bullet.y = player.position.y + 9;
        current_bullet.width = PROJECTILESIZE;
        current_bullet.height = PROJECTILESIZE/2.0;

        laser_projectiles[laser_projectiles_counter] = (PROJECTILE){current_bullet, (Vector2){current_direction, 0.0}, 0};

        laser_projectiles_counter++;
        player.ammo_laser--;
        player.shooting = 1;
    }

    if (player_shoot_timer >= PLAYERSHOOTTIME) {
        player.shooting = 0;
        player_shoot_timer = 0;
    } else if (player.shooting) {
        player_shoot_timer += GetFrameTime();
    } else
        player_shoot_timer = 0.0f;

}

bool is_player_on_ending_platform(void) {

    Rectangle player_rect = {
        player.position.x,
        player.position.y,
        player.size.x,
        player.size.y
    };

    Rectangle ending_platform_rect = {0, 0, 0, 0};

    for (int i = 0; i < MAPHEIGHT; i++) {
        for (int j = 0; j < MAPLENGTH; j++) {
            if (map[i][j] == 'L')
                ending_platform_rect = (Rectangle) {
                    TILESIZE * (j - 1),
                    TILESIZE * (i - 1),
                    TILESIZE * 3,
                    TILESIZE * 2
                };
        }
    }

    return CheckCollisionRecs(ending_platform_rect, player_rect);
}

void player_bazooka(void) {

    if (player.ammo_bazooka > 0 && IsKeyPressed(KEY_B)) {
        int current_direction;
        Rectangle current_bullet;

        if (bazooka_projectiles_counter >= MAXPROJECTILES)
            bazooka_projectiles_counter = 0;

        if (player.speed.x == 0)
            current_direction = 1;
        else
            current_direction = player.speed.x/fabsf(player.speed.x);

        if (current_direction == 1) {
            current_bullet.x = player.position.x + PLAYERSIZE;
        } else
            current_bullet.x = player.position.x;

        current_bullet.y = player.position.y + 9;
        current_bullet.width = PROJECTILESIZE*2;
        current_bullet.height = PROJECTILESIZE;

        bazooka_projectiles[bazooka_projectiles_counter] = (PROJECTILE){current_bullet, (Vector2){current_direction, player.speed.y/10.0}, 1};

        bazooka_projectiles_counter++;
        player.ammo_bazooka--;
        player.shooting = 1;
    }

}

void bazooka_update(void) {

    Rectangle map_tiles[MAPHEIGHT][MAPLENGTH];

    get_map_tiles_matrix(map_tiles, true);

    for (int i = 0; i < MAXPROJECTILES; i++) {
        //bullet movement
        bazooka_projectiles[i].bullet.x += BAZOOKAPROJECTILESPEED*bazooka_projectiles[i].direction.x;

        bazooka_projectiles[i].bullet.y += BAZOOKAPROJECTILESPEED*bazooka_projectiles[i].direction.y;

        //enemy collision check
        for (int j = 0; j < ENEMIES; j++) {
            if (projectile_enemy_hit_test(bazooka_projectiles[i], enemies[j]) &&
                bazooka_projectiles[i].projectile_type == 1) {

                bazooka_projectiles[i].bullet.y = -SCREENHEIGHT;
                enemies[j].hearts = 0;
            }
        }

        //map tiles collision
        if (check_map_tiles_collision(map_tiles, bazooka_projectiles[i].bullet) &&
            bazooka_projectiles[i].projectile_type == 1) {

            int horizontal_exp_tile = 0;
            int vertical_exp_tile = 0;


            bazooka_projectiles[i].bullet.x += PROJECTILESIZE*2;

            get_tile_on_matrix(&horizontal_exp_tile, &vertical_exp_tile, bazooka_projectiles[i].bullet);

            //explosion tile
            map[vertical_exp_tile][horizontal_exp_tile] = ' ';

            //above tile
            if (vertical_exp_tile > 0) {
                map[vertical_exp_tile - 1][horizontal_exp_tile] = ' ';
                (roll_a_dice(PLAYERLUCK) == 1) ? (map[vertical_exp_tile - 1][horizontal_exp_tile + 1] = ' ') : 0;
                (roll_a_dice(PLAYERLUCK) == 1) ? (map[vertical_exp_tile - 1][horizontal_exp_tile - 1] = ' ') : 0;
            }


            //below tile
            if (vertical_exp_tile < MAPHEIGHT - 1) {
                map[vertical_exp_tile + 1][horizontal_exp_tile] = ' ';
                (roll_a_dice(PLAYERLUCK) == 1) ? (map[vertical_exp_tile + 1][horizontal_exp_tile + 1] = ' ') : 0;
                (roll_a_dice(PLAYERLUCK) == 1) ? (map[vertical_exp_tile + 1][horizontal_exp_tile - 1] = ' ') : 0;
            }


            //adjacent tiles
            if (horizontal_exp_tile < MAPLENGTH -1)
                map[vertical_exp_tile][horizontal_exp_tile + 1] = ' ';

            if (horizontal_exp_tile > 0)
                map[vertical_exp_tile][horizontal_exp_tile - 1] = ' ';



            bazooka_projectiles[i].bullet.y = -SCREENHEIGHT;

        }

    }
}
