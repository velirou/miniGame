#ifndef ASSETS_LOADER_H
#define ASSETS_LOADER_H

#include "raylib.h"
#include <stdbool.h>

bool load_first_hero(Texture2D *out, bool mobile_build);
bool load_first_attack(Texture2D *out, bool mobile_build);
bool load_first_jump(Texture2D *out, bool mobile_build);
bool load_first_tile(Texture2D *out, bool mobile_build);
bool load_first_enemy(Texture2D *out, bool mobile_build);
bool load_first_map(Texture2D *out, bool mobile_build);

void compute_horizontal_strip(Texture2D t, int preferred_cell_w, int *out_fw, int *out_fh, int *out_nf);

#endif
