#include "player.h"
#include "textures_and_camera.h"

void is_player_blocked(Player *player) {

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

void is_player_on_map(Player *player) {

    int horizontal_tile, vertical_tile;

    get_tile_on_matrix(&horizontal_tile, &vertical_tile, (Rectangle){
        player->position.x,
        player->position.y,
        player->size.x,
        player->size.y
    });

    if (vertical_tile == MAPHEIGHT + 6)
        player->hearts = 0;

    if (horizontal_tile <= 0)
        player->blocked_left = 1;

    if (horizontal_tile >= MAPLENGTH - 1)
        player->blocked_right = 1;

}

void player_movement(Player *player, float invincibility_timer) {
    player->position.x += player->speed.x;
    player->position.y += player->speed.y;

    //horizontal movement
    if (IsKeyDown(KEY_D) && !IsKeyDown(KEY_A) && !player->blocked_right)
        player->speed.x = WALKSPEED;
    else if (IsKeyDown(KEY_A) && !IsKeyDown(KEY_D) && !player->blocked_left)
        player->speed.x = -WALKSPEED;
    else
        player->speed.x = 0.0f;

    if (invincibility_timer > 0 && invincibility_timer < INVINCIBILITYTIME/2)
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

void vulnerability_update(Player *player, float invincibility_timer) {
    time_actions_update_bool(&player->vulnerable, &invincibility_timer, INVINCIBILITYTIME);
}



