//
//  main.c
//  infman
//
//  Created by Melchior Boaretto Neto on 14/12/24.

#include "raylib.h"
#include <stdio.h>
#include <time.h>

#define SCREENWIDTH 1200
#define SCREENHEIGHT 600
#define BUTTONWIDTH 150
#define BUTTONHEIGHT 75

int current_screen = 1; //0 for gaming, 1 to pause and 2 to main_menu --> may be initialized as 2!

//declares the global textures for the main menu


//declares the global textures for the pause menu
Texture2D resume_texture;
Texture2D main_menu_texture;

//declares the global textures for the gaming screen

typedef struct {
    //vector quantities
    Vector2 position;
    Vector2 speed;
    Vector2 size;
    
    //scalar quantities
    int ammo_laser;
    int ammo_bazooka;
    int hearts;
    int points;
    
    //collision testing
    bool on_floor;
    bool blocked_right;
    bool blocked_left;
} PLAYER;

int menu_display(void);
int leader_board_display(void);

int gaming(void);

void pause(void); //makes the pause button work
int pause_display(void); //runs the pause menu with the options: RESUME -> 0 and MAIN MENU -> 2

int main(void) {
   
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
    
    //sounds initialization
    
    
    printf("Hello, World!\n");
    
    //defines as 60 the target fps
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) {
        
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
    if (IsKeyPressed(KEY_P) && !current_screen)
        current_screen = 1;
     
    if (current_screen == 1)
        current_screen = pause_display();
}

int pause_display(void) {
 
    int option = 1;
    
    Rectangle resume = {SCREENWIDTH/2 - BUTTONWIDTH/2, SCREENHEIGHT/2 - SCREENHEIGHT/12, BUTTONWIDTH, BUTTONHEIGHT};
    Rectangle main_menu = {SCREENWIDTH/2 - BUTTONWIDTH/2, SCREENHEIGHT/2 + SCREENHEIGHT/12, BUTTONWIDTH, BUTTONHEIGHT};
    
    Vector2 mouse_pointer = {0.0f, 0.0f};
    
    int resume_hovering, main_menu_hovering;
    
    while (option == 1) {
        mouse_pointer = GetMousePosition();
        resume_hovering = 0;
        main_menu_hovering = 0;
        
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
        DrawText("RESUME", resume.x + 20, resume.y + 20, 20, BLACK);
        DrawText("MAIN MENU", main_menu.x + 10, main_menu.y + 20, 20, BLACK);
        
        EndDrawing();
        
    }
    return option;
}
