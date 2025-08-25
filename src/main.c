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

int pause_display(TexturesCamera *textures_and_camera); //displays the pause menu with the options: RESUME and MAIN MENU
int main_menu_display(TexturesCamera *textures_and_camera); //displays the main menu with the options: PLAY, LEADERBOARD and EXIT
void leaderboard_display(TexturesCamera *textures_and_camera); //displays the leaderboard (the top5 players)

//drawing & camera functions
void camera_update(TexturesCamera *textures_and_camera); //updates the camera for each frame
void draw_map(TexturesCamera *textures_and_camera, Color filter); //draws scenario tiles, obstacles and everything statical
void draw_background(TexturesCamera *textures_and_camera, Color filter); //draws a background image
void draw_player(TexturesCamera *textures_and_camera, Color filter);
void draw_enemies(TexturesCamera *textures_and_camera, Color filter);
void draw_player_hearts_ammo(TexturesCamera *textures_and_camera, int hearts, Camera2D *player_camera, Color filter);
void draw_projectiles(TexturesCamera *textures_and_camera, PROJECTILE projectile_array[], Color filter); //draw the projectiles of an array
void draw_level_ending_subtitles(TexturesCamera *textures_and_camera, Camera2D player_camera, Color filter); //draw the subtitles that must show up when the player reach the end of the level

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

int main_menu_display(void) {

    int option = 2; //the "current_screen" value, changed by the player's action
    int play_hovering, leaderboard_hovering, exit_hovering; //hovering variables

    //declares a variable to know where the user will click (duh!)
    Vector2 mouse_pointer = {0.0f, 0.0f};

    //build the rectangles representing the buttons logically
    Rectangle play = {SCREENWIDTH/2 - BUTTONWIDTH/2, SCREENHEIGHT/3, BUTTONWIDTH, BUTTONHEIGHT};
    Rectangle leaderboard = {SCREENWIDTH/2 - BUTTONWIDTH/2, SCREENHEIGHT/3 + SCREENHEIGHT/6, BUTTONWIDTH, BUTTONHEIGHT};
    Rectangle exit = {SCREENWIDTH/2 - BUTTONWIDTH/2, SCREENHEIGHT/3 + SCREENHEIGHT/3, BUTTONWIDTH, BUTTONHEIGHT};

    //main menu loop that closes when the player makes an action
    while (option == 2 && do_not_exit) {
        mouse_pointer = GetMousePosition();
        play_hovering = 0;
        leaderboard_hovering = 0;
        exit_hovering = 0;

        //hovering and click tests
        if (CheckCollisionPointRec(mouse_pointer, play)) {
            play_hovering = 1;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                init_player_map();
                option = 0; //starts the game
            }
        }

        if (CheckCollisionPointRec(mouse_pointer, leaderboard)) {
            leaderboard_hovering = 1;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                leaderboard_display(); //shows the leaderboard
        }

        if (CheckCollisionPointRec(mouse_pointer, exit)) {
            exit_hovering = 1;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                do_not_exit = 0; //makes the game quit
                top_players_to_bin();
            }

        }

        //graphical part of the main menu
        BeginDrawing();

        ClearBackground(DARKGRAY);

        DrawTexture(infman_logo_texture, SCREENWIDTH/2 - 495.0/2, 97/2, WHITE);

        //draw play buttons
        if (play_hovering == 1)
            DrawTexture(play_texture, play.x, play.y, GRAY);
        else
            DrawTexture(play_texture, play.x, play.y, WHITE);

        //draw leaderboard buttons
        if (leaderboard_hovering == 1)
            DrawTexture(leaderboard_texture, leaderboard.x, leaderboard.y, GRAY);
        else
            DrawTexture(leaderboard_texture, leaderboard.x, leaderboard.y, WHITE);

        //draw exit buttons
        if (exit_hovering == 1)
            DrawTexture(exit_texture, exit.x, exit.y, GRAY);
        else
            DrawTexture(exit_texture, exit.x, exit.y, WHITE);

        //add text labels for the buttons
        DrawText("PLAY", play.x + 47.5, play.y + 27.5, FONTSIZE, BLACK);
        DrawText("LEADER", leaderboard.x + 35, leaderboard.y + 20.5, FONTSIZE, BLACK);
        DrawText("BOARD", leaderboard.x + 35 + FONTSIZE/4, leaderboard.y + 20.5 + FONTSIZE, FONTSIZE, BLACK);
        DrawText("EXIT", exit.x + 50, exit.y + 25, FONTSIZE, BLACK);

        EndDrawing();
    }
    return option;
}

void leaderboard_display(void) {
    int close_leaderboard = 0;
    int hovering;
    char score_string[MAXNAME];
    Vector2 mouse_pointer = {0.0f, 0.0f};

    Rectangle leaderboard_table = {SCREENWIDTH/2 - BUTTONWIDTH, SCREENHEIGHT/2 - SCREENHEIGHT/3, BUTTONWIDTH*2, BUTTONHEIGHT*4};

    Rectangle main_menu = {SCREENWIDTH/2 - BUTTONWIDTH/2, SCREENHEIGHT/2 + SCREENHEIGHT/3, BUTTONWIDTH, BUTTONHEIGHT};

    while (!close_leaderboard) {
        mouse_pointer = GetMousePosition();
        hovering = 0;

        //makes the button that returns to main menu work
        if (CheckCollisionPointRec(mouse_pointer, main_menu)) {
            hovering = 1;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                close_leaderboard = 1;
        }

        BeginDrawing();

        ClearBackground(DARKGRAY);

        if (hovering == 1)
            DrawTexture(leaderboard_main_menu_texture, main_menu.x, main_menu.y, GRAY);
        else
            DrawTexture(leaderboard_main_menu_texture, main_menu.x, main_menu.y, WHITE);

        //draws the leaderboard itself
        DrawTexture(leaderboard_table_texture, leaderboard_table.x, leaderboard_table.y, WHITE);

        //draw the player and score titles
        DrawText("PLAYER", leaderboard_table.x + 2, leaderboard_table.y, FONTSIZE/10*9, BLACK);
        DrawText("SCORE", leaderboard_table.x + BUTTONWIDTH*2 - 60, leaderboard_table.y, FONTSIZE/10*9, BLACK);

        //draw the top players names and its scores
        for (int i = 0; i < TOPLAYERS; i++) {
            sprintf(score_string, "%d", top_players[i].score);

            DrawText(top_players[i].name, leaderboard_table.x + 2, leaderboard_table.y + FONTSIZE + i*FONTSIZE, FONTSIZE/5*4, BLACK);
            DrawText(score_string, leaderboard_table.x + BUTTONWIDTH*2 - 60, leaderboard_table.y + FONTSIZE + i*FONTSIZE, FONTSIZE/5*4, BLACK);
        }

        DrawText("MAIN", main_menu.x + 45, main_menu.y + 17.5, FONTSIZE, BLACK);
        DrawText("MENU", main_menu.x + 45, main_menu.y + 17.5 + FONTSIZE, FONTSIZE, BLACK);

        EndDrawing();
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

void draw_map(Color filter) {

    Vector2 drawing_position;

    Vector2 level_ending_toten_position = {0};

    for (int i = 0; i < MAPHEIGHT; i++) {
        for (int j = 0; j < MAPLENGTH; j++) {
            drawing_position = (Vector2){j*TILESIZE, i*TILESIZE};
            switch (map[i][j]) {
                case 'O': //draw the floor/wall standard tile
                    DrawTextureV(floor_texture, drawing_position, filter);
                    break;
                case 'S': //draw the obstacle
                    DrawTextureV(obstacle_texture, drawing_position, filter);
                    break;
                case 'L':
                    level_ending_toten_position = drawing_position;
                    break;
            }
        }
    }

    level_ending_toten_position.x -= TILESIZE;

    level_ending_toten_position.y -= TILESIZE;


    DrawTextureV(level_ending_toten_texture, level_ending_toten_position, filter);

}

void draw_background(Color filter) {
    int backgrounds_to_draw = ceilf(MAPLENGTH*TILESIZE);

    for (int i = -1; i < backgrounds_to_draw; i++) {
        DrawTexture(background_texture, 202*i, 0, filter);
    }
}

void draw_player(Color filter) {
    int rec_redim = 0;

    Rectangle source_rectangle = {
        0,
        0,
        player.size.x,
        player.size.y
    };

    player_sprite_timer += GetFrameTime();
    if (player_sprite_timer >= SPRITEFRAMETIME) {
        if (player.hearts > 0)
            player_sprite_counter++;

        if (player_sprite_counter > 2)
            player_sprite_counter = 0;

        player_sprite_timer = 0;
    }

    if (player.shooting) {
        rec_redim = PLAYERSIZESHOOTJUMP - PLAYERSIZE;
        source_rectangle.width = PLAYERSIZESHOOTJUMP;
    }

    if (!player.on_floor) {
        source_rectangle.width = PLAYERSIZESHOOTJUMP;
        source_rectangle.height = PLAYERSIZESHOOTJUMP;
    }

    source_rectangle.x = player_sprite_counter * (player.size.x + rec_redim);


    if (player.speed.x < 0)
        source_rectangle.y = player.size.y;


    if (player.speed.y != 0) {

        if (player.shooting)
            source_rectangle.x = PLAYERSIZESHOOTJUMP;
        else
            source_rectangle.x = 0;

        if (player.speed.x < 0)
            source_rectangle.y = PLAYERSIZESHOOTJUMP;
        else
            source_rectangle.y = 0;

        DrawTextureRec(player_jumping_texture, source_rectangle, player.position, filter);

    } else {

        if (player.speed.x == 0) {

            source_rectangle.x = 0;

            if (player.shooting)
                source_rectangle.x += 2*PLAYERSIZE;
            else if (time(NULL) % 7 == 0)
                source_rectangle.x += PLAYERSIZE;

            DrawTextureRec(player_idle_texture, source_rectangle, player.position, filter);

        } else {

            if (player.shooting)
                DrawTextureRec(player_running_shoot_texture, source_rectangle, player.position, filter);
            else
                DrawTextureRec(player_running_texture, source_rectangle, player.position, filter);

        }
    }
}

void camera_update(Camera2D *camera) {
    if (!current_screen || current_screen == 1) {
        camera->target.x = player.position.x;
    }
}

void player_death_test(Camera2D player_camera) {
    if (player.hearts <= 0 && !current_screen)
        player_death(player_camera);
}

void draw_player_hearts_ammo(int hearts, Camera2D *player_camera, Color filter) {
    Vector2 heart_position = {-2*TILESIZE, 6*TILESIZE};
    Vector2 laser_ammo_position = {heart_position.x + PLAYERSIZE/2.0 + TILESIZE * DEFAULTZOOM, heart_position.y + TILESIZE * DEFAULTZOOM};
    Vector2 bazooka_ammo_position = {heart_position.x + TILESIZE * DEFAULTZOOM, laser_ammo_position.y + TILESIZE * DEFAULTZOOM};

    char laser_ammo_string[8];
    char bazooka_ammo_string[8];

    //converts the ammo integers to strings
    sprintf(laser_ammo_string, "%d", player.ammo_laser);
    sprintf(bazooka_ammo_string, "%d", player.ammo_bazooka);

    EndMode2D();

    //draw the hearts
    for (int i = 0; i < hearts; i++) {
        heart_position.x += TILESIZE * DEFAULTZOOM;
        DrawTextureEx(player_heart_texture, heart_position, 0, DEFAULTZOOM, filter);
    }

    //draw the laser ammo
    DrawTextureEx(laser_ammo_texture, laser_ammo_position, 0, DEFAULTZOOM, filter);
    DrawText(laser_ammo_string, laser_ammo_position.x + TILESIZE*DEFAULTZOOM, laser_ammo_position.y, DEFAULTZOOM*FONTSIZE/2, DARKGRAY);


    //draw the bazooka ammo
    DrawTextureEx(bazooka_ammo_texture, bazooka_ammo_position, 0, DEFAULTZOOM, filter);
    DrawText(bazooka_ammo_string, bazooka_ammo_position.x + PLAYERSIZE*DEFAULTZOOM, bazooka_ammo_position.y, DEFAULTZOOM*FONTSIZE/2, DARKGRAY);

    BeginMode2D(*player_camera);
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

void draw_projectiles(PROJECTILE projectile_array[], Color filter) {

    for (int i = 0; i < MAXPROJECTILES; i++) {
        PROJECTILE projectile = projectile_array[i];

        switch (projectile.projectile_type) {
            case 0:
                if (projectile.direction.x == 1)
                    DrawTextureRec(laser_bullet_texture, (Rectangle){0,0,PROJECTILESIZE, PROJECTILESIZE/2.0}, (Vector2) {projectile.bullet.x, projectile.bullet.y}, filter);
                else if (projectile_array[i].direction.x == -1)
                    DrawTextureRec(laser_bullet_texture, (Rectangle){8,0,PROJECTILESIZE, PROJECTILESIZE/2.0}, (Vector2) {projectile.bullet.x, projectile.bullet.y}, filter);
                break;

            case 1:
                if (projectile.direction.x == 1)
                    DrawTextureRec(bazooka_bullet_texture, (Rectangle){0,0,2*PROJECTILESIZE, PROJECTILESIZE}, (Vector2) {projectile.bullet.x, projectile.bullet.y}, filter);
                else if (projectile_array[i].direction.x == -1)
                    DrawTextureRec(bazooka_bullet_texture, (Rectangle){2*PROJECTILESIZE, 0, 2*PROJECTILESIZE, PROJECTILESIZE}, (Vector2) {projectile.bullet.x, projectile.bullet.y}, filter);
                break;

            case 2:
                if (projectile.direction.x == 1)
                    DrawTextureRec(enemies_laser_bullet_texture, (Rectangle){0,0,PROJECTILESIZE, PROJECTILESIZE/2.0}, (Vector2) {projectile.bullet.x, projectile.bullet.y}, filter);
                else if (projectile_array[i].direction.x == -1)
                    DrawTextureRec(enemies_laser_bullet_texture, (Rectangle){8,0,PROJECTILESIZE, PROJECTILESIZE/2.0}, (Vector2) {projectile.bullet.x, projectile.bullet.y}, filter);
                break;
        }
    }
}

void draw_enemies(Color filter) {
    for (int i = 0; i < ENEMIES; i++) {
        ENEMY en = enemies[i];

        if (en.alive) {
            Rectangle drawing_square = {0, 0, ENEMYSIZE1, ENEMYSIZE1};

            if (en.speed.x > 0)
                drawing_square.y = ENEMYSIZE1;

            if (en.hearts <= ENEMYHEARTS/2 && PLAYERHEARTS > 1)
                drawing_square.x += 2*ENEMYSIZE1;

            if (en.shooting == 1)
                drawing_square.x += ENEMYSIZE1;

            DrawTextureRec(enemy_texture, drawing_square, (Vector2){en.position_size.x, en.position_size.y}, filter);
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

void draw_level_ending_subtitles(Camera2D player_camera, Color filter) {

    if (is_player_on_ending_platform()) {

        char player_score_string_comp[40] = "WITH YOUR SCORE -> ";

        char player_score_string[9];

        sprintf(player_score_string, "%d", player.score);

        strcat(player_score_string_comp, player_score_string);

        EndMode2D();

        DrawTextEx(GetFontDefault(), "PRESS  L  TO END THIS LEVEL", (Vector2){SCREENWIDTH/4, SCREENHEIGHT/4}, 2*FONTSIZE, 2, BLACK);
        DrawTextEx(GetFontDefault(), player_score_string_comp, (Vector2){SCREENWIDTH/4, SCREENHEIGHT/4 + 2*FONTSIZE}, 2*FONTSIZE, 2, BLACK);

        BeginMode2D(player_camera);
    }
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
