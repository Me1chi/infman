//
//  main.c
//  infman
//
//  Created by Melchior Boaretto Neto on 14/12/24.

#include "raylib.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

//screen, menus & button macros
#define SCREENWIDTH 1200.0
#define SCREENHEIGHT 600.0
#define BUTTONWIDTH 150.0
#define BUTTONHEIGHT 75.0
#define FONTSIZE 20.0
#define TOPLAYERS 10
#define MAXNAME 9
#define DEFAULTZOOM 3.2

//color macros
#define INFMANBLUE1 (Color){0, 136, 252, 255}
#define INFMANBLUE2 (Color){1, 248, 252, 255}

//map macros
#define MAPLENGTH 100
#define MAPHEIGHT 10
#define TILESIZE 16
#define CIRCUITPARTS 20

//player macros
#define PLAYERSIZE 24
#define PLAYERSIZESHOOTJUMP 32
#define PLAYERBLINKTIME 7 //every x seconds player blinks if idle
#define SPRITEFRAMETIME 0.2 //in seconds
#define PLAYERHEARTS 5
#define PLAYERHEARTSMAX 15
#define PLAYERSHOOTTIME 0.5
#define PLAYERAMMO1 17
#define PLAYERAMMO2 5
#define WALKSPEED 2.5
#define JUMPSPEED 2.75
#define GRAVITY 0.1
#define MAXFALLINGSPEED 3.0
#define INVINCIBILITYTIME 0.5
#define DAMAGESPEEDRETARDING 1.5

//enemy macros
#define ENEMIES 5
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

//this type is meant to be used in the top 10 ranking
typedef struct {
    char name[MAXNAME + 1]; //the + 1 comes from the NULL character
    int score;
    
} PLAYER_ON_TOP;


//global variables
int current_screen = 2; //0 for gaming, 1 to pause and 2 to main_menu --> may be initialized as 2!
int do_not_exit = 1; //defined as 1, must only be modified by the main menu EXIT button or the ERROR HANDLING functions!!
float invincibility_timer = 0;
float enemies_shooting_timer[ENEMIES] = {0};
PLAYER_ON_TOP top_players[TOPLAYERS]; //array containing the top players, filled by the reading of top_scores.bin
char map[MAPHEIGHT][MAPLENGTH]; //matrix containing the map description, filled by the reading of terrain.txt
ENEMY enemies[ENEMIES] = {0};
int enemies_counter;
PROJECTILE laser_projectiles[MAXPROJECTILES] = {0};
int laser_projectiles_counter;
DROP hearts[ENEMIES] = {0};
PLAYER player;
float player_sprite_timer = 0;
int player_sprite_counter = 0;
float player_shoot_timer = 0;

//declares the global textures for the main menu
Texture2D play_texture;
Texture2D leaderboard_texture;
Texture2D exit_texture;
Texture2D leaderboard_table_texture;
Texture2D leaderboard_main_menu_texture;

//declares the global textures for the pause menu
Texture2D resume_texture;
Texture2D main_menu_texture;

//declares the global textures for the gaming screen
Texture2D background_texture;
Texture2D floor_texture;
Texture2D obstacle_texture;
Texture2D enemy_texture;
Texture2D player_idle_texture;
Texture2D player_running_texture;
Texture2D player_running_shoot_texture;
Texture2D player_jumping_texture;
Texture2D player_teleport_texture;

Texture2D player_heart_texture;
Texture2D laser_bullet_texture;
Texture2D enemies_laser_bullet_texture;

//functions declarations (organized)

//textures related functions
void load_textures(void); //load all the game textures
void unload_textures(void); //unload all the game textures

//binaries related functions
void bin_to_top_players(void); //reads a binary file containing the top players information

//text related functions
Vector2 txt_to_map(void); //reads a binary file containing the map information and returns the player's initial position

//main menu related functions
void main_menu_test(void); //tests if the player should be in the main menu and calls main_menu_test() if it does (MUST BE THE FIRST FUNCTION TO RUN IN THE MAIN LOOP!!!
int main_menu_display(void); //displays the main menu with the options: PLAY, LEADERBOARD and EXIT
void leaderboard_display(void); //displays the leaderboard (the top5 players)

//pause menu related functions
void pause(Camera2D player_camera); //verifies if the game can be paused and calls pause_display() when P is pressed
int pause_display(Camera2D player_camera); //displays the pause menu with the options: RESUME and MAIN MENU

//gaming related functions
void gaming_test(Camera2D *player_camera);
int gaming(Camera2D *player_camera);

//drawing & camera functions
void camera_update(Camera2D *camera);
void draw_map(Color filter);
void draw_background(Color filter);
void draw_player(Color filter);
void draw_enemies(Color filter);
void draw_player_hearts(int hearts, Camera2D *player_camera);
void draw_projectiles(PROJECTILE projectile_array[], Color filter);

//player mechanics/update functions
void init_player_map(void);
void is_player_blocked(void);
void is_player_on_map(void);
void player_movement(void);
void player_jump(int scale);
void player_laser(void);
void vulnerability_update(void);
void spike_damage(void);
bool projectile_player_hit_test(PROJECTILE proj);
void player_damage(char source);
void player_death_test(Camera2D player_camera);
void player_death(Camera2D player_camera);
void player_win(void);

//enemies mechanics/update functions
void enemy_movement(void);
void enemies_meet_player(void);
bool enemy_looking_at_player(ENEMY en);
bool enemy_should_shoot(ENEMY en, int position_on_array);
void enemies_laser(void);
bool projectile_enemy_hit_test(PROJECTILE projectile, ENEMY enemy);
void enemies_drop_manager(void);

//miscellaneous
int roll_a_dice(int max_number);
void time_actions_update_bool(bool *update_var, float *timer, float goal_time);
void get_map_tiles_matrix(Rectangle map_tiles[MAPHEIGHT][MAPLENGTH], int do_spikes_matter);
bool check_map_tiles_collision(Rectangle map_tiles[MAPHEIGHT][MAPLENGTH], Rectangle object);



//main function
int main(void) {
    //functions to initialize the arrays with the files
    bin_to_top_players(); //top players initialization
    
    //initializes the game window
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "INFman");
    
    //dynamic camera initialization
    Camera2D camera = {0};
    camera.target.y = MAPHEIGHT;
    camera.offset = (Vector2){SCREENWIDTH / 6.0f, 0};
    camera.zoom = DEFAULTZOOM;
    
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
        //printf("x = %f | y = %f\n", player.position.x, player.position.y);
    }
    
    //textures unloading
    unload_textures();
    
    //closes the audio device
    CloseAudioDevice();
    
    //closes the window
    CloseWindow();
    
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
    Rectangle resume = {player.position.x + SCREENWIDTH/12, MAPHEIGHT*TILESIZE/2 - BUTTONHEIGHT/2/20, BUTTONWIDTH/DEFAULTZOOM, BUTTONHEIGHT/DEFAULTZOOM};
    Rectangle main_menu = {player.position.x + SCREENWIDTH/12, MAPHEIGHT*TILESIZE/2 + BUTTONHEIGHT/2, BUTTONWIDTH/DEFAULTZOOM, BUTTONHEIGHT/DEFAULTZOOM};
    
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
        
        draw_projectiles(laser_projectiles, DARKGRAY);
        
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
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                do_not_exit = 0; //makes the game quit
        }
        
        //graphical part of the main menu
        BeginDrawing();
        
        ClearBackground(RAYWHITE);
        
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
        DrawText("PLAY", play.x + 45, play.y + 25, FONTSIZE, BLACK);
        DrawText("LEADERBOARD", leaderboard.x + 10, leaderboard.y + 25, FONTSIZE*19/20, BLACK);
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
        
        ClearBackground(RAYWHITE);
        
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
        
        DrawText("MAIN MENU", main_menu.x + 15, main_menu.y + 25, FONTSIZE, BLACK);
        
        EndDrawing();
    }
}

void bin_to_top_players(void) {
    FILE *fileptr;
    
    if ((fileptr = fopen("/Users/melch/Desktop/Projetos/projetos_faculdade/infman/infman/bin/top_scores.bin", "r+")) != NULL) {
        fread(top_players, sizeof(PLAYER_ON_TOP), TOPLAYERS, fileptr);
    } else
        do_not_exit = 0;
    
    fclose(fileptr);
}

Vector2 txt_to_map(void) {
    int player_exists = 0;
    char read;
    FILE *fileptr;
    Vector2 player_position = {0, 0};
    enemies_counter = 0;
    
    if ((fileptr = fopen("/Users/melch/Desktop/Projetos/projetos_faculdade/infman/resources/map/terrain.txt", "r")) != NULL) {
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
            }
        }
    }
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
        hearts[i].state = 0;
        hearts[i].position = (Vector2){0.0, 0.0};
        hearts[i].drop_type = 0;
    }
}

void load_textures(void) {
    
    //main menu textures
    Image redImage = GenImageColor(BUTTONWIDTH, BUTTONHEIGHT, RED);
    play_texture = LoadTextureFromImage(redImage);
    UnloadImage(redImage);
    
    Image yellowImage = GenImageColor(BUTTONWIDTH, BUTTONHEIGHT, YELLOW);
    leaderboard_texture = LoadTextureFromImage(yellowImage);
    UnloadImage(yellowImage);
    
    Image orangeImage = GenImageColor(BUTTONWIDTH, BUTTONHEIGHT, ORANGE);
    exit_texture = LoadTextureFromImage(orangeImage);
    UnloadImage(orangeImage);
    
    Image violetImage = GenImageColor(BUTTONWIDTH*2, BUTTONHEIGHT*4, VIOLET);
    leaderboard_table_texture = LoadTextureFromImage(violetImage);
    UnloadImage(violetImage);
    
    Image greenImage1 = GenImageColor(BUTTONWIDTH, BUTTONHEIGHT, GREEN);
    leaderboard_main_menu_texture = LoadTextureFromImage(greenImage1);
    UnloadImage(greenImage1);
    
    //pause menu textures
    resume_texture = LoadTexture("/Users/melch/Desktop/Projetos/projetos_faculdade/infman/resources/map/button1.png");
    
    main_menu_texture = LoadTexture("/Users/melch/Desktop/Projetos/projetos_faculdade/infman/resources/map/button1.png");
    
    //gaming textures
    background_texture = LoadTexture("/Users/melch/Desktop/projetos/projetos_faculdade/infman/resources/map/background.png");
    
    floor_texture = LoadTexture("/Users/melch/Desktop/projetos/projetos_faculdade/infman/resources/map/tile.png");
    
    obstacle_texture = LoadTexture("/Users/melch/Desktop/projetos/projetos_faculdade/infman/resources/map/spike.png");
    
    enemy_texture = LoadTexture("/Users/melch/Desktop/projetos/projetos_faculdade/infman/resources/mobs/enemies.png");
    
    player_idle_texture = LoadTexture("/Users/melch/Desktop/projetos/projetos_faculdade/infman/resources/player/infman_idle.png");
    
    player_running_texture = LoadTexture("/Users/melch/Desktop/Projetos/projetos_faculdade/infman/resources/player/infman_running.png");
    
    player_running_shoot_texture = LoadTexture("/Users/melch/Desktop/Projetos/projetos_faculdade/infman/resources/player/infman_running_shooting.png");
    
    player_jumping_texture = LoadTexture("/Users/melch/Desktop/Projetos/projetos_faculdade/infman/resources/player/infman_jumping.png");
    
    player_heart_texture = LoadTexture("/Users/melch/Desktop/Projetos/projetos_faculdade/infman/resources/player/inf_man-heart.png");
    
    laser_bullet_texture = LoadTexture("/Users/melch/Desktop/Projetos/projetos_faculdade/infman/resources/player/player_laser_bullet.png");
    
    enemies_laser_bullet_texture = LoadTexture("/Users/melch/Desktop/Projetos/projetos_faculdade/infman/resources/mobs/enemies_laser_bullet.png");
}

void unload_textures(void) {
    //main menu textures
    UnloadTexture(play_texture);
    UnloadTexture(leaderboard_texture);
    UnloadTexture(exit_texture);
    UnloadTexture(leaderboard_table_texture);
    UnloadTexture(leaderboard_main_menu_texture);
    
    //pause menu textures
    UnloadTexture(resume_texture);
    UnloadTexture(main_menu_texture);
    
    //gaming textures
    UnloadTexture(background_texture);
    UnloadTexture(floor_texture);
    UnloadTexture(obstacle_texture);
    UnloadTexture(enemy_texture);
    UnloadTexture(player_idle_texture);
    UnloadTexture(player_running_texture);
    UnloadTexture(player_running_shoot_texture);
    UnloadTexture(player_heart_texture);
    UnloadTexture(laser_bullet_texture);
    UnloadTexture(enemies_laser_bullet_texture);
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

void player_jump(int scale) {
    player.speed.y = -JUMPSPEED*scale;
}

void is_player_on_map(void) {
    
    int horizontal_tile = floorf(player.position.x / TILESIZE);
    int vertical_tile = floorf(player.position.y / TILESIZE);

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
    
    draw_projectiles(laser_projectiles, WHITE);
    
    draw_player(WHITE);
    
    draw_enemies(WHITE);
    
    draw_player_hearts(player.hearts, player_camera);
    
    laser_shoot();
    
    EndMode2D();
    
    EndDrawing();
    
    player_movement();
    
    player_laser();
    
    enemy_movement();
    
    enemies_laser();
    
    enemies_drop_manager();
    
    camera_update(player_camera);
    
    player_death_test(*player_camera);
    
    spike_damage();
    
    vulnerability_update();
    
    return 0;
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
    
    Rectangle main_menu = {player.position.x + SCREENWIDTH/12, MAPHEIGHT*TILESIZE/2 + BUTTONHEIGHT/2, BUTTONWIDTH/DEFAULTZOOM, BUTTONHEIGHT/DEFAULTZOOM};
    
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

void player_win(void) {
    
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

void draw_player_hearts(int hearts, Camera2D *player_camera) {
    Vector2 heart_position = {-2*TILESIZE, 6*TILESIZE};
    
    EndMode2D();
    
    for (int i = 0; i < hearts; i++) {
        heart_position.x += TILESIZE * DEFAULTZOOM;
        DrawTextureEx(player_heart_texture, heart_position, 0, DEFAULTZOOM, WHITE);
    }
    
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
                    DrawTextureRec(laser_bullet_texture, (Rectangle){0,0,PROJECTILESIZE, PROJECTILESIZE/2}, (Vector2) {projectile.bullet.x, projectile.bullet.y}, filter);
                else if (projectile_array[i].direction.x == -1)
                    DrawTextureRec(laser_bullet_texture, (Rectangle){8,0,PROJECTILESIZE, PROJECTILESIZE/2}, (Vector2) {projectile.bullet.x, projectile.bullet.y}, filter);
                break;
            case 2:
                if (projectile.direction.x == 1)
                    DrawTextureRec(enemies_laser_bullet_texture, (Rectangle){0,0,PROJECTILESIZE, PROJECTILESIZE/2}, (Vector2) {projectile.bullet.x, projectile.bullet.y}, filter);
                else if (projectile_array[i].direction.x == -1)
                    DrawTextureRec(enemies_laser_bullet_texture, (Rectangle){8,0,PROJECTILESIZE, PROJECTILESIZE/2}, (Vector2) {projectile.bullet.x, projectile.bullet.y}, filter);
                break;
        }
    }
}

void spike_damage(void) {
    if (player.vulnerable) {
        int horizontal_tile = floorf((player.position.x)/TILESIZE);
        int horizontal_tile2 = floorf((player.position.x + PLAYERSIZE/2)/TILESIZE);
        int vertical_tile = floorf((player.position.y + PLAYERSIZE)/TILESIZE);
        
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
    
    for (int i = 0; i < ENEMIES; i++) {
        if (fabsf(enemies[i].position_size.x - enemies[i].pivot.x) > enemies[i].movement_range.x)
            enemies[i].speed.x = -enemies[i].speed.x;
        
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
            hearts[i].state = 1;
            hearts[i].position = enemies[i].pivot;
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
            current_bullet.height = PROJECTILESIZE/2;
            
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
        current_bullet.height = PROJECTILESIZE/2;
        
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

void player_bazooka(void) {
    
}
