#include "player.h"
#include "map.h"
#include "textures_and_camera.h"
#include "projectiles.h"
#include "game.h"
#include <math.h>

Vector2 find_player_spawn(SmartMap map) {

    char *charptr;

    for(int i = 0; i < vector_len(map); i++) {
        for (int j = 0; j < matrix_row_len(map, i); j++) {
            charptr = matrix_get(map, i, j);

            if (*charptr == 'P') {
                return (Vector2){j*TILESIZE, i*TILESIZE};
            }
        }
    }

    return (Vector2) {0.0, 0.0}; // In case of player not found

}

Player init_player(SmartMap map) {

    Player player;

    //player attributes
    player.hearts = PLAYERHEARTS;
    player.size = (Vector2){PLAYERSIZE, PLAYERSIZE};
    player.speed = (Vector2){0, 0};
    player.position = find_player_spawn(map); //initializes the player position and the map
    player.sprite_counter = 0;
    player.ammo_laser = PLAYERAMMO1;
    player.ammo_bazooka = PLAYERAMMO2;
    player.score = 0;
    player.shooting = 0;
    player.blocked_left = 0;
    player.blocked_right = 0;
    player.on_floor = 0;
    player.on_ceiling = 0;
    player.vulnerable = 0;

    player.timer.shoot = 0;
    player.timer.invincibility = 0;
    player.timer.sprite = 0;

    return player;
}

void is_player_blocked(Player *player, SmartMap map) {

    int on_floor_test = 0;
    int blocked_left_test = 0;
    int blocked_left_temp = 0;
    int blocked_right_test = 0;
    int blocked_right_temp = 0;

    Rectangle player_box = {
        player->position.x - 1,
        player->position.y,
        player->size.x + 2,
        player->size.y + 2
    };

    Vector2 extremes = get_map_extremes(map);
    int extreme_y = extremes.y;
    int extreme_x = extremes.x;

    Rectangle map_tiles_data[extreme_y][extreme_x];

    Rectangle *map_tiles[extreme_y];

    for (int i = 0; i < extreme_y; i++) {
        map_tiles[i] = map_tiles_data[i];
    }

    get_map_tiles_matrix(map_tiles, map, 0);

    for (int i = 0; i < vector_len(map); i++) {
        for (int j = 0; j < matrix_row_len(map, i); j++) {
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
        player->on_floor = 1;
    else
        player->on_floor = 0;

    if (blocked_right_test > 0)
        player->blocked_right = 1;
    else
        player->blocked_right = 0;

    if (blocked_left_test > 0)
        player->blocked_left = 1;
    else
        player->blocked_left = 0;
}

void is_player_on_map(Player *player, SmartMap map) {

    int horizontal_tile, vertical_tile;

    get_tile_on_matrix(&horizontal_tile, &vertical_tile, (Rectangle){
        player->position.x,
        player->position.y,
        player->size.x,
        player->size.y
    });

    Vector2 extremes = get_map_extremes(map);

    if (vertical_tile == extremes.y + 6)
        player->hearts = 0;

    if (horizontal_tile <= 0)
        player->blocked_left = 1;

    if (horizontal_tile >= extremes.x - 1)
        player->blocked_right = 1;

}

void player_movement(Player *player) {
    player->position.x += player->speed.x;
    player->position.y += player->speed.y;

    //horizontal movement
    if (IsKeyDown(KEY_D) && !IsKeyDown(KEY_A) && !player->blocked_right)
        player->speed.x = WALKSPEED;
    else if (IsKeyDown(KEY_A) && !IsKeyDown(KEY_D) && !player->blocked_left)
        player->speed.x = -WALKSPEED;
    else
        player->speed.x = 0.0f;

    if (player->timer.invincibility > 0 && player->timer.invincibility < INVINCIBILITYTIME/2)
        player->speed.x = player->speed.x/DAMAGESPEEDRETARDING;

    //vertical movement
    if (player->on_floor) {
        player->speed.y = 0.0f;

        if (IsKeyDown(KEY_W))
            player_jump(player, 1);

    } else if (player->speed.y < MAXFALLINGSPEED)
        player->speed.y += GRAVITY;
}

void player_jump(Player *player, float scale) {
    player->speed.y = -JUMPSPEED*scale;
}

void vulnerability_update(Player *player) {
    time_actions_update_bool(&player->vulnerable, &player->timer.invincibility, INVINCIBILITYTIME);
}

void player_damage(Player *player, char source) {
    switch (source) {
        case 'S':
            player->hearts--;
            break;

        case 'L':
            player->hearts--;
            player->speed.x = player->speed.x/10;
            break;
    }

    player->vulnerable = 0;
}

void spike_damage(Player *player, SmartMap map) {
    if (player->vulnerable) {
        int horizontal_tile = floorf((player->position.x)/TILESIZE);
        int horizontal_tile2 = floorf((player->position.x + PLAYERSIZE)/TILESIZE);
        int vertical_tile = floorf((player->position.y + PLAYERSIZE - 8)/TILESIZE);

        char tests[4];
        char *charptr;

        charptr = matrix_get(map, vertical_tile, horizontal_tile);
        tests[0] = charptr ? *charptr : '{';

        charptr = matrix_get(map, vertical_tile, horizontal_tile2);
        tests[1] = charptr ? *charptr : '{';

        charptr = matrix_get(map, vertical_tile - 1, horizontal_tile);
        tests[2] = charptr ? *charptr : '{';

        charptr = matrix_get(map, vertical_tile - 1, horizontal_tile2);
        tests[3] = charptr ? *charptr : '{';

        bool should_take_damage;
        for (int i = 0; i < 4; i++) {
            if (tests[i] == 'S') {
                should_take_damage = true;
            }
        }

        if (should_take_damage) {
            player_jump(player, 1.2);
            player_damage(player, 'S');
        }
    }
}

bool projectile_player_hit_test(Player *player, Projectile proj) {
    Rectangle player_collision_box = {
        player->position.x,
        player->position.y,
        player->size.x,
        player->size.x,
    };

    bool collision_test;

    if (player->vulnerable)
        collision_test = CheckCollisionRecs(player_collision_box, proj.bullet);
    else
        collision_test = 0;

    return collision_test;
}

void player_death(Player *player, DynVector enemies, TexturesCamera *txt_cam, SmartMap map, GameContext *ctx, ProjVector projs) {
    int accept_flag = 0;
    int hovering;
    char player_score_string[MAXNAME];
    int map_h = get_map_extremes(map).y;

    sprintf(player_score_string, "%d", player->score);

    Vector2 mouse_pointer = {0.0f, 0.0f};

    EndDrawing();


    Rectangle main_menu = {player->position.x + SCREENWIDTH/12,
        map_h*TILESIZE/2.0 + BUTTONHEIGHT/2,
        BUTTONWIDTH/DEFAULTZOOM,
        BUTTONHEIGHT/DEFAULTZOOM
    };

    while(accept_flag == 0) {
        mouse_pointer = GetMousePosition();
        mouse_pointer = GetScreenToWorld2D(GetMousePosition(), txt_cam->camera);
        hovering = 0;

        if (CheckCollisionPointRec(mouse_pointer, main_menu)) {
            hovering = 1;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                ctx->curr_screen = 2;
                accept_flag = 1;
            }
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode2D(txt_cam->camera);

        draw_background(map, txt_cam, DARKGRAY);
        draw_map(map, txt_cam, DARKGRAY);
        draw_player(player, txt_cam, DARKGRAY);
        draw_enemies(enemies, txt_cam, DARKGRAY);
        draw_projectiles(projs, txt_cam, DARKGRAY);

        if (hovering)
            DrawTexture(txt_cam->textures.menu.main_menu, main_menu.x, main_menu.y, GRAY);
        else
            DrawTexture(txt_cam->textures.menu.main_menu, main_menu.x, main_menu.y, WHITE);

        DrawTextEx(GetFontDefault(), "MAIN MENU", (Vector2){main_menu.x + 16/DEFAULTZOOM, main_menu.y + 27.5/DEFAULTZOOM}, FONTSIZE/DEFAULTZOOM/1.15, 1, BLACK);

        EndMode2D();

        DrawTextEx(GetFontDefault(), "YOU DIED", (Vector2) {12*SCREENWIDTH/33, SCREENHEIGHT/10}, 3*FONTSIZE, FONTSIZE/4, RED);

        DrawTextEx(GetFontDefault(), "SCORE: ", (Vector2){3*SCREENWIDTH/8, SCREENHEIGHT/3}, 3*FONTSIZE, FONTSIZE/4, INFMANBLUE1);

        DrawTextEx(GetFontDefault(), player_score_string, (Vector2){3*SCREENWIDTH/8 + 9*FONTSIZE + 9*FONTSIZE/4, SCREENHEIGHT/3}, 3*FONTSIZE, FONTSIZE/4, INFMANBLUE2);

        EndDrawing();
    }
}

void player_death_test(Player *player, DynVector enemies, TexturesCamera *txt_cam, SmartMap map, GameContext *ctx, ProjVector projs) {
    if (player->hearts <= 0 && !ctx->curr_screen)
        player_death(player, enemies, txt_cam, map, ctx, projs);
}

bool is_player_on_ending_platform(Player *player, SmartMap map) {

    Rectangle player_rect = {
        player->position.x,
        player->position.y,
        player->size.x,
        player->size.y
    };

    Rectangle ending_platform_rect = {0, 0, 0, 0};

    for (int i = 0; i < vector_len(map); i++) {
        for (int j = 0; j < matrix_row_len(map, i); j++) {
            char *charptr = matrix_get(map, i, j);

            if (*charptr == 'L') {
                ending_platform_rect = (Rectangle) {
                    TILESIZE * (j - 1),
                    TILESIZE * (i - 1),
                    TILESIZE * 3,
                    TILESIZE * 2
                };
            }
        }
    }

    return CheckCollisionRecs(ending_platform_rect, player_rect);
}

void player_laser(Player *player, ProjVector *projs) {

    if (player->ammo_laser > 0 && IsKeyPressed(KEY_SPACE)) {
        int current_direction;
        Rectangle current_bullet;

        if (projs->counter >= projs->max)
            projs->counter = 0;

        if (player->speed.x == 0)
            current_direction = 1;
        else
            current_direction = player->speed.x/fabsf(player->speed.x);

        if (current_direction == 1) {
            current_bullet.x = player->position.x + PLAYERSIZE;
        } else
            current_bullet.x = player->position.x;

        current_bullet.y = player->position.y + 9;
        current_bullet.width = PROJECTILESIZE;
        current_bullet.height = PROJECTILESIZE/2.0;

        projs->arr[projs->counter] = (Projectile){current_bullet, (Vector2){current_direction, 0.0}, 0};

        projs->counter++;
        player->ammo_laser--;
        player->shooting = 1;
    }

    if (player->timer.shoot >= PLAYERSHOOTTIME) {
        player->shooting = 0;
        player->timer.shoot = 0;
    } else if (player->shooting) {
        player->timer.shoot += GetFrameTime();
    } else
        player->timer.shoot = 0.0f;

}

void player_bazooka(Player *player, ProjVector *projs) {

    if (player->ammo_bazooka > 0 && IsKeyPressed(KEY_B)) {
        int current_direction;
        Rectangle current_bullet;

        if (projs->counter >= projs->max)
            projs->counter = 0;

        if (player->speed.x == 0)
            current_direction = 1;
        else
            current_direction = player->speed.x/fabsf(player->speed.x);

        if (current_direction == 1) {
            current_bullet.x = player->position.x + PLAYERSIZE;
        } else
            current_bullet.x = player->position.x;

        current_bullet.y = player->position.y + 9;
        current_bullet.width = PROJECTILESIZE*2;
        current_bullet.height = PROJECTILESIZE;

        projs->arr[projs->counter] = (Projectile){current_bullet, (Vector2){current_direction, player->speed.y/10.0}, 1};

        projs->counter++;
        player->ammo_bazooka--;
        player->shooting = 1;
    }

}

