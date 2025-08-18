#include "raylib.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "file_handler.h"
#include "textures_and_camera.h"
#include "top_scores.h"

//map macros
#define MAPLENGTH 200
#define MAPHEIGHT 11
#define CIRCUITPARTS 20

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

//enemy macros
#define ENEMIES 12
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

//projectile macros
#define MAXPROJECTILES 96
#define PROJECTILESIZE 8
#define LASERPROJECTILESPEED 3.0
#define BAZOOKAPROJECTILESPEED 1.5

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

    //state testing
    bool shooting;
    bool on_floor;
    bool on_ceiling;
    bool blocked_right;
    bool blocked_left;
    bool vulnerable;

} PLAYER;

typedef struct {
    //vector quantities
    Rectangle position_size;
    Vector2 speed;
    Vector2 pivot;
    Vector2 movement_range;

    //scalar quantities
    int hearts;
    int reload_time;

    //state testing
    bool blocked_left;
    bool blocked_right;
    bool shooting;
    bool alive;
    bool met_player;

} ENEMY;

typedef struct {
    Rectangle bullet;
    Vector2 direction;
    int projectile_type; //0 for laser, 1 for bazooka, 2 for enemy shot and so on...

} PROJECTILE;

typedef struct {
    Vector2 position;
    int drop_type; //0 for heart, 1 for circuit part and so on...
    int state; //0 for unavailable, 1 for available, 2 for took

} DROP;

///global variables declaration (BEGGINING)

//game state flag variables
int current_screen = 2; //0 for gaming, 1 to pause and 2 to main_menu --> may be initialized as 2! *NOTE: 3 got used to the winning screen!! So it is reserved too

int do_not_exit = 1; //defined as 1, must only be modified by the main menu EXIT button or the ERROR HANDLING functions!!

//important timers
float invincibility_timer = 0;
float player_sprite_timer = 0;
float player_shoot_timer = 0;
float enemies_shooting_timer[ENEMIES] = {0};

//counters and arrays (related when together)
int enemies_counter;
ENEMY enemies[ENEMIES] = {0};

int laser_projectiles_counter;
PROJECTILE laser_projectiles[MAXPROJECTILES] = {0};

int bazooka_projectiles_counter;
PROJECTILE bazooka_projectiles[MAXPROJECTILES] = {0};

DROP ammo[ENEMIES] = {0};

int player_sprite_counter = 0;

//the player and the map
PLAYER player;
char map[MAPHEIGHT][MAPLENGTH]; //matrix containing the map description, filled by the reading of terrain.txt

///global variables declaration (ENDING)

///functions declaration (BEGGINING)

//text related functions
Vector2 txt_to_map(void); //reads a text file containing the map information and returns the player's initial position

//main menu related functions
void main_menu_test(void); //tests if the player should be in the main menu and calls main_menu_test() if it does (MUST BE THE FIRST FUNCTION TO RUN IN THE MAIN LOOP!!!
int main_menu_display(TexturesCamera *textures_and_camera); //displays the main menu with the options: PLAY, LEADERBOARD and EXIT
void leaderboard_display(TexturesCamera *textures_and_camera); //displays the leaderboard (the top5 players)

//pause menu related functions
void pause(TexturesCamera *textures_and_camera); //verifies if the game can be paused and calls pause_display() when P is pressed
int pause_display(TexturesCamera *textures_and_camera); //displays the pause menu with the options: RESUME and MAIN MENU

//gaming related functions
void gaming_test(TexturesCamera *textures_and_camera); //tests if the game must start
int gaming(TexturesCamera *textures_and_camera); //all of the gaming functions run inside it

//drawing & camera functions
void camera_update(TexturesCamera *textures_and_camera); //updates the camera for each frame
void draw_map(TexturesCamera *textures_and_camera, Color filter); //draws scenario tiles, obstacles and everything statical
void draw_background(TexturesCamera *textures_and_camera, Color filter); //draws a background image
void draw_player(TexturesCamera *textures_and_camera, Color filter);
void draw_enemies(TexturesCamera *textures_and_camera, Color filter);
void draw_player_hearts_ammo(TexturesCamera *textures_and_camera, int hearts, Camera2D *player_camera, Color filter);
void draw_projectiles(TexturesCamera *textures_and_camera, PROJECTILE projectile_array[], Color filter); //draw the projectiles of an array
void draw_level_ending_subtitles(TexturesCamera *textures_and_camera, Camera2D player_camera, Color filter); //draw the subtitles that must show up when the player reach the end of the level

//player mechanics/update functions
void init_player_map(void); //initializes all the important information of the player, enemies and scenario
void is_player_blocked(void); //checks player collision
void is_player_on_map(void); //doesn't let the player to flee from the fight
void player_movement(void);
void player_jump(float scale);
void player_laser(void);
void player_bazooka(void);
void vulnerability_update(void); //makes the player vulnerable again after certain time
void spike_damage(void);
bool projectile_player_hit_test(PROJECTILE proj); //tests if the player was hit by some projectile
void player_damage(char source);
void player_death_test(Camera2D player_camera); //tests if the game must end the death screen
void player_death(Camera2D player_camera); //death screen with an option to return to main menu
bool is_player_on_ending_platform(void);
void player_win(Camera2D player_camera, Color filter); //when the player reaches the end of the level, call a lot of functions to save their score

//enemies mechanics/update functions
void enemy_movement(void);
void enemies_meet_player(void); //tests if the enemy can/could see the player
bool enemy_looking_at_player(ENEMY en);
bool enemy_should_shoot(ENEMY en, int position_on_array); //use probability to return if the enemy should shoot
void enemies_laser(void);
bool projectile_enemy_hit_test(PROJECTILE projectile, ENEMY enemy);
void enemies_drop_manager(void); //IN THE FUTURE

//miscellaneous
void laser_shoot(void); //these two are used to update the projectiles and its interactions
void bazooka_update(void);
int roll_a_dice(int max_number);
void time_actions_update_bool(bool *update_var, float *timer, float goal_time); //update some variables that change within an specific time
void get_map_tiles_matrix(Rectangle map_tiles[MAPHEIGHT][MAPLENGTH], int do_spikes_matter); //get the map tiles in a rectangles matrix
bool check_map_tiles_collision(Rectangle map_tiles[MAPHEIGHT][MAPLENGTH], Rectangle object); //check all the blocks in the map tiles matrix. Searching for collision
void get_tile_on_matrix(int *hor_tile, int *ver_tile, Rectangle object); //gets the coordinate of the player in the original map matrix

///functions declartion (ENDING)


//main function
int main(void) {

    //initializes the game window
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "INFman");

    TopPlayer *top_players = top_players_from_bin();

    // TESTING

    TexturesCamera textures_and_camera;
    load_textures(&textures_and_camera.textures);
    init_camera(&textures_and_camera.camera, SCREENWIDTH, MAPHEIGHT);

    // TESTING

    //initializes the audio devide (if using -> FIX THE XCODE SOUND ISSUE)
    InitAudioDevice();

    //textures initialization
    load_textures();

    //sounds initialization


    printf("Hello, World!\n");

    //defines as 60 the target fps
    SetTargetFPS(60);

    //random seed initialization
    long current_time = time(NULL);
    unsigned int current_time_reduced = (unsigned int)current_time;
    srand(current_time_reduced);

    while (!WindowShouldClose() && do_not_exit) {
        main_menu_test();
        pause(camera);
        gaming_test(&camera);
    }

    //textures unloading
    unload_textures(&textures_and_camera.textures);

    //closes the audio device
    CloseAudioDevice();

    //closes the window
    CloseWindow();

    return 0;
}

void gaming_test(Camera2D *player_camera) {
    //tests if the game must run
    if (current_screen == 0)
        gaming(player_camera);
}

int gaming(Camera2D *player_camera) {

    is_player_blocked();

    is_player_on_map();

    BeginDrawing();

    ClearBackground(RAYWHITE);

    BeginMode2D(*player_camera);

    draw_background(WHITE);

    draw_map(WHITE);

    draw_projectiles(bazooka_projectiles, WHITE);

    draw_projectiles(laser_projectiles, WHITE);

    draw_player(WHITE);

    draw_enemies(WHITE);

    draw_player_hearts_ammo(player.hearts, player_camera, WHITE);

    player_win(*player_camera, WHITE);

    laser_shoot();

    bazooka_update();

    EndMode2D();

    EndDrawing();

    player_movement();

    player_laser();

    player_bazooka();

    enemy_movement();

    enemies_laser();

    enemies_drop_manager();

    camera_update(player_camera);

    player_death_test(*player_camera);

    spike_damage();

    vulnerability_update();

    return 0;
}

void pause(Camera2D player_camera) {
    //tests if the pause menu must open
    if (IsKeyPressed(KEY_P) && !current_screen)
        current_screen = 1;

    if (current_screen == 1)
        current_screen = pause_display(player_camera);
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

void init_player_map(void) {
    //player attributes
    player.hearts = PLAYERHEARTS;
    player.size = (Vector2){PLAYERSIZE, PLAYERSIZE};
    player.speed = (Vector2){0, 0};
    player.position = txt_to_map(); //initializes the player position and the map
    player.ammo_laser = PLAYERAMMO1;
    player.ammo_bazooka = PLAYERAMMO2;
    player.score = 0;
    player.shooting = 0;
    player.blocked_left = 0;
    player.blocked_right = 0;
    player.on_floor = 0;
    player.on_ceiling = 0;
    player.vulnerable = 0;
    invincibility_timer = 0;

    //projectiles attributes
    for (int i = 0; i < MAXPROJECTILES; i++)
        laser_projectiles[i].bullet.y = -SCREENHEIGHT;
    laser_projectiles_counter = 0;

    //enemies and its drops attributes
    for (int i = 0; i < ENEMIES; i++) {
        //enemies
        enemies[i].speed.x = (float)(ENEMYMINSPEEDX*10 + (rand() % (int)(ENEMYMAXSPEEDX*10 - ENEMYMINSPEEDX*10 + 1))/10.0);
        enemies[i].speed.y = (float)(ENEMYMINSPEEDY*10 + (rand() % (int)(ENEMYMAXSPEEDY*10 - ENEMYMINSPEEDY*10 + 1))/10.0);;
        enemies[i].movement_range.x = TILESIZE*(ENEMYMINMOVERANGEX + (rand() % (ENEMYMAXMOVERANGEX - ENEMYMINMOVERANGEX + 1)));
        enemies[i].movement_range.y = TILESIZE*(ENEMYMINMOVERANGEY + (rand() % (ENEMYMAXMOVERANGEY - ENEMYMINMOVERANGEY + 1)));
        enemies[i].hearts = ENEMYHEARTS;
        enemies[i].blocked_left = 0;
        enemies[i].blocked_right = 0;
        enemies[i].shooting = 0;
        enemies[i].alive = 1;
        enemies[i].reload_time = ENEMYRELOADTIMEMIN + (rand() % (ENEMYRELOADTIMEMAX - ENEMYRELOADTIMEMIN + 1));
        enemies[i].met_player = 0;

        //drops
        ammo[i].state = 0;
        ammo[i].position = (Vector2){0.0, 0.0};
        ammo[i].drop_type = 0;
    }
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

void player_movement(void) {
    player.position.x += player.speed.x;
    player.position.y += player.speed.y;

    //horizontal movement
    if (IsKeyDown(KEY_D) && !IsKeyDown(KEY_A) && !player.blocked_right)
        player.speed.x = WALKSPEED;
    else if (IsKeyDown(KEY_A) && !IsKeyDown(KEY_D) && !player.blocked_left)
        player.speed.x = -WALKSPEED;
    else
        player.speed.x = 0.0f;

    if (invincibility_timer > 0 && invincibility_timer < INVINCIBILITYTIME/2)
        player.speed.x = player.speed.x/DAMAGESPEEDRETARDING;

    //vertical movement
    if (player.on_floor) {
        player.speed.y = 0.0f;

        if (IsKeyDown(KEY_W))
            player_jump(1);

    } else if (player.speed.y < MAXFALLINGSPEED)
        player.speed.y += GRAVITY;
}

void player_jump(float scale) {
    player.speed.y = -JUMPSPEED*scale;
}

void is_player_on_map(void) {

    int horizontal_tile, vertical_tile;

    get_tile_on_matrix(&horizontal_tile, &vertical_tile, (Rectangle){
        player.position.x,
        player.position.y,
        player.size.x,
        player.size.y
    });

    if (vertical_tile == MAPHEIGHT + 6)
        player.hearts = 0;

    if (horizontal_tile <= 0)
        player.blocked_left = 1;

    if (horizontal_tile >= MAPLENGTH - 1)
        player.blocked_right = 1;

}

void is_player_blocked(void) {

    int on_floor_test = 0;
    int blocked_left_test = 0;
    int blocked_left_temp = 0;
    int blocked_right_test = 0;
    int blocked_right_temp = 0;

    Rectangle player_box = {
        player.position.x - 1,
        player.position.y,
        player.size.x + 2,
        player.size.y + 2
    };

    Rectangle map_tiles[MAPHEIGHT][MAPLENGTH];

    get_map_tiles_matrix(map_tiles, false);

    for (int i = 0; i < MAPHEIGHT; i++) {
        for (int j = 0; j < MAPLENGTH; j++) {
            blocked_left_temp = 0;
            blocked_right_temp = 0;

            if (CheckCollisionRecs(map_tiles[i][j], player_box)) {

                if (player_box.y + player_box.height <= map_tiles[i][j].y + 6) {
                    on_floor_test++;
                    if (player_box.y + player_box.height < map_tiles[i][j].y + 2)
                        on_floor_test += 1000;
                }

                if (player_box.x <= map_tiles[i][j].x - TILESIZE + 5 && on_floor_test == 0) {
                    blocked_right_test++;
                    blocked_right_temp = 1;
                }

                if (player_box.x >= map_tiles[i][j].x + TILESIZE - 5 && on_floor_test == 0) {
                    blocked_left_test++;
                    blocked_left_temp = 1;
                }

                if ((blocked_left_temp && blocked_right_temp) && on_floor_test == 0)
                    on_floor_test--;

            }

        }
    }

    if (on_floor_test > 0)
        player.on_floor = 1;
    else
        player.on_floor = 0;

    if (blocked_right_test > 0)
        player.blocked_right = 1;
    else
        player.blocked_right = 0;

    if (blocked_left_test > 0)
        player.blocked_left = 1;
    else
        player.blocked_left = 0;

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

void player_death(Camera2D player_camera) {
    int accept_flag = 0;
    int hovering;
    char player_score_string[MAXNAME];

    sprintf(player_score_string, "%d", player.score);

    Vector2 mouse_pointer = {0.0f, 0.0f};

    EndDrawing();

    Rectangle main_menu = {player.position.x + SCREENWIDTH/12, MAPHEIGHT*TILESIZE/2.0 + BUTTONHEIGHT/2, BUTTONWIDTH/DEFAULTZOOM, BUTTONHEIGHT/DEFAULTZOOM};

    while(accept_flag == 0) {
        mouse_pointer = GetMousePosition();
        mouse_pointer = GetScreenToWorld2D(GetMousePosition(), player_camera);
        hovering = 0;

        if (CheckCollisionPointRec(mouse_pointer, main_menu)) {
            hovering = 1;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                current_screen = 2;
                accept_flag = 1;
            }
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode2D(player_camera);

        draw_background(DARKGRAY);

        draw_map(DARKGRAY);

        draw_projectiles(bazooka_projectiles, DARKGRAY);

        draw_projectiles(laser_projectiles, DARKGRAY);

        draw_player(DARKGRAY);

        draw_enemies(DARKGRAY);

        if (hovering)
            DrawTexture(main_menu_texture, main_menu.x, main_menu.y, GRAY);
        else
            DrawTexture(main_menu_texture, main_menu.x, main_menu.y, WHITE);

        DrawTextEx(GetFontDefault(), "MAIN MENU", (Vector2){main_menu.x + 16/DEFAULTZOOM, main_menu.y + 27.5/DEFAULTZOOM}, FONTSIZE/DEFAULTZOOM/1.15, 1, BLACK);

        EndMode2D();

        DrawTextEx(GetFontDefault(), "YOU DIED", (Vector2) {12*SCREENWIDTH/33, SCREENHEIGHT/10}, 3*FONTSIZE, FONTSIZE/4, RED);

        DrawTextEx(GetFontDefault(), "SCORE: ", (Vector2){3*SCREENWIDTH/8, SCREENHEIGHT/3}, 3*FONTSIZE, FONTSIZE/4, INFMANBLUE1);

        DrawTextEx(GetFontDefault(), player_score_string, (Vector2){3*SCREENWIDTH/8 + 9*FONTSIZE + 9*FONTSIZE/4, SCREENHEIGHT/3}, 3*FONTSIZE, FONTSIZE/4, INFMANBLUE2);

        EndDrawing();
    }
}

void player_damage(char source) {
    switch (source) {
        case 'S':
            player.hearts--;
            break;

        case 'L':
            player.hearts--;
            player.speed.x = player.speed.x/10;
            break;
    }

    player.vulnerable = 0;
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

bool projectile_enemy_hit_test(PROJECTILE projectile, ENEMY enemy) {
    return CheckCollisionRecs(projectile.bullet, enemy.position_size);;
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

void spike_damage(void) {
    if (player.vulnerable) {
        int horizontal_tile = floorf((player.position.x)/TILESIZE);
        int horizontal_tile2 = floorf((player.position.x + PLAYERSIZE)/TILESIZE);
        int vertical_tile = floorf((player.position.y + PLAYERSIZE - 8)/TILESIZE);

        if (map[vertical_tile][horizontal_tile] == 'S' || map[vertical_tile][horizontal_tile2] == 'S' || map[vertical_tile - 1][horizontal_tile] == 'S' || map[vertical_tile - 1][horizontal_tile2] == 'S') {
            player_jump(1.2);
            player_damage('S');
        }
    }
}

void vulnerability_update(void) {
    time_actions_update_bool(&player.vulnerable, &invincibility_timer, INVINCIBILITYTIME);
}

void enemy_movement(void) {
    enemies_meet_player();

    Rectangle map_tiles[MAPHEIGHT][MAPLENGTH];
    get_map_tiles_matrix(map_tiles, 0);

    int horizontal_tile = 0;
    int vertical_tile = 0;

    for (int i = 0; i < ENEMIES; i++) {
        get_tile_on_matrix(&horizontal_tile, &vertical_tile, enemies[i].position_size);

        if (horizontal_tile <= 0 || horizontal_tile >= MAPLENGTH - 1)
            enemies[i].speed.x = -enemies[i].speed.x;

        if (fabsf(enemies[i].position_size.x - enemies[i].pivot.x) > enemies[i].movement_range.x)
            enemies[i].speed.x = -enemies[i].speed.x;


        if (vertical_tile <= 0 || vertical_tile >= MAPHEIGHT) {
            enemies[i].speed.y = -enemies[i].speed.y;
            if (vertical_tile <= 4)
                enemies[i].position_size.y = enemies[i].pivot.y;
        }

        if (fabsf(enemies[i].position_size.y - enemies[i].pivot.y) > enemies[i].movement_range.y ||
            check_map_tiles_collision(map_tiles, enemies[i].position_size))
            enemies[i].speed.y = -enemies[i].speed.y;


        if (enemies[i].hearts <= 0) {
            enemies[i].pivot = (Vector2){enemies[i].pivot.x, enemies[i].pivot.y};
            enemies[i].position_size.y = -SCREENHEIGHT;
        }

        if (enemy_looking_at_player(enemies[i]) &&
            enemies[i].met_player)
            enemies[i].shooting = 1;
        else
            enemies[i].shooting = 0;

        enemies[i].position_size.x += enemies[i].speed.x;
        enemies[i].position_size.y += enemies[i].speed.y;
    }
}

void enemies_drop_manager(void) {
    for (int i = 0; i < ENEMIES; i++) {
        if (enemies[i].hearts <= 0 && enemies[i].alive) {
            if (!enemies[i].met_player)
                player.score += 15;
            else if (!enemy_looking_at_player(enemies[i]))
                player.score += 50;
            else
                player.score += 100;
            ammo[i].state = 1;
            ammo[i].position = enemies[i].pivot;
            enemies[i].alive = 0;
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

void enemies_laser(void) {
    for (int i = 0; i < ENEMIES; i++) {
        if (enemy_should_shoot(enemies[i], i)) {

            int current_direction_x;
            Rectangle current_bullet;

            if (laser_projectiles_counter >= MAXPROJECTILES)
                laser_projectiles_counter = 0;

            current_direction_x = enemies[i].speed.x/fabsf(enemies[i].speed.x);

            if (current_direction_x == 1)
                current_bullet.x = enemies[i].position_size.x + ENEMYSIZE1;
            else
                current_bullet.x = enemies[i].position_size.x;

            current_bullet.y = enemies[i].position_size.y + 12;
            current_bullet.width = PROJECTILESIZE;
            current_bullet.height = PROJECTILESIZE/2.0;

            laser_projectiles[laser_projectiles_counter] = (PROJECTILE){current_bullet, (Vector2){current_direction_x, 0.0}, 2};

            laser_projectiles_counter++;
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

bool enemy_should_shoot(ENEMY en, int position_on_array) {
    int decision;

    int enemy_can_shoot = 0;

    if (enemies_shooting_timer[position_on_array] >= en.reload_time && en.shooting) {
        enemy_can_shoot = 1;
        enemies_shooting_timer[position_on_array] = 0;
    } else if (!enemy_can_shoot) {
        enemies_shooting_timer[position_on_array] += GetFrameTime();
    } else
        enemies_shooting_timer[position_on_array] = 0;

    if (roll_a_dice(CHANCEOFSHOOTING) == 1 && enemy_can_shoot)
        decision = 1;
    else
        decision = 0;

    return decision;
}

int roll_a_dice(int max_number) {
    if (max_number == 0)
        return 0;
    else
        return 1 + (rand() % max_number);
}

bool projectile_player_hit_test(PROJECTILE proj) {
    Rectangle player_collision_box = {
        player.position.x,
        player.position.y,
        player.size.x,
        player.size.x,
    };

    bool collision_test;

    if (player.vulnerable)
        collision_test = CheckCollisionRecs(player_collision_box, proj.bullet);
    else
        collision_test = 0;

    return collision_test;
}

void get_map_tiles_matrix(Rectangle map_tiles[MAPHEIGHT][MAPLENGTH], int do_spikes_matter) {

    for (int i = 0; i < MAPHEIGHT; i++) {
        for (int j = 0; j < MAPLENGTH; j++) {
            if (map[i][j] == 'O' || map[i][j] == 'S'*do_spikes_matter)
                map_tiles[i][j] = (Rectangle){
                    j * TILESIZE,
                    i * TILESIZE,
                    TILESIZE,
                    TILESIZE
                };
            else
                map_tiles[i][j] = (Rectangle){
                    0,
                    0,
                    0,
                    0
                };
        }
    }
}

bool check_map_tiles_collision(Rectangle map_tiles[MAPHEIGHT][MAPLENGTH], Rectangle object) {
    int testing = 0;

    for (int i = 0; i < MAPHEIGHT; i++) {
        for (int j = 0; j < MAPLENGTH; j++) {

            testing += CheckCollisionRecs(map_tiles[i][j], object);

        }
    }

    return testing;
}

void enemies_meet_player(void) {
    for (int i = 0; i < ENEMIES; i++) {
        if (fabsf(player.position.x - enemies[i].position_size.x) < ENEMYFOV/DEFAULTZOOM)
            enemies[i].met_player = 1;
    }
}

bool enemy_looking_at_player(ENEMY en) {
    bool test = 0;

    test = (en.speed.x < 0 && player.position.x < en.position_size.x) || (en.speed.x > 0 && player.position.x > en.position_size.x);

    return test;
}

void time_actions_update_bool(bool *update_var, float *timer, float goal_time) {
    if (*timer >= goal_time) {
        *update_var = 1;
        *timer = 0;
    } else if (!(*update_var)) {
        *timer += GetFrameTime();
    } else
        *timer = 0.0f;
}

void get_tile_on_matrix(int *hor_tile, int *ver_tile, Rectangle object) {
    *hor_tile = floorf(object.x / TILESIZE);
    *ver_tile = floorf(object.y / TILESIZE);
}

void player_win(Camera2D player_camera, Color filter) {
    if (is_player_on_ending_platform()) {
        draw_level_ending_subtitles(player_camera, filter);
        if(IsKeyPressed(KEY_L))
            current_screen = 3;
    }

    PLAYER_ON_TOP player_on_top = {0, 0};

    while (current_screen == 3) {
        player_on_top = get_user_name_score(player_camera);
    }

    applies_array_modifications(player_on_top);

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
