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

int current_screen = 1; //0 for gaming, 1 to pause and 2 for main_menu --> may be initialized as 2!

Texture2D blue_texture;
Texture2D green_texture;


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

int menu_display(void); //
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
    
    
    //sounds initialization
    
    
    printf("Hello, World!\n");
    
    //define as 60 the target fps
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) {
        pause();
    }
    
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
    
    while (option == 1) {
        mouse_pointer = GetMousePosition();
        
        if (CheckCollisionPointRec(mouse_pointer, resume)) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                option--;
        }
        
        if (CheckCollisionPointRec(mouse_pointer, main_menu)) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                option++;
        }
        
        BeginDrawing();
        
        ClearBackground(RAYWHITE);// Draw buttons
        
                DrawTexture(blue_texture, resume.x, resume.y, WHITE);
                DrawTexture(green_texture, main_menu.x, main_menu.y, WHITE);

                // Add text labels for buttons
                DrawText("RESUME", resume.x + 20, resume.y + 20, 20, BLACK);
                DrawText("MAIN MENU", main_menu.x + 10, main_menu.y + 20, 20, BLACK);
        
        EndDrawing();
        
    }
    return option;
}
