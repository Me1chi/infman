//
//  main.c
//  infman
//
//  Created by Melchior Boaretto Neto on 14/12/24.

#include "raylib.h"
#include <stdio.h>
#include <time.h>

#define SCREENWIDTH 1200.0
#define SCREENHEIGHT 600.0
#define BUTTONWIDTH 150.0
#define BUTTONHEIGHT 75.0
#define MAPLENGTH 100
#define MAPHEIGHT 10
#define FONTSIZE 20.0
#define TOPLAYERS 10
#define MAXNAME 9

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
    bool blocked_right;
    bool blocked_left;
} PLAYER;

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

//declares the global textures for the main menu
Texture2D play_texture;
Texture2D leaderboard_texture;
Texture2D exit_texture;
Texture2D leaderboard_table_texture;

//declares the global textures for the pause menu
Texture2D resume_texture;
Texture2D main_menu_texture;

//declares the global textures for the gaming screen



//functions declarations (organized)

//binaries related functions
void bin_to_top_players(void); //reads a binary file containing the top players information

//text related functions
void txt_to_map(void); //reads a binary file containing the map information

//main menu related functions
void main_menu_test(void); //tests if the player should be in the main menu and calls main_menu_test() if it does (MUST BE THE FIRST FUNCTION TO RUN IN THE MAIN LOOP!!!
int main_menu_display(void); //displays the main menu with the options: PLAY, LEADERBOARD and EXIT
void leaderboard_display(void); //displays the leaderboard (the top5 players)

//pause menu related functions
void pause(void); //verifies if the game can be paused and calls pause_display() when P is pressed
int pause_display(void); //displays the pause menu with the options: RESUME and MAIN MENU

//gaming related functions
int gaming(void);
void draw_map(void);

//main function
int main(void) {
    //functions to initialize the arrays with the files
    bin_to_top_players(); //top players
    txt_to_map(); //map matrix
    
    //initializes the game window
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "INFman");
    
    //initializes the audio devide (if using -> FIX THE XCODE SOUND ISSUE)
    InitAudioDevice();
    
    //textures initialization
    Image blueImage = GenImageColor(BUTTONWIDTH, BUTTONHEIGHT, BLUE);
    resume_texture = LoadTextureFromImage(blueImage);
    UnloadImage(blueImage);

    Image greenImage = GenImageColor(BUTTONWIDTH, BUTTONHEIGHT, GREEN);
    main_menu_texture = LoadTextureFromImage(greenImage);
    UnloadImage(greenImage);
    
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
    //sounds initialization
    
    
    printf("Hello, World!\n");
    
    //defines as 60 the target fps
    SetTargetFPS(60);
    
    while (!WindowShouldClose() && do_not_exit) {
        main_menu_test();
        pause();
        
    }
    
    //textures unloading
    UnloadTexture(resume_texture);
    UnloadTexture(main_menu_texture);
    
    //closes the audio device
    CloseAudioDevice();
    
    //closes the window
    CloseWindow();
    
    return 0;
}

void pause(void) {
    //tests if the pause menu must open
    if (IsKeyPressed(KEY_P) && !current_screen)
        current_screen = 1;

    if (current_screen == 1)
        current_screen = pause_display();
}

int pause_display(void) {
 
    int option = 1; //the "current_screen" value, changed by the player's action
    int resume_hovering, main_menu_hovering; //hovering variables
    
    //declares a variable to know where the user will click (duh!)
    Vector2 mouse_pointer = {0.0f, 0.0f};
    
    //build the rectangles representing the buttons logically
    Rectangle resume = {SCREENWIDTH/2 - BUTTONWIDTH/2, SCREENHEIGHT/2 - SCREENHEIGHT/12, BUTTONWIDTH, BUTTONHEIGHT};
    Rectangle main_menu = {SCREENWIDTH/2 - BUTTONWIDTH/2, SCREENHEIGHT/2 + SCREENHEIGHT/12, BUTTONWIDTH, BUTTONHEIGHT};
    
    //pause menu loop that waits for the player's action
    while (option == 1) {
        mouse_pointer = GetMousePosition();
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
        DrawText("RESUME", resume.x + 30, resume.y + 20, FONTSIZE, BLACK);
        DrawText("MAIN MENU", main_menu.x + 15, main_menu.y + 25, FONTSIZE, BLACK);
        
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
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                option = 0; //starts the game
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
            DrawTexture(main_menu_texture, main_menu.x, main_menu.y, GRAY);
        else
            DrawTexture(main_menu_texture, main_menu.x, main_menu.y, WHITE);
        
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

void txt_to_map(void) {
    char lido;
    FILE *fileptr;
    
    if ((fileptr = fopen("/Users/melch/Desktop/Projetos/projetos_faculdade/infman/resources/map/terrain.txt", "r")) != NULL) {
        for (int i = 0; i < MAPHEIGHT; i++) {
            for (int j = 0; j < MAPLENGTH; j++) {
                fscanf(fileptr, "%c", &lido);
                if (lido == '\n')
                    fscanf(fileptr, "%c", &lido);
                map[i][j] = lido;
            }
        }
        fclose(fileptr);
    } else
        do_not_exit = 0;
}


