#include "../include/assets_loader.h"

#include <stdio.h>

static bool load_texture_asset(Texture2D *out, const char *filename, bool mobile_build)
{
    char path[160];
    if (mobile_build) {
        *out = LoadTexture(filename);
        if (out->width > 0) return true;
        int nm = snprintf(path, sizeof path, "assets/%s", filename);
        if (nm > 0 && nm < (int)sizeof path) {
            *out = LoadTexture(path);
            if (out->width > 0) return true;
        }
    }

    int n = snprintf(path, sizeof path, "assets/%s", filename);
    if (n > 0 && n < (int)sizeof path && FileExists(path)) {
        *out = LoadTexture(path);
        if (out->width > 0) return true;
    }
    if (FileExists(filename)) {
        *out = LoadTexture(filename);
        if (out->width > 0) return true;
    }
    return false;
}

static bool load_first_from_list(Texture2D *out, const char *const *names, int count, bool mobile_build)
{
    for (int i = 0; i < count; i++) {
        if (load_texture_asset(out, names[i], mobile_build)) return true;
    }
    return false;
}

bool load_first_hero(Texture2D *out, bool mobile_build)
{
    static const char *k_names[] = {
        "hero-idle-strip.png",
        "sprite-hero.png",
        "medieval-rpg-main-character-d002.png",
        "player_spritesheet.png",
    };
    return load_first_from_list(out, k_names, (int)(sizeof(k_names) / sizeof(k_names[0])), mobile_build);
}

bool load_first_attack(Texture2D *out, bool mobile_build)
{
    static const char *k_names[] = {
        "hero-attack-strip.png",
        "sprite-hero-attack.png",
    };
    return load_first_from_list(out, k_names, (int)(sizeof(k_names) / sizeof(k_names[0])), mobile_build);
}

bool load_first_jump(Texture2D *out, bool mobile_build)
{
    static const char *k_names[] = {
        "hero-jump-strip.png",
        "sprite-hero-jump.png",
        "jump-hero-strip.png",
    };
    return load_first_from_list(out, k_names, (int)(sizeof(k_names) / sizeof(k_names[0])), mobile_build);
}

bool load_first_tile(Texture2D *out, bool mobile_build)
{
    static const char *k_names[] = {
        "tile-stone-ground.png",
        "tile-ground.png",
        "tile.png",
    };
    return load_first_from_list(out, k_names, (int)(sizeof(k_names) / sizeof(k_names[0])), mobile_build);
}

bool load_first_enemy(Texture2D *out, bool mobile_build)
{
    static const char *k_names[] = {
        "enemy-echo-basic.png",
        "sprite-simple-enemy.png",
    };
    return load_first_from_list(out, k_names, (int)(sizeof(k_names) / sizeof(k_names[0])), mobile_build);
}

bool load_first_map(Texture2D *out, bool mobile_build)
{
    static const char *k_names[] = {
        "ui-choir-map.png",
        "sprite-quest-map.png",
    };
    return load_first_from_list(out, k_names, (int)(sizeof(k_names) / sizeof(k_names[0])), mobile_build);
}

void compute_horizontal_strip(Texture2D t, int preferred_cell_w, int *out_fw, int *out_fh, int *out_nf)
{
    int W = t.width;
    int H = t.height;
    *out_fh = H;

    if (preferred_cell_w <= 0) preferred_cell_w = 48;

    if (W <= 0) {
        *out_fw = preferred_cell_w;
        *out_nf = 1;
        return;
    }

    if (W == 64 && H == 32) {
        *out_nf = 1;
        *out_fw = 64;
        *out_fh = 32;
        return;
    }

    if (W >= preferred_cell_w && W % preferred_cell_w == 0) {
        *out_nf = W / preferred_cell_w;
        *out_fw = preferred_cell_w;
        if (*out_nf > 32) *out_nf = 32;
        return;
    }

    if (W % 32 == 0 && W >= 64) {
        *out_nf = W / 32;
        *out_fw = 32;
        if (*out_nf > 32) *out_nf = 32;
        return;
    }

    if (W % 2 == 0 && W >= 48) {
        int hw = W / 2;
        if (hw >= 12 && hw <= 80) {
            *out_nf = 2;
            *out_fw = hw;
            return;
        }
    }

    *out_nf = 1;
    *out_fw = W;
}
