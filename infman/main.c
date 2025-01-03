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

#define SCREENWIDTH 1200.0
#define SCREENHEIGHT 600.0
#define BUTTONWIDTH 150.0
#define BUTTONHEIGHT 75.0
#define MAPLENGTH 100
#define MAPHEIGHT 10
#define ENEMIES 5
#define PLAYERSIZE 16
#define ENEMYSIZE1 16
#define PLAYERAMMO1 17
#define PLAYERAMMO2 5
#define PLAYERHEARTS 3
#define JUMPSPEED 2.5
#define GRAVITY 0.1
#define WALKSPEED 2.0
#define TILESIZE 16
#define FONTSIZE 20.0
#define TOPLAYERS 10
#define MAXPROJECTILES 96
#define LASERPROJECTILESPEED 3.0
#define PROJECTILESIZE 8
#define MAXNAME 9
#define DEFAULTZOOM 3.2
#define INFMANBLUE1 (Color){0, 136, 252, 255}
#define INFMANBLUE2 (Color){1, 248, 252, 255}

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
    bool laser_shooting;
    bool bazooka_shooting;
    bool on_floor;
    bool on_ceiling;
    bool blocked_right;
    bool blocked_left;
} PLAYER;

typedef struct {
    //vector quantities
    Vector2 position;
    Vector2 speed;
    
    //scalar quantities
    int hearts;
    
    //state testing
    bool blocked_left;
    bool blocked_right;
    bool shooting;
} ENEMY;

typedef struct {
    Rectangle bullet;
    Color bullet_type;
    int direction;
    
} PROJECTILE;

//this type is meant to be used in the top 10 ranking
typedef struct {
    char name[MAXNAME + 1]; //the + 1 comes from the NULL character
    int score;
} PLAYER_ON_TOP;


//global variables
int current_screen = 2; //0 for gaming, 1 to pause and 2 to main_menu --> may be initialized as 2!
int do_not_exit = 1; //defined as 1, must only be modified by the main menu EXIT button or the ERROR HANDLING functions!!
PLAYER_ON_TOP top_players[TOPLAYERS]; //array containing the top players, filled by the reading of top_scores.bin
char map[MAPHEIGHT][MAPLENGTH]; //matrix containing the map description, filled by the reading of terrain.txt
PLAYER player;
ENEMY enemies[ENEMIES];
int enemies_counter;
PROJECTILE laser_projectiles[MAXPROJECTILES] = {0};
int laser_projectiles_counter;

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
Texture2D player_texture;
Texture2D player_heart_texture;

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
void init_player_map(void);
void draw_map(Color filter);
void draw_background(Color filter);
void draw_player(void);
void player_movement(void);
void player_jump(int scale);
void is_player_on(char option); //Collision test: L or R -> left or right, C or F -> ceiling or floor
void camera_update(Camera2D *camera);
void player_death_test(Camera2D player_camera);
void player_death(Camera2D player_camera);
void player_win(void);
void player_damage(void);
void draw_player_hearts(int hearts, Camera2D *player_camera);
void laser_shoot(void);
bool projectile_enemy_hit_test(PROJECTILE projectile, ENEMY enemy);

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
    Vector2 resume_text_position = {resume.x + 27.5/DEFAULTZOOM, resume.y + 25/DEFAULTZOOM};
    Vector2 main_menu_text_position = {main_menu.x + 10/DEFAULTZOOM, main_menu.y + 25/DEFAULTZOOM};
    
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
        DrawTextEx(GetFontDefault(), "RESUME", resume_text_position, FONTSIZE/DEFAULTZOOM, 1, BLACK);
        
        DrawTextEx(GetFontDefault(), "MAIN MENU", main_menu_text_position, FONTSIZE/DEFAULTZOOM, 1, BLACK);
        
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
    Rectangle play = {SCREENWIDTH/2 - BUTTONWIDTH/2, SCREENHEIGHT/3, 2*BUTTONWIDTH, BUTTONHEIGHT};
    Rectangle leaderboard = {SCREENWIDTH/2 - BUTTONWIDTH/2, SCREENHEIGHT/3 + SCREENHEIGHT/6, 2*BUTTONWIDTH, BUTTONHEIGHT};
    Rectangle exit = {SCREENWIDTH/2 - BUTTONWIDTH/2, SCREENHEIGHT/3 + SCREENHEIGHT/3, 2*BUTTONWIDTH, BUTTONHEIGHT};
    
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
    
    if ((fileptr = fopen("/Users/melch/Desktop/Projetos/projetos_faculdade/infman/resources/map/terrain.txt", "r")) != NULL) {
        for (int i = 0; i < MAPHEIGHT; i++) {
            for (int j = 0; j < MAPLENGTH; j++) {
                fscanf(fileptr, "%c", &read);
                if (read == '\n')
                    fscanf(fileptr, "%c", &read);
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
                case 'M': //draw the enemies
                    DrawTextureV(enemy_texture, drawing_position, filter);
                    break;
                case 'P': //draw the player
                    DrawTextureV(player_texture, player.position, filter);
                    //DrawTexture(player_texture, player.position.x - 8, player.position.y - 8, filter);
                    break;
            }
        }
    }
}

void init_player_map(void) {
    player.hearts = PLAYERHEARTS;
    player.size = (Vector2){PLAYERSIZE, PLAYERSIZE};
    player.speed = (Vector2){0, 0};
    player.position = txt_to_map(); //initializes the player position and the map
    player.ammo_laser = PLAYERAMMO1;
    player.ammo_bazooka = PLAYERAMMO2;
    player.score = 0;
    player.laser_shooting = 0;
    player.bazooka_shooting = 0;
    player.blocked_left = 0;
    player.blocked_right = 0;
    player.on_floor = 0;
    player.on_ceiling = 0;
    
    laser_projectiles_counter = 0;
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
    Image blueImage = GenImageColor(BUTTONWIDTH/DEFAULTZOOM, BUTTONHEIGHT/DEFAULTZOOM, BLUE);
    resume_texture = LoadTextureFromImage(blueImage);
    UnloadImage(blueImage);

    Image greenImage = GenImageColor(BUTTONWIDTH/DEFAULTZOOM, BUTTONHEIGHT/DEFAULTZOOM, GREEN);
    main_menu_texture = LoadTextureFromImage(greenImage);
    UnloadImage(greenImage);
    
    //gaming textures
    background_texture = LoadTexture("/Users/melch/Desktop/projetos/projetos_faculdade/infman/resources/map/background.png");
    
    floor_texture = LoadTexture("/Users/melch/Desktop/projetos/projetos_faculdade/infman/resources/map/tile1.png");
    
    obstacle_texture = LoadTexture("/Users/melch/Desktop/projetos/projetos_faculdade/infman/resources/map/spike.png");
    
    Image darkPurpleImage = GenImageColor(PLAYERSIZE, PLAYERSIZE, DARKPURPLE);
    enemy_texture = LoadTextureFromImage(darkPurpleImage);
    UnloadImage(darkPurpleImage);
    
    Image beigeImage = GenImageColor(PLAYERSIZE, PLAYERSIZE, BEIGE);
    player_texture = LoadTextureFromImage(beigeImage);
    UnloadImage(beigeImage);
    
    player_heart_texture = LoadTexture("/Users/melch/Desktop/Projetos/projetos_faculdade/infman/resources/player/inf_man-heart.png");
}

void unload_textures(void) {
    //main menu textures
    UnloadTexture(player_texture);
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
    UnloadTexture(player_texture);
    UnloadTexture(player_heart_texture);
}

void draw_background(Color filter) {
    int backgrounds_to_draw = ceilf(MAPLENGTH*TILESIZE);
    
    for (int i = -1; i < backgrounds_to_draw; i++) {
        DrawTexture(background_texture, 202*i, 0, filter);
    }
}

void draw_player(void) {
    //6 possible sprites: stand, walking right/left, shooting laser/bazooka (but they can be combined)
    if (player.speed.x == 0) {
        
    }
}

void player_movement(void) {
    player.position.x += player.speed.x;
    player.position.y += player.speed.y;
    
    //horizontal movement
    if (IsKeyDown(KEY_D) && !player.blocked_right)
        player.speed.x = 1.5f;
    else if (IsKeyDown(KEY_A)  && !player.blocked_left)
        player.speed.x = -1.5f;
    else
        player.speed.x = 0.0f;
    
    //vertical movement
    if (player.on_floor) {
        player.speed.y = 0.0f;
        
        if (IsKeyDown(KEY_SPACE)) {
            player_jump(1);
        }
    } else
        player.speed.y += GRAVITY;
}

void player_jump(int scale) {
    player.speed.y = -JUMPSPEED*scale;
}

void is_player_on(char option) {
    int out_of_limits_x = 0;
    int out_of_limits_y = 0;
    int out_of_limits = 0;
    
    int horizontal_tile = floorf(player.position.x / TILESIZE);
    int vertical_tile = floorf(player.position.y / TILESIZE);
    
    //checks if the player is on the map;
    if (horizontal_tile <= 0 || horizontal_tile >= MAPLENGTH)
        out_of_limits_x = 1;
    
    if (vertical_tile <= 0 || vertical_tile >= MAPHEIGHT) {
        out_of_limits_y = 1;
    }
    
    out_of_limits = out_of_limits_x + out_of_limits_y;
    
    switch (option) {
        case 'L':
            if (!out_of_limits && horizontal_tile > 0) {
                if (map[vertical_tile][horizontal_tile] == 'O') {
                    player.blocked_left = 1;
                } else
                    player.blocked_left = 0;
            } else if (horizontal_tile == 0) {
                player.blocked_left = 1;
            }
            break;
            
        case 'R':
            if (!out_of_limits && horizontal_tile < MAPLENGTH - 1) {
                if (map[vertical_tile][horizontal_tile + 1] == 'O') {
                    player.blocked_right = 1;
                } else
                    player.blocked_right = 0;
            } else if (horizontal_tile == MAPLENGTH - 1) {
                player.blocked_right = 1;
            }
            break;
            
        case 'C':
            if (!out_of_limits_y && vertical_tile > 0) {
                if (map[vertical_tile - 1][horizontal_tile] == 'O' || map[vertical_tile - 1][horizontal_tile + 1] == 'O') {
                    player.on_ceiling = 1;
                } else
                    player.on_ceiling = 0;
            } else
                player.on_ceiling = 1;
            break;
            
        case 'F':
            if (!out_of_limits_y && vertical_tile < MAPHEIGHT - 1) {
                if (map[vertical_tile + 1][horizontal_tile] == 'O' || map[vertical_tile + 1][horizontal_tile + 1] == 'O') {
                    player.on_floor = 1;
                } else
                    player.on_floor = 0;
            } else if (vertical_tile == MAPHEIGHT + 6) {
                player.hearts = 0;
            }
            break;
    }
}

void gaming_test(Camera2D *player_camera) {
    //tests if the game must run
    if (current_screen == 0)
        gaming(player_camera);
}

int gaming(Camera2D *player_camera) {
    is_player_on('R');
    is_player_on('L');
    is_player_on('C');
    is_player_on('F');
    
    BeginDrawing();
    
    ClearBackground(RAYWHITE);
    
    BeginMode2D(*player_camera);
    
    draw_background(WHITE);
    
    draw_map(WHITE);
    
    draw_player_hearts(player.hearts, player_camera);
    
    laser_shoot();
    
    EndMode2D();
    
    EndDrawing();
    
    player_movement();
    
    camera_update(player_camera);
    
    player_death_test(*player_camera);
    
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
        
        DrawTextEx(GetFontDefault(), "MAIN MENU", (Vector2){main_menu.x + 10/DEFAULTZOOM, main_menu.y + 25/DEFAULTZOOM}, FONTSIZE/DEFAULTZOOM, 1, BLACK);
        
        EndMode2D();
        
        DrawTextEx(GetFontDefault(), "YOU DIED", (Vector2) {12*SCREENWIDTH/33, SCREENHEIGHT/10}, 3*FONTSIZE, FONTSIZE/4, RED);

        DrawTextEx(GetFontDefault(), "SCORE: ", (Vector2){3*SCREENWIDTH/8, SCREENHEIGHT/3}, 3*FONTSIZE, FONTSIZE/4, INFMANBLUE1);
        
        DrawTextEx(GetFontDefault(), player_score_string, (Vector2){3*SCREENWIDTH/8 + 9*FONTSIZE + 9*FONTSIZE/4, SCREENHEIGHT/3}, 3*FONTSIZE, FONTSIZE/4, INFMANBLUE2);
        
        EndDrawing();
    }
}

void player_win(void) {
    
}

void player_damage(void) {
    
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
    if (player.ammo_laser > 0 && IsKeyPressed(KEY_E)) {
        int current_direction;
        
        if (laser_projectiles_counter >= MAXPROJECTILES)
            laser_projectiles_counter = 0;
        
        if (player.speed.x == 0)
            current_direction = 1;
        else
            current_direction = player.speed.x/fabsf(player.speed.x);
        
        Rectangle current_bullet = {player.position.x + PLAYERSIZE, player.position.y + PLAYERSIZE/2, PROJECTILESIZE, PROJECTILESIZE/2};
        
        laser_projectiles[laser_projectiles_counter] = (PROJECTILE){current_bullet, YELLOW, current_direction};
        
        laser_projectiles_counter++;
        player.ammo_laser--;
    }
    
    for (int i = 0; i < MAXPROJECTILES; i++) {
        laser_projectiles[i].bullet.x += LASERPROJECTILESPEED*laser_projectiles[i].direction;
        
        DrawRectangle(laser_projectiles[i].bullet.x, laser_projectiles[i].bullet.y, PROJECTILESIZE, PROJECTILESIZE/2, laser_projectiles[i].bullet_type);
        
        for (int j = 0; j < ENEMIES; j++) {
            if (projectile_enemy_hit_test(laser_projectiles[i], enemies[j])) {
                laser_projectiles[i].bullet.x = MAPLENGTH + SCREENWIDTH;
                enemies[j].hearts--;
            }
        }
    }
}

bool projectile_enemy_hit_test(PROJECTILE projectile, ENEMY enemy) {
    Rectangle enemy_rectangle = {enemy.position.x, enemy.position.y, ENEMYSIZE1, ENEMYSIZE1};
    
    return CheckCollisionRecs(projectile.bullet, enemy_rectangle);;
}


