/*
 * Phase 1 prototype — The Sunken Choir (movement, collision, attack, sprint, dummy enemy).
 * Tunables: section "Gameplay tuning" below.
 * Manual test: A/D  Space  Shift sprint  J attack  R reset  F1 hitboxes.
 *
 * Art (PNG): tries assets/<file> first, then cwd. Canonical names — see ASSET_* below.
 */

#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

/* --- Gameplay tuning (Phase 1) --- */
#define MAP_WIDTH  56
#define MAP_HEIGHT 40
#define TILE_SIZE  48

#define GRAVITY           2200.0f
#define MOVE_SPEED        260.0f
#define RUN_SPEED         400.0f
#define JUMP_VELOCITY    -520.0f
#define JUMP_CUT_MULT      0.45f
#define COYOTE_TIME        0.12f
#define JUMP_BUFFER_TIME   0.10f
#define MAX_FALL_SPEED    900.0f

#define ATTACK_DURATION    0.28f
#define ATTACK_ACTIVE_START 0.05f
#define ATTACK_ACTIVE_END   0.18f
#define ATTACK_COOLDOWN    0.10f
#define ATTACK_RANGE        56.0f
#define ATTACK_HEIGHT       40.0f

#define HITSTOP_DURATION   0.09f
#define ENEMY_MAX_HP           4
#define ENEMY_HURT_IFRAMES     0.35f
#define ENEMY_KNOCKBACK       180.0f

#define PLAYER_DRAW_W       48.0f
#define PLAYER_DRAW_H       64.0f
#define ENEMY_DRAW_W        48.0f
#define ENEMY_DRAW_H        64.0f
#define HITBOX_PAD            8.0f
/* Sprite drawn this many px up so feet line up with collider feet (hitbox inset was sinking art) */
#define PLAYER_SPRITE_Y_OFF (-HITBOX_PAD)

/* --- Canonical asset filenames (PNG). Order = load priority within category. --- */
#define ASSET_HERO_A     "sprite-hero.png"
#define ASSET_HERO_B     "medieval-rpg-main-character-d002.png"
#define ASSET_HERO_C     "player_spritesheet.png"
#define ASSET_BG_A       "sprite-background.png"
#define ASSET_BG_B       "background.png"
#define ASSET_BG_C       "sprite-castle.png"
#define ASSET_TILE_A     "tile-ground.png"
#define ASSET_TILE_B     "tile.png"
#define ASSET_ENEMY      "sprite-simple-enemy.png"

/* World / pit: respawn if you fall past playable space (open bottom in pit) */
static Vector2 g_spawn = { 0 };
static float   g_respawn_msg_timer = 0.0f;

/* Try assets/<name> then ./<name>. Returns true if texture loaded (width > 0). */
static bool load_texture_asset(Texture2D *out, const char *filename)
{
    char path[160];
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

static bool load_first_hero(Texture2D *out)
{
    if (load_texture_asset(out, ASSET_HERO_A)) return true;
    if (load_texture_asset(out, ASSET_HERO_B)) return true;
    if (load_texture_asset(out, ASSET_HERO_C)) return true;
    return false;
}

static bool load_first_bg(Texture2D *out)
{
    if (load_texture_asset(out, ASSET_BG_A)) return true;
    if (load_texture_asset(out, ASSET_BG_B)) return true;
    if (load_texture_asset(out, ASSET_BG_C)) return true;
    return false;
}

static bool load_first_tile(Texture2D *out)
{
    if (load_texture_asset(out, ASSET_TILE_A)) return true;
    if (load_texture_asset(out, ASSET_TILE_B)) return true;
    return false;
}

typedef enum PlayerState {
    PLAYER_NORMAL = 0,
    PLAYER_ATTACKING
} PlayerState;

typedef struct Player {
    Vector2 position;
    Vector2 velocity;
    bool on_ground;
    float coyote_timer;
    float jump_buffer_timer;
    int facing;
    PlayerState state;
    float state_timer;
    float iframes_timer;
    float attack_cd_timer;
    int anim_frame;
    float anim_time;
} Player;

typedef struct DummyEnemy {
    Rectangle bounds;
    int hp;
    float hurt_iframes;
    float hurt_flash;
    bool dead;
} DummyEnemy;

static int level_map[MAP_HEIGHT][MAP_WIDTH];

/*
 * Horizontal sprite strip: detect frame count + frame width.
 * If width is not a multiple of PLAYER_DRAW_W (48), integer division can yield 1 frame
 * and the full texture is drawn — e.g. 64px sheet with two 32px poses looks like "two chars".
 */
static void compute_horizontal_strip(Texture2D t, int *out_fw, int *out_fh, int *out_nf)
{
    int W = t.width;
    int H = t.height;
    *out_fh = H;
    const int cell = (int)PLAYER_DRAW_W;

    if (W <= 0) {
        *out_fw = cell;
        *out_nf = 1;
        return;
    }

    /*
     * ASSET_HERO_A / ASSET_HERO_B: one wide pose 64×32,
     * NOT two 32×32 cels. Old logic split into L/R halves → half body + flashing.
     */
    if (W == 64 && H == 32) {
        *out_nf = 1;
        *out_fw = 64;
        *out_fh = 32;
        return;
    }

    /* 1) N frames of 48px (matches gameplay width) */
    if (W >= cell && W % cell == 0) {
        *out_nf = W / cell;
        *out_fw = cell;
        if (*out_nf > 32) *out_nf = 32;
        return;
    }

    /* 2) N frames of 32px (common pixel art; e.g. 64 = 2 frames, 128 = 4) */
    if (W % 32 == 0 && W >= 64) {
        *out_nf = W / 32;
        *out_fw = 32;
        if (*out_nf > 32) *out_nf = 32;
        return;
    }

    /* 3) Two equal halves (typical 2-frame walk) — only if splitting won’t treat tiny sheets wrong */
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

static void draw_tile_texture(Texture2D t, int px0, int py0)
{
    Rectangle src = { 0.0f, 0.0f, (float)t.width, (float)t.height };
    Rectangle dest = { (float)px0, (float)py0, (float)TILE_SIZE, (float)TILE_SIZE };
    DrawTexturePro(t, src, dest, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
}

static void fill_rect(int x0, int y0, int x1, int y1, int v)
{
    if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
    if (y0 > y1) { int t = y0; y0 = y1; y1 = t; }
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT)
                level_map[y][x] = v;
        }
    }
}

/*
 * Tall arena: left/right decks, central pit with open bottom (kill plane).
 * Climb uses ~48px (1 tile) vertical steps — within one jump arc (~61px max).
 * Zig-zag offsets force jumps; you cannot walk the full height without jumping.
 */
static void build_test_arena(void)
{
    for (int y = 0; y < MAP_HEIGHT; y++)
        for (int x = 0; x < MAP_WIDTH; x++)
            level_map[y][x] = 0;

    /* Outer shell */
    fill_rect(0, 0, MAP_WIDTH - 1, 0, 1);
    fill_rect(0, MAP_HEIGHT - 1, MAP_WIDTH - 1, MAP_HEIGHT - 1, 1);
    fill_rect(0, 0, 0, MAP_HEIGHT - 1, 1);
    fill_rect(MAP_WIDTH - 1, 0, MAP_WIDTH - 1, MAP_HEIGHT - 1, 1);

    /* Bottom lip at sides only — pit (x 18–35) has no bottom tiles → void */
    for (int x = 1; x < MAP_WIDTH - 1; x++) {
        if (x < 18 || x > 35)
            level_map[MAP_HEIGHT - 1][x] = 1;
    }
    fill_rect(1, MAP_HEIGHT - 2, 17, MAP_HEIGHT - 2, 1);
    fill_rect(38, MAP_HEIGHT - 2, MAP_WIDTH - 2, MAP_HEIGHT - 2, 1);

    const int fg = MAP_HEIGHT - 8;

    /* Side decks (gap over pit) */
    fill_rect(2, fg, 17, fg, 1);
    fill_rect(38, fg, MAP_WIDTH - 3, fg, 1);

    /* Pit sidewalls */
    fill_rect(17, fg + 1, 17, MAP_HEIGHT - 3, 1);
    fill_rect(36, fg + 1, 36, MAP_HEIGHT - 3, 1);

    /* Stair platforms: each step is 1 tile higher than the last (requires jump) */
    fill_rect(19, fg - 1, 22, fg - 1, 1);
    fill_rect(23, fg - 2, 26, fg - 2, 1);
    fill_rect(25, fg - 3, 28, fg - 3, 1);
    fill_rect(24, fg - 4, 27, fg - 4, 1);
    fill_rect(23, fg - 5, 26, fg - 5, 1);
    fill_rect(22, fg - 6, 25, fg - 6, 1);
    fill_rect(23, fg - 7, 28, fg - 7, 1);
    fill_rect(26, fg - 8, 31, fg - 8, 1);
    fill_rect(27, fg - 9, 32, fg - 9, 1);
    fill_rect(26, fg - 10, 30, fg - 10, 1);

    /* Boss perch (dummy) — top of the climb */
    fill_rect(25, fg - 11, 32, fg - 11, 1);

    /* Roof accents */
    fill_rect(10, 4, 16, 4, 1);
    fill_rect(40, 4, 46, 4, 1);

    g_spawn = (Vector2){ 4.0f * (float)TILE_SIZE,
                         (float)(fg * TILE_SIZE) - PLAYER_DRAW_H - 0.5f };
}

static bool tile_solid(int tx, int ty)
{
    if (tx < 0 || ty < 0 || tx >= MAP_WIDTH || ty >= MAP_HEIGHT) return true;
    return level_map[ty][tx] == 1;
}

static Rectangle player_collider(Vector2 pos)
{
    float w = PLAYER_DRAW_W - HITBOX_PAD * 2.0f;
    float h = PLAYER_DRAW_H - HITBOX_PAD * 2.0f;
    return (Rectangle){ pos.x + HITBOX_PAD, pos.y + HITBOX_PAD, w, h };
}

static void resolve_axis_x(Rectangle *body, float prev_x, float *vel_x)
{
    for (int ty = 0; ty < MAP_HEIGHT; ty++) {
        for (int tx = 0; tx < MAP_WIDTH; tx++) {
            if (!tile_solid(tx, ty)) continue;
            Rectangle tile = { (float)(tx * TILE_SIZE), (float)(ty * TILE_SIZE),
                               (float)TILE_SIZE, (float)TILE_SIZE };
            if (!CheckCollisionRecs(*body, tile)) continue;

            float overlap_left = (body->x + body->width) - tile.x;
            float overlap_right = (tile.x + tile.width) - body->x;
            if (prev_x + body->width <= tile.x + 0.01f) {
                body->x = tile.x - body->width;
                if (*vel_x > 0) *vel_x = 0;
            } else if (prev_x >= tile.x + tile.width - 0.01f) {
                body->x = tile.x + tile.width;
                if (*vel_x < 0) *vel_x = 0;
            } else {
                if (overlap_left < overlap_right) {
                    body->x = tile.x - body->width;
                    if (*vel_x > 0) *vel_x = 0;
                } else {
                    body->x = tile.x + tile.width;
                    if (*vel_x < 0) *vel_x = 0;
                }
            }
        }
    }
}

static void resolve_axis_y(Rectangle *body, float prev_y, float *vel_y, bool *on_ground)
{
    for (int ty = 0; ty < MAP_HEIGHT; ty++) {
        for (int tx = 0; tx < MAP_WIDTH; tx++) {
            if (!tile_solid(tx, ty)) continue;
            Rectangle tile = { (float)(tx * TILE_SIZE), (float)(ty * TILE_SIZE),
                               (float)TILE_SIZE, (float)TILE_SIZE };
            if (!CheckCollisionRecs(*body, tile)) continue;

            if (prev_y + body->height <= tile.y + 0.5f && *vel_y >= 0) {
                body->y = tile.y - body->height;
                *vel_y = 0;
                *on_ground = true;
            } else if (prev_y >= tile.y + tile.height - 0.5f && *vel_y < 0) {
                body->y = tile.y + tile.height;
                if (*vel_y < 0) *vel_y = 0;
            }
        }
    }
}

static bool shift_held(void)
{
    return IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
}

static void physics_player(Player *p, float dt)
{
    if (p->iframes_timer > 0.0f) p->iframes_timer -= dt;
    if (p->attack_cd_timer > 0.0f) p->attack_cd_timer -= dt;

    float move_x = 0.0f;
    if (IsKeyDown(KEY_A)) move_x -= 1.0f;
    if (IsKeyDown(KEY_D)) move_x += 1.0f;
    if (move_x > 0.1f) p->facing = 1;
    else if (move_x < -0.1f) p->facing = -1;

    float move_speed = shift_held() ? RUN_SPEED : MOVE_SPEED;

    if (p->state == PLAYER_ATTACKING) {
        p->state_timer -= dt;
        /* Full horizontal control while swinging (slowing to ~0.35 felt like movement “stopping”) */
        p->velocity.x = move_x * move_speed;
        p->velocity.y += GRAVITY * dt;
        if (p->state_timer <= 0.0f) p->state = PLAYER_NORMAL;
    } else {
        p->velocity.x = move_x * move_speed;
        p->velocity.y += GRAVITY * dt;
        if (p->velocity.y > MAX_FALL_SPEED) p->velocity.y = MAX_FALL_SPEED;

        if (p->on_ground) p->coyote_timer = COYOTE_TIME;
        else if (p->coyote_timer > 0.0f) p->coyote_timer -= dt;

        if (IsKeyPressed(KEY_SPACE)) p->jump_buffer_timer = JUMP_BUFFER_TIME;
        if (p->jump_buffer_timer > 0.0f) p->jump_buffer_timer -= dt;

        bool can_jump = (p->on_ground || p->coyote_timer > 0.0f);
        if (can_jump && p->jump_buffer_timer > 0.0f) {
            p->velocity.y = JUMP_VELOCITY;
            p->on_ground = false;
            p->coyote_timer = 0.0f;
            p->jump_buffer_timer = 0.0f;
        }

        if (IsKeyReleased(KEY_SPACE) && p->velocity.y < 0.0f)
            p->velocity.y *= JUMP_CUT_MULT;

        if (IsKeyPressed(KEY_J) && p->attack_cd_timer <= 0.0f && p->state == PLAYER_NORMAL) {
            p->state = PLAYER_ATTACKING;
            p->state_timer = ATTACK_DURATION;
            p->attack_cd_timer = ATTACK_DURATION + ATTACK_COOLDOWN;
        }
    }

    p->on_ground = false;

    float prev_body_x = player_collider(p->position).x;

    p->position.x += p->velocity.x * dt;
    Rectangle body = player_collider(p->position);
    resolve_axis_x(&body, prev_body_x, &p->velocity.x);
    p->position.x = body.x - HITBOX_PAD;

    float prev_body_y = player_collider(p->position).y;

    p->position.y += p->velocity.y * dt;
    body = player_collider(p->position);
    resolve_axis_y(&body, prev_body_y, &p->velocity.y, &p->on_ground);
    p->position.y = body.y - HITBOX_PAD;
}

static void try_respawn_fall(Player *p)
{
    float feet = p->position.y + PLAYER_DRAW_H;
    float kill_line = (float)(MAP_HEIGHT * TILE_SIZE) + 4.0f;
    if (feet <= kill_line) return;

    p->position = g_spawn;
    p->velocity = (Vector2){ 0.0f, 0.0f };
    p->on_ground = false;
    p->coyote_timer = 0.0f;
    p->jump_buffer_timer = 0.0f;
    p->state = PLAYER_NORMAL;
    p->state_timer = 0.0f;
    p->attack_cd_timer = 0.0f;
    p->iframes_timer = 0.45f;
    g_respawn_msg_timer = 2.0f;
}

/* Full run reset (manual key) — spawn, enemy, timers */
static void reset_run(Player *p, DummyEnemy *e, Rectangle enemy_spawn, float *hitstop)
{
    p->position = g_spawn;
    p->velocity = (Vector2){ 0.0f, 0.0f };
    p->on_ground = false;
    p->coyote_timer = 0.0f;
    p->jump_buffer_timer = 0.0f;
    p->state = PLAYER_NORMAL;
    p->state_timer = 0.0f;
    p->attack_cd_timer = 0.0f;
    p->iframes_timer = 0.0f;
    p->anim_frame = 0;
    p->anim_time = 0.0f;
    e->bounds = enemy_spawn;
    e->hp = ENEMY_MAX_HP;
    e->hurt_iframes = 0.0f;
    e->hurt_flash = 0.0f;
    e->dead = false;
    *hitstop = 0.0f;
    g_respawn_msg_timer = 0.0f;
}

static Rectangle attack_hitbox(const Player *p)
{
    float x = p->position.x + (p->facing > 0 ? PLAYER_DRAW_W : -ATTACK_RANGE);
    float y = p->position.y + PLAYER_DRAW_H * 0.35f;
    return (Rectangle){ x, y, ATTACK_RANGE, ATTACK_HEIGHT };
}

static bool attack_is_active(const Player *p)
{
    if (p->state != PLAYER_ATTACKING) return false;
    float t = ATTACK_DURATION - p->state_timer;
    return t >= ATTACK_ACTIVE_START && t <= ATTACK_ACTIVE_END;
}

static void update_enemy(DummyEnemy *e, const Rectangle *attack, bool attack_active, float *hitstop,
                         Vector2 player_pos, float dt)
{
    if (e->dead) return;
    if (e->hurt_iframes > 0.0f) e->hurt_iframes -= dt;
    if (e->hurt_flash > 0.0f) e->hurt_flash -= dt;

    if (attack_active && e->hurt_iframes <= 0.0f && CheckCollisionRecs(*attack, e->bounds)) {
        e->hp--;
        e->hurt_iframes = ENEMY_HURT_IFRAMES;
        e->hurt_flash = 0.12f;
        *hitstop = HITSTOP_DURATION;
        float cx = e->bounds.x + e->bounds.width * 0.5f;
        float px = player_pos.x + PLAYER_DRAW_W * 0.5f;
        float knock = (cx >= px) ? ENEMY_KNOCKBACK : -ENEMY_KNOCKBACK;
        e->bounds.x += knock * dt * 6.0f;
        if (e->hp <= 0) e->dead = true;
    }
}

int main(void)
{
    const int screen_w = 960;
    const int screen_h = 540;
    build_test_arena();

    InitWindow(screen_w, screen_h, "The Sunken Choir — Phase 1");

    Texture2D player_tex = {0};
    Texture2D bg_tex = {0};
    Texture2D tile_tex = {0};
    Texture2D enemy_tex = {0};
    bool use_player_sprite = false;
    bool use_bg = false;
    bool use_tile_sprite = false;
    bool use_enemy_sprite = false;

    use_player_sprite = load_first_hero(&player_tex);
    if (use_player_sprite)
        SetTextureFilter(player_tex, TEXTURE_FILTER_POINT);

    use_bg = load_first_bg(&bg_tex);

    use_tile_sprite = load_first_tile(&tile_tex);
    if (use_tile_sprite)
        SetTextureFilter(tile_tex, TEXTURE_FILTER_POINT);

    use_enemy_sprite = load_texture_asset(&enemy_tex, ASSET_ENEMY);
    if (use_enemy_sprite)
        SetTextureFilter(enemy_tex, TEXTURE_FILTER_POINT);

    int sprite_frame_w = (int)PLAYER_DRAW_W;
    int sprite_frame_h = (int)PLAYER_DRAW_H;
    int sprite_frame_count = 1;
    if (use_player_sprite)
        compute_horizontal_strip(player_tex, &sprite_frame_w, &sprite_frame_h, &sprite_frame_count);

    Player player = {0};
    player.position = g_spawn;
    player.facing = 1;

    DummyEnemy enemy = {0};
    Rectangle enemy_spawn_bounds;
    {
        const int fg = MAP_HEIGHT - 8;
        const int perch = fg - 11;
        float ex = 28.0f * (float)TILE_SIZE + 4.0f;
        float ey = (float)(perch * TILE_SIZE) - ENEMY_DRAW_H;
        enemy_spawn_bounds = (Rectangle){ ex, ey, ENEMY_DRAW_W, ENEMY_DRAW_H };
    }
    enemy.bounds = enemy_spawn_bounds;
    enemy.hp = ENEMY_MAX_HP;

    Camera2D cam = {0};
    cam.offset = (Vector2){ screen_w / 2.0f, screen_h / 2.0f };
    cam.zoom = 1.0f;

    float hitstop_timer = 0.0f;
    bool debug_draw = false;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float raw_dt = GetFrameTime();
        if (IsKeyPressed(KEY_F1)) debug_draw = !debug_draw;
        if (IsKeyPressed(KEY_R))
            reset_run(&player, &enemy, enemy_spawn_bounds, &hitstop_timer);

        if (hitstop_timer > 0.0f) {
            hitstop_timer -= raw_dt;
            if (hitstop_timer < 0.0f) hitstop_timer = 0.0f;
        }
        if (g_respawn_msg_timer > 0.0f) g_respawn_msg_timer -= raw_dt;

        float dt = raw_dt;
        if (hitstop_timer > 0.0f) dt = 0.0f;

        if (dt > 0.0f) physics_player(&player, dt);
        if (dt > 0.0f) try_respawn_fall(&player);

        Rectangle atk = attack_hitbox(&player);
        bool atk_active = attack_is_active(&player);
        if (dt > 0.0f)
            update_enemy(&enemy, &atk, atk_active, &hitstop_timer, player.position, dt);

        float move_in = (IsKeyDown(KEY_D) ? 1.0f : 0.0f) - (IsKeyDown(KEY_A) ? 1.0f : 0.0f);
        {
            bool walk_cycle = (player.state == PLAYER_NORMAL) && fabsf(move_in) > 0.1f && player.on_ground;
            bool run_in_air = (player.state == PLAYER_NORMAL) && fabsf(move_in) > 0.1f && !player.on_ground;
            if (walk_cycle || run_in_air) {
                player.anim_time += dt;
                float step = walk_cycle ? 0.1f : 0.12f;
                if (walk_cycle && shift_held()) step = 0.065f;
                int nfr = use_player_sprite ? sprite_frame_count : 2;
                if (nfr < 1) nfr = 1;
                if (!use_player_sprite && nfr < 2) nfr = 2;
                if (player.anim_time > step) {
                    player.anim_time = 0.0f;
                    player.anim_frame = (player.anim_frame + 1) % nfr;
                }
            } else if (player.state != PLAYER_ATTACKING) {
                player.anim_frame = 0;
                player.anim_time = 0.0f;
            }
        }

        float world_w = (float)(MAP_WIDTH * TILE_SIZE);
        float world_h = (float)(MAP_HEIGHT * TILE_SIZE);
        float half_vw = (screen_w / cam.zoom) * 0.5f;
        float half_vh = (screen_h / cam.zoom) * 0.5f;
        float px = player.position.x + PLAYER_DRAW_W * 0.5f;
        float py = player.position.y + PLAYER_DRAW_H * 0.5f;
        cam.target.x = fmaxf(half_vw, fminf(world_w - half_vw, px));
        cam.target.y = fmaxf(half_vh, fminf(world_h - half_vh, py));

        BeginDrawing();
        ClearBackground((Color){ 18, 20, 28, 255 });

        BeginMode2D(cam);
            if (use_bg) {
                DrawTextureEx(bg_tex, (Vector2){ cam.target.x * -0.15f, -40.0f }, 0.0f, 2.2f, WHITE);
            } else {
                DrawRectangleGradientV(0, 0, (int)world_w + 400, (int)world_h + 400,
                                       (Color){ 28, 32, 48, 255 }, (Color){ 12, 10, 18, 255 });
            }

            for (int y = 0; y < MAP_HEIGHT; y++) {
                for (int x = 0; x < MAP_WIDTH; x++) {
                    if (level_map[y][x] != 1) continue;
                    int px0 = x * TILE_SIZE;
                    int py0 = y * TILE_SIZE;
                    if (use_tile_sprite)
                        draw_tile_texture(tile_tex, px0, py0);
                    else
                        DrawRectangle(px0, py0, TILE_SIZE, TILE_SIZE, (Color){ 55, 52, 62, 255 });
                }
            }

            if (!enemy.dead) {
                if (use_enemy_sprite) {
                    Rectangle src = { 0.0f, 0.0f, (float)enemy_tex.width, (float)enemy_tex.height };
                    DrawTexturePro(enemy_tex, src, enemy.bounds, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
                    if (enemy.hurt_flash > 0.0f)
                        DrawRectangleRec(enemy.bounds, (Color){ 255, 255, 255, 90 });
                } else {
                    Color ec = (enemy.hurt_flash > 0.0f) ? WHITE : (Color){ 160, 72, 96, 255 };
                    DrawRectangleRec(enemy.bounds, ec);
                }
                DrawRectangleLinesEx(enemy.bounds, 2.0f, (Color){ 40, 36, 44, 255 });
            }

            if (use_player_sprite) {
                int af = player.anim_frame;
                if (sprite_frame_count > 0) af %= sprite_frame_count;
                if (af < 0) af = 0;
                Color tint = WHITE;
                if (player.state == PLAYER_ATTACKING) {
                    if (sprite_frame_count >= 2)
                        af = 1;
                    else if (attack_is_active(&player))
                        tint = (Color){ 255, 210, 170, 255 };
                }
                float fw = (float)sprite_frame_w;
                float fh = (float)sprite_frame_h;
                float frame_left = (float)(af * sprite_frame_w);
                /* Flip with negative source width (reliable in raylib); dest stays axis-aligned. */
                Rectangle fr = { frame_left, 0.0f, fw, fh };
                if (player.facing < 0) {
                    fr.x = frame_left + fw;
                    fr.width = -fw;
                }
                float dest_w = PLAYER_DRAW_W;
                float dest_h = sprite_frame_h * (PLAYER_DRAW_W / (float)sprite_frame_w);
                float draw_x = player.position.x;
                float draw_y = player.position.y + PLAYER_DRAW_H - HITBOX_PAD - dest_h;
                if (player.state == PLAYER_ATTACKING && sprite_frame_count < 2 && attack_is_active(&player))
                    draw_x += (float)player.facing * 4.0f;
                Rectangle dest = { draw_x, draw_y, dest_w, dest_h };
                DrawTexturePro(player_tex, fr, dest, (Vector2){ 0.0f, 0.0f }, 0.0f, tint);
            } else {
                Color pc = (player.iframes_timer > 0.0f && ((int)(GetTime() * 20.0) % 2 == 0))
                               ? (Color){ 200, 200, 220, 120 }
                               : (Color){ 100, 180, 220, 255 };
                if (player.state == PLAYER_ATTACKING && attack_is_active(&player))
                    pc = (Color){ 255, 200, 140, 255 };
                Vector2 dp = { player.position.x, player.position.y + PLAYER_SPRITE_Y_OFF };
                if (player.state == PLAYER_ATTACKING && attack_is_active(&player))
                    dp.x += (float)player.facing * 4.0f;
                DrawRectangleV(dp, (Vector2){ PLAYER_DRAW_W, PLAYER_DRAW_H }, pc);
            }

            if (debug_draw) {
                Rectangle hb = player_collider(player.position);
                DrawRectangleLinesEx(hb, 1.0f, GREEN);
                DrawRectangleLinesEx(atk, 1.0f, (attack_is_active(&player) ? YELLOW : GRAY));
                if (!enemy.dead) DrawRectangleLinesEx(enemy.bounds, 1.0f, ORANGE);
            }
        EndMode2D();

        DrawText("A/D move  Space jump  Shift sprint  J attack  R reset  F1 hitboxes", 12, 10, 18,
                 (Color){ 200, 198, 210, 255 });
        if (g_respawn_msg_timer > 0.0f)
            DrawText("Fell into the pit — respawned at start", 12, 34, 18, (Color){ 255, 180, 120, 255 });
        else if (enemy.dead)
            DrawText("Dummy cleared — Phase 1 arena", 12, 34, 18, LIME);

        EndDrawing();
    }

    if (use_player_sprite) UnloadTexture(player_tex);
    if (use_bg) UnloadTexture(bg_tex);
    if (use_tile_sprite) UnloadTexture(tile_tex);
    if (use_enemy_sprite) UnloadTexture(enemy_tex);
    CloseWindow();
    return 0;
}
