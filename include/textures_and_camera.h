#pragma once
#include "enemies.h"
#include "file_handler.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include "projectiles.h"
#include "raylib.h"
#include "top_scores.h"
#include "utils.h"
#include <string.h>
#include <time.h>

//screen, menus & button macros
#define SCREENWIDTH 1200.0
#define SCREENHEIGHT 600.0
#define FONTSIZE 20.0

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

void load_textures(GameTextures *textures);
void unload_textures(GameTextures *textures);

void init_camera(Camera2D* camera, float screen_width, float map_height);
void camera_update(Player *player, TexturesCamera *txt_cam, GameContext *ctx);

void draw_map(SmartMap map, TexturesCamera *txt_cam, Color filter);
void draw_background(SmartMap map, TexturesCamera *txt_cam, Color filter);
void draw_player_hearts_ammo(Player *player, TexturesCamera *txt_cam, Color filter);
void draw_projectiles(ProjVector projs, TexturesCamera *txt_cam, Color filter);
void draw_level_ending_subtitles(Player *player, TexturesCamera *txt_cam, Color filter);
void draw_enemies(DynVector enemies, TexturesCamera *txt_cam, Color filter);
void draw_player(Player *player, TexturesCamera *txt_cam, Color filter);


//displays the pause menu with the options: RESUME and MAIN MENU
int pause_display(Player *player, DynVector enemies, SmartMap map, ProjVector projs, TexturesCamera *txt_cam);
void leaderboard_display(TexturesCamera *txt_cam, TopPlayer top_players[]); //displays the leaderboard (the top5 players)
int main_menu_display(Player *player,
                      DynVector *enemies,
                      ProjVector *projs,
                      SmartMap map,
                      TexturesCamera *txt_cam,
                      TopPlayer top_players[],
                      GameContext *ctx
                      );
 //displays the main menu with the options: PLAY, LEADERBOARD and EXIT

void camera_update(Player *player, TexturesCamera *txt_cam, GameContext *ctx) {
    if (!ctx->curr_screen || ctx->curr_screen == 1) {
        txt_cam->camera.target.x = player->position.x;
    }
}

void draw_map(SmartMap map, TexturesCamera *txt_cam, Color filter) {

    Vector2 drawing_position;
    Vector2 level_ending_toten_position = {0};

    Vector2 extremes = get_map_extremes(map);
    int map_ex = extremes.x;
    int map_ey = extremes.y;

    for (int i = 0; i < map_ey; i++) {
        for (int j = 0; j < matrix_row_len(map, i); j++) {
            drawing_position = (Vector2){j*TILESIZE, i*TILESIZE};

            char *charptr = matrix_get(map, i, j);

            switch (*charptr) {
                case 'O': //draw the floor/wall standard tile
                    DrawTextureV(txt_cam->textures.scenario.floor, drawing_position, filter);
                    break;
                case 'S': //draw the obstacle
                    DrawTextureV(txt_cam->textures.scenario.obstacle, drawing_position, filter);
                    break;
                case 'L':
                    level_ending_toten_position = drawing_position;
                    break;
            }
        }
    }

    level_ending_toten_position.x -= TILESIZE;

    level_ending_toten_position.y -= TILESIZE;


    DrawTextureV(txt_cam->textures.scenario.level_ending_totem, level_ending_toten_position, filter);

}

void draw_background(SmartMap map, TexturesCamera *txt_cam, Color filter) {
    int backgrounds_to_draw = ceilf(get_map_extremes(map).x*TILESIZE);

    for (int i = -1; i < backgrounds_to_draw; i++) {
        DrawTexture(txt_cam->textures.scenario.background, 202*i, 0, filter);
    }
}

void draw_player_hearts_ammo(Player *player, TexturesCamera *txt_cam, Color filter) {
    Vector2 heart_position = {-2*TILESIZE, 6*TILESIZE};
    Vector2 laser_ammo_position = {heart_position.x + PLAYERSIZE/2.0 + TILESIZE * DEFAULTZOOM, heart_position.y + TILESIZE * DEFAULTZOOM};
    Vector2 bazooka_ammo_position = {heart_position.x + TILESIZE * DEFAULTZOOM, laser_ammo_position.y + TILESIZE * DEFAULTZOOM};

    char laser_ammo_string[8];
    char bazooka_ammo_string[8];

    //converts the ammo integers to strings
    sprintf(laser_ammo_string, "%d", player->ammo_laser);
    sprintf(bazooka_ammo_string, "%d", player->ammo_bazooka);

    EndMode2D();

    //draw the hearts
    for (int i = 0; i < player->hearts; i++) {
        heart_position.x += TILESIZE * DEFAULTZOOM;
        DrawTextureEx(txt_cam->textures.player.heart, heart_position, 0, DEFAULTZOOM, filter);
    }

    //draw the laser ammo
    DrawTextureEx(txt_cam->textures.projectile.laser_ammo, laser_ammo_position, 0, DEFAULTZOOM, filter);
    DrawText(laser_ammo_string, laser_ammo_position.x + TILESIZE*DEFAULTZOOM, laser_ammo_position.y, DEFAULTZOOM*FONTSIZE/2, DARKGRAY);


    //draw the bazooka ammo
    DrawTextureEx(txt_cam->textures.projectile.bazooka_ammo, bazooka_ammo_position, 0, DEFAULTZOOM, filter);
    DrawText(bazooka_ammo_string, bazooka_ammo_position.x + PLAYERSIZE*DEFAULTZOOM, bazooka_ammo_position.y, DEFAULTZOOM*FONTSIZE/2, DARKGRAY);

    BeginMode2D(txt_cam->camera);
}

void draw_projectiles(ProjVector projs, TexturesCamera *txt_cam, Color filter) {

    for (int i = 0; i < projs.max; i++) {
        Projectile projectile = projs.arr[i];

        switch (projectile.projectile_type) {
            case 0:
                if (projectile.direction.x == 1)
                    DrawTextureRec(txt_cam->textures.projectile.laser_bullet, (Rectangle){0,0,PROJECTILESIZE, PROJECTILESIZE/2.0}, (Vector2) {projectile.bullet.x, projectile.bullet.y}, filter);
                else if (projs.arr[i].direction.x == -1)
                    DrawTextureRec(txt_cam->textures.projectile.laser_bullet, (Rectangle){8,0,PROJECTILESIZE, PROJECTILESIZE/2.0}, (Vector2) {projectile.bullet.x, projectile.bullet.y}, filter);
                break;

            case 1:
                if (projectile.direction.x == 1)
                    DrawTextureRec(txt_cam->textures.projectile.bazooka_bullet, (Rectangle){0,0,2*PROJECTILESIZE, PROJECTILESIZE}, (Vector2) {projectile.bullet.x, projectile.bullet.y}, filter);
                else if (projs.arr[i].direction.x == -1)
                    DrawTextureRec(txt_cam->textures.projectile.bazooka_bullet, (Rectangle){2*PROJECTILESIZE, 0, 2*PROJECTILESIZE, PROJECTILESIZE}, (Vector2) {projectile.bullet.x, projectile.bullet.y}, filter);
                break;

            case 2:
                if (projectile.direction.x == 1)
                    DrawTextureRec(txt_cam->textures.projectile.enemy_laser_bullet, (Rectangle){0,0,PROJECTILESIZE, PROJECTILESIZE/2.0}, (Vector2) {projectile.bullet.x, projectile.bullet.y}, filter);
                else if (projs.arr[i].direction.x == -1)
                    DrawTextureRec(txt_cam->textures.projectile.enemy_laser_bullet, (Rectangle){8,0,PROJECTILESIZE, PROJECTILESIZE/2.0}, (Vector2) {projectile.bullet.x, projectile.bullet.y}, filter);
                break;
        }
    }
}

void draw_level_ending_subtitles(Player *player, TexturesCamera *txt_cam, Color filter){

    if (is_player_on_ending_platform()) {

        char player_score_string_comp[40] = "WITH YOUR SCORE -> ";

        char player_score_string[9];

        sprintf(player_score_string, "%d", player->score);

        strcat(player_score_string_comp, player_score_string);

        EndMode2D();

        DrawTextEx(GetFontDefault(), "PRESS  L  TO END THIS LEVEL", (Vector2){SCREENWIDTH/4, SCREENHEIGHT/4}, 2*FONTSIZE, 2, BLACK);
        DrawTextEx(GetFontDefault(), player_score_string_comp, (Vector2){SCREENWIDTH/4, SCREENHEIGHT/4 + 2*FONTSIZE}, 2*FONTSIZE, 2, BLACK);

        BeginMode2D(txt_cam->camera);
    }
}

void draw_enemies(DynVector enemies, TexturesCamera *txt_cam, Color filter) {
    for (int i = 0; i < vector_len(enemies); i++) {
        Enemy *enptr = vector_get(enemies, i);

        if (enptr->alive) {
            Rectangle drawing_square = {0, 0, ENEMYSIZE1, ENEMYSIZE1};

            if (enptr->speed.x > 0)
                drawing_square.y = ENEMYSIZE1;

            if (enptr->hearts <= ENEMYHEARTS/2 && PLAYERHEARTS > 1)
                drawing_square.x += 2*ENEMYSIZE1;

            if (enptr->shooting == 1)
                drawing_square.x += ENEMYSIZE1;

            DrawTextureRec(txt_cam->textures.enemy.enemy, drawing_square, (Vector2){enptr->position_size.x, enptr->position_size.y}, filter);
        }
    }
}

void draw_player(Player *player, TexturesCamera *txt_cam, Color filter) {
    int rec_redim = 0;

    Rectangle source_rectangle = {
        0,
        0,
        player->size.x,
        player->size.y
    };

    player->timer.sprite += GetFrameTime();
    if (player->timer.sprite >= SPRITEFRAMETIME) {
        if (player->hearts > 0)
            player->sprite_counter++;

        if (player->sprite_counter > 2)
            player->sprite_counter = 0;

        player->timer.sprite = 0;
    }

    if (player->shooting) {
        rec_redim = PLAYERSIZESHOOTJUMP - PLAYERSIZE;
        source_rectangle.width = PLAYERSIZESHOOTJUMP;
    }

    if (!player->on_floor) {
        source_rectangle.width = PLAYERSIZESHOOTJUMP;
        source_rectangle.height = PLAYERSIZESHOOTJUMP;
    }

    source_rectangle.x = player->sprite_counter * (player->size.x + rec_redim);


    if (player->speed.x < 0)
        source_rectangle.y = player->size.y;


    if (player->speed.y != 0) {

        if (player->shooting)
            source_rectangle.x = PLAYERSIZESHOOTJUMP;
        else
            source_rectangle.x = 0;

        if (player->speed.x < 0)
            source_rectangle.y = PLAYERSIZESHOOTJUMP;
        else
            source_rectangle.y = 0;

        DrawTextureRec(txt_cam->textures.player.jumping, source_rectangle, player->position, filter);

    } else {

        if (player->speed.x == 0) {

            source_rectangle.x = 0;

            if (player->shooting)
                source_rectangle.x += 2*PLAYERSIZE;
            else if (time(NULL) % 7 == 0)
                source_rectangle.x += PLAYERSIZE;

            DrawTextureRec(txt_cam->textures.player.idle, source_rectangle, player->position, filter);

        } else {

            if (player->shooting)
                DrawTextureRec(txt_cam->textures.player.running_shoot, source_rectangle, player->position, filter);
            else
                DrawTextureRec(txt_cam->textures.player.running, source_rectangle, player->position, filter);

        }
    }
}




int pause_display(Player *player, DynVector enemies, SmartMap map, ProjVector projs, TexturesCamera *txt_cam) {

    int option = 1; //the "current_screen" value, changed by the player's action
    int resume_hovering, main_menu_hovering; //hovering variables

    //declares a variable to know where the user will click (duh!)
    Vector2 mouse_pointer = {0.0f, 0.0f};

    //build the rectangles representing the buttons logically

    Vector2 extremes = get_map_extremes(map);
    Rectangle resume = {player->position.x + SCREENWIDTH/12, extremes.y*TILESIZE/2.0 - BUTTONHEIGHT/2/20, BUTTONWIDTH/DEFAULTZOOM, BUTTONHEIGHT/DEFAULTZOOM};
    Rectangle main_menu = {player->position.x + SCREENWIDTH/12, extremes.y*TILESIZE/2.0 + BUTTONHEIGHT/2, BUTTONWIDTH/DEFAULTZOOM, BUTTONHEIGHT/DEFAULTZOOM};

    //declaring the variables to store the position of the text drawn in the buttons
    Vector2 resume_text_position = {resume.x + 32.5/DEFAULTZOOM, resume.y + 29/DEFAULTZOOM};
    Vector2 main_menu_text_position = {main_menu.x + 16/DEFAULTZOOM, main_menu.y + 27.5/DEFAULTZOOM};

    //pause menu loop that waits for the player's action
    while (option == 1) {
        mouse_pointer = GetMousePosition();
        //automatic adjust to the mouse pointer, using the camera mode
        mouse_pointer = GetScreenToWorld2D(GetMousePosition(), txt_cam->camera);
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

        BeginMode2D(txt_cam->camera);

        draw_background(map, txt_cam, DARKGRAY);
        draw_map(map, txt_cam, DARKGRAY);
        draw_player(player, txt_cam, DARKGRAY);
        draw_enemies(enemies, txt_cam, DARKGRAY);
        draw_projectiles(projs, txt_cam, DARKGRAY);
        draw_player_hearts_ammo(player, txt_cam, DARKGRAY);
        draw_level_ending_subtitles(player, txt_cam, DARKGRAY);

        //draw resume buttons
        if (resume_hovering == 1)
            DrawTexture(txt_cam->textures.menu.resume_button, resume.x, resume.y, GRAY);
        else
            DrawTexture(txt_cam->textures.menu.resume_button, resume.x, resume.y, WHITE);

        //draw main menu buttons
        if (main_menu_hovering == 1)
            DrawTexture(txt_cam->textures.menu.main_menu, main_menu.x, main_menu.y, GRAY);
        else
            DrawTexture(txt_cam->textures.menu.main_menu, main_menu.x, main_menu.y, WHITE);

        //add text labels for the buttons
        DrawTextEx(GetFontDefault(), "RESUME", resume_text_position, FONTSIZE/DEFAULTZOOM/1.15, 1, BLACK);

        DrawTextEx(GetFontDefault(), "MAIN MENU", main_menu_text_position, FONTSIZE/DEFAULTZOOM/1.15, 1, BLACK);

        EndMode2D();

        EndDrawing();

    }
    return option;
}

void leaderboard_display(TexturesCamera *txt_cam, TopPlayer top_players[]) {
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
            DrawTexture(txt_cam->textures.menu.leaderboard_main_menu, main_menu.x, main_menu.y, GRAY);
        else
            DrawTexture(txt_cam->textures.menu.leaderboard_main_menu, main_menu.x, main_menu.y, WHITE);

        //draws the leaderboard itself
        DrawTexture(txt_cam->textures.menu.leaderboard_table, leaderboard_table.x, leaderboard_table.y, WHITE);

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

int main_menu_display(Player *player,
                      DynVector *enemies,
                      ProjVector *projs,
                      SmartMap map,
                      TexturesCamera *txt_cam,
                      TopPlayer top_players[],
                      GameContext *ctx
                      ) {

    int option = 2; //the "current_screen" value, changed by the player's action
    int play_hovering, leaderboard_hovering, exit_hovering; //hovering variables

    //declares a variable to know where the user will click (duh!)
    Vector2 mouse_pointer = {0.0f, 0.0f};

    //build the rectangles representing the buttons logically
    Rectangle play = {SCREENWIDTH/2 - BUTTONWIDTH/2, SCREENHEIGHT/3, BUTTONWIDTH, BUTTONHEIGHT};
    Rectangle leaderboard = {SCREENWIDTH/2 - BUTTONWIDTH/2, SCREENHEIGHT/3 + SCREENHEIGHT/6, BUTTONWIDTH, BUTTONHEIGHT};
    Rectangle exit = {SCREENWIDTH/2 - BUTTONWIDTH/2, SCREENHEIGHT/3 + SCREENHEIGHT/3, BUTTONWIDTH, BUTTONHEIGHT};

    //main menu loop that closes when the player makes an action
    while (option == 2 && ctx->running) {
        mouse_pointer = GetMousePosition();
        play_hovering = 0;
        leaderboard_hovering = 0;
        exit_hovering = 0;

        //hovering and click tests
        if (CheckCollisionPointRec(mouse_pointer, play)) {
            play_hovering = 1;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {

                *player = init_player(map);
                *projs = init_proj_vector();
                init_enemies(enemies, map);

                option = 0; //starts the game
            }
        }

        if (CheckCollisionPointRec(mouse_pointer, leaderboard)) {
            leaderboard_hovering = 1;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                leaderboard_display(txt_cam, top_players); //shows the leaderboard
        }

        if (CheckCollisionPointRec(mouse_pointer, exit)) {
            exit_hovering = 1;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                ctx->running = 0; //makes the game quit
                top_players_to_bin(top_players);
            }

        }

        //graphical part of the main menu
        BeginDrawing();

        ClearBackground(DARKGRAY);

        DrawTexture(txt_cam->textures.menu.infman_logo, SCREENWIDTH/2 - 495.0/2, 97/2, WHITE);

        //draw play buttons
        if (play_hovering == 1)
            DrawTexture(txt_cam->textures.menu.play_button, play.x, play.y, GRAY);
        else
            DrawTexture(txt_cam->textures.menu.play_button, play.x, play.y, WHITE);

        //draw leaderboard buttons
        if (leaderboard_hovering == 1)
            DrawTexture(txt_cam->textures.menu.leaderboard_button, leaderboard.x, leaderboard.y, GRAY);
        else
            DrawTexture(txt_cam->textures.menu.leaderboard_button, leaderboard.x, leaderboard.y, WHITE);

        //draw exit buttons
        if (exit_hovering == 1)
            DrawTexture(txt_cam->textures.menu.exit_button, exit.x, exit.y, GRAY);
        else
            DrawTexture(txt_cam->textures.menu.exit_button, exit.x, exit.y, WHITE);

        //add text labels for the buttons
        DrawText("PLAY", play.x + 47.5, play.y + 27.5, FONTSIZE, BLACK);
        DrawText("LEADER", leaderboard.x + 35, leaderboard.y + 20.5, FONTSIZE, BLACK);
        DrawText("BOARD", leaderboard.x + 35 + FONTSIZE/4, leaderboard.y + 20.5 + FONTSIZE, FONTSIZE, BLACK);
        DrawText("EXIT", exit.x + 50, exit.y + 25, FONTSIZE, BLACK);

        EndDrawing();
    }
    return option;
}

