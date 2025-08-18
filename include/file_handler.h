#pragma once
#include "top_scores.h"

#define FROM_ROOT(relative_path) "../" relative_path // Here its possible to change the root dir
#define FROM_CONFIG(file) FROM_ROOT("config/" file)
#define FROM_RESOURCES(category_slash_file) FROM_ROOT("resources/" category_slash_file)

//binaries related functions
TopPlayer *top_players_from_bin();
void top_players_to_bin(TopPlayer *top_players);
