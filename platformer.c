/*
 * Phase 2 — Metroidvania spine (rooms, fade transitions, bench, gates, map, shortcut).
 * Tunables: "Gameplay tuning" below.
 * Manual test: explore Entrance→Hub; take key; bench (E); East (key) — cling + dummy → drift;
 *   Hub south (cling) → Shaft — spikes need drift; map pickup; shortcut (E) when open;
 *   pit / spikes respawn at last bench. M map if owned. R full reset. F1 hitboxes.
 *
 * Art (PNG): tries assets/<file> first, then cwd. Canonical names — see ASSET_* below.
 */

#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* --- Gameplay tuning (Phase 1) --- */
#define MAP_WIDTH  56
#define MAP_HEIGHT 40
#define TILE_SIZE  48
/* East arena floor row — must match build_room_east */
#define EAST_FLOOR_ROW (MAP_HEIGHT - 6)
#define HUB_FLOOR_ROW  (MAP_HEIGHT - 4)
#define SHAFT_TOP_ROW  6

/* 1 = start in Shaft (Hub-south vertical room) for tests; 0 = normal new game at Entrance */
#define PLAYTEST_START_IN_SHAFT 0

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

#define FADE_SPEED            4.0f
#define TRANSITION_COOLDOWN   0.18f
#define BENCH_INTERACT_R      52.0f
#define PICKUP_RADIUS         28.0f
#define MAX_DOORS             8
#define ROOM_COUNT            4

#define PLAYER_DRAW_W       48.0f
#define PLAYER_DRAW_H       64.0f
#define ENEMY_DRAW_W        48.0f
#define ENEMY_DRAW_H        64.0f
#define HITBOX_PAD            8.0f
/* Sprite drawn this many px up so feet line up with collider feet (hitbox inset was sinking art) */
#define PLAYER_SPRITE_Y_OFF (-HITBOX_PAD)

#define BASE_SCREEN_W        960
#define BASE_SCREEN_H        540
#define TOUCH_BTN_SIZE       88.0f
#define TOUCH_BTN_GAP        18.0f
#define TOUCH_BTN_ALPHA      100
#define TOUCH_BTN_TEXT_ALPHA 220
#define MOBILE_CAM_ZOOM       1.45f

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
#define MOBILE_BUILD 1
#else
#define MOBILE_BUILD 0
#endif

/* --- Canonical asset filenames (PNG). Order = load priority within category. --- */
#define ASSET_HERO_A     "sprite-hero.png"
#define ASSET_HERO_B     "medieval-rpg-main-character-d002.png"
#define ASSET_HERO_C     "player_spritesheet.png"
#define ASSET_TILE_A     "tile-ground.png"
#define ASSET_TILE_B     "tile.png"
#define ASSET_ENEMY      "sprite-simple-enemy.png"
#define ASSET_MAP        "sprite-quest-map.png"

typedef enum RoomId {
    ROOM_ENTRANCE = 0,
    ROOM_HUB = 1,
    ROOM_EAST = 2,
    ROOM_SHAFT = 3
} RoomId;

typedef enum GateNeed {
    GATE_NONE = 0,
    GATE_KEY,
    GATE_CLING,
    GATE_DRIFT,
    GATE_SHORTCUT,
    /* Hub east → arena: key to enter once; seals after Cantor so you don’t loop the same room */
    GATE_EAST_ARENA
} GateNeed;

typedef enum FadePhase {
    FADE_IDLE = 0,
    FADE_OUT,
    FADE_IN
} FadePhase;

typedef struct DoorDef {
    Rectangle zone;
    RoomId to_room;
    float dest_x;
    float dest_y;
    GateNeed need;
} DoorDef;

typedef struct PickupState {
    Vector2 pos;
    bool collected;
} PickupState;

/* World / pit: respawn if you fall past playable space (open bottom in pit) */
static Vector2 g_spawn = { 0 };
static const char *g_toast = NULL;
static float       g_toast_timer = 0.0f;

static RoomId g_room = ROOM_ENTRANCE;
static bool   g_room_seen[ROOM_COUNT];
static DoorDef g_doors[MAX_DOORS];
static int     g_ndoors = 0;
static Rectangle g_bench_zone = { 0 };
static Vector2   g_bench_spawn = { 0 };
static bool      g_has_bench_here = false;
static Rectangle g_hub_shaft_mark = { 0 };
static bool      g_has_hub_shaft_mark = false;
static Rectangle g_hazard = { 0 };
static bool    g_has_hazard = false;

static FadePhase g_fade = FADE_IDLE;
static float     g_fade_alpha = 0.0f;
static RoomId    g_pending_room = ROOM_ENTRANCE;
static float     g_pending_px = 0.0f;
static float     g_pending_py = 0.0f;

static bool g_has_cathedral_key = false;
static bool g_has_chord_cling = false;
static bool g_has_veil_drift = false;
static bool g_has_map = false;
static bool g_map_open = false;
static bool g_mobile_run_mode = true;

static bool g_dummy_defeated = false;
static bool g_shortcut_unlocked = false;
static PickupState g_pick_key = { 0 };
static PickupState g_pick_cling = { 0 };
static PickupState g_pick_map = { 0 };

static float g_gate_msg_timer = 0.0f;
static const char *g_gate_msg = NULL;
/* Brief safety after fade-in; door latch (prev-in-zone) does most of the work. */
static float g_transition_cd = 0.0f;
static bool  g_prev_in_door[MAX_DOORS];

/* Try assets/<name> then ./<name>. Returns true if texture loaded (width > 0). */
static bool load_texture_asset(Texture2D *out, const char *filename)
{
    char path[160];
#if MOBILE_BUILD
    *out = LoadTexture(filename);
    if (out->width > 0) return true;
    int nm = snprintf(path, sizeof path, "assets/%s", filename);
    if (nm > 0 && nm < (int)sizeof path) {
        *out = LoadTexture(path);
        if (out->width > 0) return true;
    }
#endif
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

typedef struct InputState {
    float move_x;
    bool run_held;
    bool jump_pressed;
    bool jump_released;
    bool attack_pressed;
    bool interact_pressed;
    bool reset_pressed;
    bool toggle_debug_pressed;
    bool toggle_map_pressed;
    bool using_touch;
} InputState;

typedef struct TouchUi {
    Rectangle left_btn;
    Rectangle right_btn;
    Rectangle run_btn;
    Rectangle jump_btn;
    Rectangle attack_btn;
    Rectangle interact_btn;
    Rectangle map_btn;
    bool left_down_prev;
    bool right_down_prev;
    bool run_down_prev;
    bool jump_down_prev;
    bool attack_down_prev;
    bool interact_down_prev;
    bool map_down_prev;
} TouchUi;

static Rectangle east_enemy_spawn(void);
static void sync_dummy_for_room(DummyEnemy *e);
static Rectangle player_collider(Vector2 pos);
static void push_toast(const char *msg, float sec);
static void gather_input(InputState *in, TouchUi *touch_ui, int screen_w, int screen_h);
static void draw_touch_controls(const TouchUi *touch_ui);

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

static void clear_level(void)
{
    memset(level_map, 0, sizeof level_map);
}

static void door_push(Rectangle z, RoomId to, float dx, float dy, GateNeed need)
{
    if (g_ndoors >= MAX_DOORS) return;
    g_doors[g_ndoors++] = (DoorDef){ z, to, dx, dy, need };
}

static bool gate_satisfied(GateNeed need)
{
    switch (need) {
        case GATE_NONE: return true;
        case GATE_KEY: return g_has_cathedral_key;
        case GATE_CLING: return g_has_chord_cling;
        case GATE_DRIFT: return g_has_veil_drift;
        case GATE_SHORTCUT: return g_shortcut_unlocked;
        case GATE_EAST_ARENA:
            return g_has_cathedral_key && !g_dummy_defeated;
        default: return true;
    }
}

static const char *gate_denial_text(GateNeed need)
{
    switch (need) {
        case GATE_KEY: return "Locked — need Cathedral Key";
        case GATE_CLING: return "Too narrow — need Chord Cling";
        case GATE_DRIFT: return "Need Veil Drift to pass";
        case GATE_SHORTCUT: return "Shortcut not open yet";
        case GATE_EAST_ARENA:
            if (!g_has_cathedral_key) return "Locked — need Cathedral Key";
            if (g_dummy_defeated) return "The east choir is sealed — take the shaft south.";
            return NULL;
        default: return NULL;
    }
}

/*
 * Entrance: key pickup; door right → Hub.
 */
static void build_room_entrance(void)
{
    clear_level();
    g_ndoors = 0;
    g_has_bench_here = false;
    g_has_hazard = false;
    const int fl = HUB_FLOOR_ROW;

    fill_rect(0, 0, MAP_WIDTH - 1, 0, 1);
    fill_rect(0, 0, 0, MAP_HEIGHT - 1, 1);
    fill_rect(0, fl, MAP_WIDTH - 1, MAP_HEIGHT - 1, 1);
    /* Right wall with doorway (gap) */
    fill_rect(MAP_WIDTH - 1, 0, MAP_WIDTH - 1, fl - 4, 1);
    fill_rect(MAP_WIDTH - 1, fl + 1, MAP_WIDTH - 1, MAP_HEIGHT - 1, 1);

    door_push((Rectangle){ (float)((MAP_WIDTH - 1) * TILE_SIZE - 8), (float)((fl - 3) * TILE_SIZE),
                           56.0f, (float)(5 * TILE_SIZE) },
              ROOM_HUB, 2.5f * (float)TILE_SIZE, (float)(fl * TILE_SIZE) - PLAYER_DRAW_H - 0.5f, GATE_NONE);

    if (!g_pick_key.collected)
        g_pick_key.pos = (Vector2){ 5.0f * (float)TILE_SIZE + 8.0f,
                                   (float)(fl * TILE_SIZE) - 48.0f };

    g_spawn = (Vector2){ 3.0f * (float)TILE_SIZE, (float)(fl * TILE_SIZE) - PLAYER_DRAW_H - 0.5f };
}

/*
 * Hub: bench; west → Entrance; east → arena (key; seals after Cantor); south → Shaft (cling).
 */
static void build_room_hub(void)
{
    clear_level();
    g_ndoors = 0;
    g_has_hazard = false;
    g_has_hub_shaft_mark = false;
    const int fl = HUB_FLOOR_ROW;

    fill_rect(0, 0, MAP_WIDTH - 1, 0, 1);
    fill_rect(0, fl, MAP_WIDTH - 1, MAP_HEIGHT - 1, 1);
    /* Left doorway */
    fill_rect(0, 0, 0, fl - 4, 1);
    fill_rect(0, fl + 1, 0, MAP_HEIGHT - 1, 1);
    /* Right doorway */
    fill_rect(MAP_WIDTH - 1, 0, MAP_WIDTH - 1, fl - 4, 1);
    fill_rect(MAP_WIDTH - 1, fl + 1, MAP_WIDTH - 1, MAP_HEIGHT - 1, 1);
    /* Ceiling mid */
    fill_rect(1, 0, MAP_WIDTH - 2, 2, 1);

    g_has_bench_here = true;
    g_bench_zone = (Rectangle){ 4.0f * (float)TILE_SIZE, (float)((fl - 2) * TILE_SIZE),
                                (float)(3 * TILE_SIZE), (float)(2 * TILE_SIZE) };
    g_bench_spawn = (Vector2){ g_bench_zone.x + g_bench_zone.width * 0.5f - PLAYER_DRAW_W * 0.5f,
                               (float)(fl * TILE_SIZE) - PLAYER_DRAW_H - 0.5f };

    door_push((Rectangle){ 8.0f, (float)((fl - 3) * TILE_SIZE), 40.0f, (float)(5 * TILE_SIZE) },
              ROOM_ENTRANCE,
              (float)((MAP_WIDTH - 3) * TILE_SIZE), (float)(fl * TILE_SIZE) - PLAYER_DRAW_H - 0.5f, GATE_NONE);

    door_push((Rectangle){ (float)((MAP_WIDTH - 1) * TILE_SIZE - 4), (float)((fl - 3) * TILE_SIZE),
                           44.0f, (float)(4 * TILE_SIZE) },
              ROOM_EAST, 6.0f * (float)TILE_SIZE,
              (float)(EAST_FLOOR_ROW * TILE_SIZE) - PLAYER_DRAW_H - 0.5f, GATE_EAST_ARENA);

    /*
     * South → Shaft: trigger must overlap *standing* on the main deck. The old rect used
     * (MAP_HEIGHT-2)*TILE (near world bottom) — player feet never crossed it, so the exit was invisible.
     */
    {
        float cx = (float)(MAP_WIDTH / 2 * TILE_SIZE);
        Rectangle south = { cx - 52.0f, (float)((fl - 1) * TILE_SIZE) - 12.0f, 104.0f, (float)(3 * TILE_SIZE) };
        g_hub_shaft_mark = south;
        g_has_hub_shaft_mark = true;
        door_push(south, ROOM_SHAFT, (float)(MAP_WIDTH / 2 * TILE_SIZE - PLAYER_DRAW_W * 0.5f),
                  (float)(SHAFT_TOP_ROW * TILE_SIZE) - PLAYER_DRAW_H - 0.5f, GATE_CLING);
    }

    g_spawn = (Vector2){ 10.0f * (float)TILE_SIZE, (float)(fl * TILE_SIZE) - PLAYER_DRAW_H - 0.5f };
}

/*
 * East arena: flat greybox combat room — no pit climb (jump height ~62px max).
 * Dummy on a low dais; Chord Cling on the main floor; shortcut south after Cantor.
 */
static void build_room_east(void)
{
    clear_level();
    g_ndoors = 0;
    g_has_bench_here = false;
    g_has_hazard = false;

    const int fg = EAST_FLOOR_ROW;

    /* Solid left/right borders with doorway gap column 0 (rows fg-3..fg) */
    fill_rect(0, 0, 0, fg - 4, 1);
    fill_rect(0, fg + 1, 0, MAP_HEIGHT - 1, 1);
    fill_rect(MAP_WIDTH - 1, 0, MAP_WIDTH - 1, fg - 4, 1);
    fill_rect(MAP_WIDTH - 1, fg + 1, MAP_WIDTH - 1, MAP_HEIGHT - 1, 1);

    /* Ceiling + continuous ground (top walk surface is row fg) */
    fill_rect(0, 0, MAP_WIDTH - 1, 0, 1);
    fill_rect(1, fg, MAP_WIDTH - 2, MAP_HEIGHT - 1, 1);
    /* Seal column 0 / last column at floor row only (door gap stays rows fg-3..fg-1) */
    fill_rect(0, fg, 0, fg, 1);
    fill_rect(MAP_WIDTH - 1, fg, MAP_WIDTH - 1, fg, 1);

    /* Small dais for the dummy — one tile up, easy jump from floor */
    fill_rect(24, fg - 1, 33, fg - 1, 1);

    fill_rect(10, 4, 16, 4, 1);
    fill_rect(40, 4, 46, 4, 1);

    /* Dest Y must use Hub floor row — using East fg here spawned you in empty air above Hub deck → fall / wrong doors */
    door_push((Rectangle){ 8.0f, (float)((fg - 3) * TILE_SIZE), 40.0f, (float)(5 * TILE_SIZE) },
              ROOM_HUB, 14.0f * (float)TILE_SIZE,
              (float)(HUB_FLOOR_ROW * TILE_SIZE) - PLAYER_DRAW_H - 0.5f, GATE_NONE);

    /* Shortcut south — must sit on the arena floor (same bug class as Hub south door if too low) */
    {
        float sx = (float)(MAP_WIDTH / 2 * TILE_SIZE);
        Rectangle sh = { sx - 64.0f, (float)((fg - 1) * TILE_SIZE) - 10.0f, 128.0f, (float)(3 * TILE_SIZE) };
        door_push(sh, ROOM_HUB, 10.0f * (float)TILE_SIZE,
                  (float)(HUB_FLOOR_ROW * TILE_SIZE) - PLAYER_DRAW_H - 0.5f, GATE_SHORTCUT);
    }

    if (!g_pick_cling.collected)
        g_pick_cling.pos = (Vector2){ 10.0f * (float)TILE_SIZE + 8.0f,
                                      (float)(fg * TILE_SIZE) - 36.0f };

    g_spawn = (Vector2){ 6.0f * (float)TILE_SIZE, (float)(fg * TILE_SIZE) - PLAYER_DRAW_H - 0.5f };
}

/*
 * Shaft: vertical drop; map pickup; hazard band needs Veil Drift; exits top→Hub, bottom→Hub.
 */
static void build_room_shaft(void)
{
    clear_level();
    g_ndoors = 0;
    g_has_bench_here = false;
    const int fl_top = SHAFT_TOP_ROW;
    const int fl_bot = MAP_HEIGHT - 4;
    const int cx = MAP_WIDTH / 2;
    /* Thinner shaft walls (was 4 tiles each) — wider walk space; inner columns ~22–33 */
    const int wall_l = cx - 8;
    const int wall_r = cx + 7;

    fill_rect(0, 0, MAP_WIDTH - 1, 0, 1);
    /*
     * One solid slab for the bottom rows — avoids seam bugs from wall fills overwriting the
     * center strip (left/right/bridge pieces left hidden holes next to wall columns).
     */
    fill_rect(0, fl_bot, MAP_WIDTH - 1, MAP_HEIGHT - 1, 1);
    fill_rect(wall_l, fl_top, wall_l + 1, MAP_HEIGHT - 3, 1);
    fill_rect(wall_r - 1, fl_top, wall_r, MAP_HEIGHT - 3, 1);
    fill_rect(cx - 7, fl_top, cx + 7, fl_top + 1, 1);
    fill_rect(cx - 5, fl_top + 8, cx + 5, fl_top + 9, 1);
    fill_rect(cx - 5, MAP_HEIGHT - 12, cx + 5, MAP_HEIGHT - 11, 1);
    /*
     * Ledges down the shaft — without these, entering from Hub is one long free-fall into the
     * kill plane → respawn at bench → feels like an endless pit loop.
     */
    for (int row = fl_top + 4; row < fl_bot - 2; row += 5) {
        if (row == fl_top + 8 || row == MAP_HEIGHT - 12) continue;
        fill_rect(cx - 6, row, cx + 6, row, 1);
    }

    /* Top exit: band near the ceiling only — not overlapping the landing (avoids Hub↔Shaft ping-pong) */
    door_push(
        (Rectangle){ (float)(cx * TILE_SIZE - 48), 20.0f, 112.0f,
                      fmaxf(40.0f, (float)(fl_top * TILE_SIZE) - PLAYER_DRAW_H - 36.0f) },
        ROOM_HUB, (float)(10 * TILE_SIZE), (float)(fl_bot * TILE_SIZE) - PLAYER_DRAW_H - 0.5f, GATE_NONE);

    /*
     * Bottom exits: allow progression by going down then left OR right.
     * Keeping a center exit too avoids a dead zone if player drops straight through the middle.
     */
    {
        float by = (float)((fl_bot - 1) * TILE_SIZE) - 14.0f;
        door_push((Rectangle){ 10.0f, by, 120.0f, (float)(3 * TILE_SIZE) },
                  ROOM_HUB, 8.0f * (float)TILE_SIZE,
                  (float)(fl_bot * TILE_SIZE) - PLAYER_DRAW_H - 0.5f, GATE_NONE);
        door_push((Rectangle){ (float)(MAP_WIDTH * TILE_SIZE) - 130.0f, by, 120.0f, (float)(3 * TILE_SIZE) },
                  ROOM_HUB, 14.0f * (float)TILE_SIZE,
                  (float)(fl_bot * TILE_SIZE) - PLAYER_DRAW_H - 0.5f, GATE_NONE);

        float bx = (float)(cx * TILE_SIZE);
        door_push((Rectangle){ bx - 52.0f, by, 104.0f, (float)(3 * TILE_SIZE) },
                  ROOM_HUB, 12.0f * (float)TILE_SIZE,
                  (float)(fl_bot * TILE_SIZE) - PLAYER_DRAW_H - 0.5f, GATE_NONE);
    }

    /*
     * No AABB “mist” kill here: the old rect overlapped the drop path and sides and snapped you to
     * g_spawn (top) every frame — felt like a reset when moving left/right. Veil Drift can gate
     * something else later; traversal must work without Cantor first.
     */
    g_has_hazard = false;

    if (!g_pick_map.collected)
        g_pick_map.pos = (Vector2){ (float)(cx * TILE_SIZE - PLAYER_DRAW_W * 0.5f),
                                    (float)(fl_top * TILE_SIZE + 2.0f * TILE_SIZE) };

    g_spawn = (Vector2){ (float)(cx * TILE_SIZE - PLAYER_DRAW_W * 0.5f),
                         (float)(fl_top * TILE_SIZE) - PLAYER_DRAW_H - 0.5f };
}

static void latch_doors_from_player(Player *p)
{
    Rectangle b = player_collider(p->position);
    for (int i = 0; i < g_ndoors; i++)
        g_prev_in_door[i] = CheckCollisionRecs(b, g_doors[i].zone);
}

static void build_room(RoomId id)
{
    memset(g_prev_in_door, 0, sizeof g_prev_in_door);
    switch (id) {
        case ROOM_ENTRANCE: build_room_entrance(); break;
        case ROOM_HUB: build_room_hub(); break;
        case ROOM_EAST: build_room_east(); break;
        case ROOM_SHAFT: build_room_shaft(); break;
        default: build_room_entrance(); break;
    }
}

static void begin_fade_to(RoomId to, float px, float py)
{
    g_pending_room = to;
    g_pending_px = px;
    g_pending_py = py;
    g_fade_alpha = 0.0f;
    g_fade = FADE_OUT;
}

static void tick_fade(Player *p, DummyEnemy *enemy, float dt)
{
    if (g_fade == FADE_IDLE) return;
    if (g_fade == FADE_OUT) {
        g_fade_alpha += dt * FADE_SPEED;
        if (g_fade_alpha >= 1.0f) {
            g_fade_alpha = 1.0f;
            g_room = g_pending_room;
            g_room_seen[(int)g_room] = true;
            build_room(g_room);
            p->position = (Vector2){ g_pending_px, g_pending_py };
            p->velocity = (Vector2){ 0.0f, 0.0f };
            p->on_ground = false;
            p->coyote_timer = 0.0f;
            p->jump_buffer_timer = 0.0f;
            p->state = PLAYER_NORMAL;
            p->state_timer = 0.0f;
            p->attack_cd_timer = 0.0f;
            sync_dummy_for_room(enemy);
            latch_doors_from_player(p);
            g_fade = FADE_IN;
        }
    } else if (g_fade == FADE_IN) {
        g_fade_alpha -= dt * FADE_SPEED;
        if (g_fade_alpha <= 0.0f) {
            g_fade_alpha = 0.0f;
            g_fade = FADE_IDLE;
            g_transition_cd = TRANSITION_COOLDOWN;
        }
    }
}

static void try_transition(Player *p)
{
    if (g_fade != FADE_IDLE) return;
    if (g_transition_cd > 0.0f) return;
    Rectangle body = player_collider(p->position);

    for (int i = 0; i < g_ndoors; i++) {
        DoorDef *d = &g_doors[i];
        bool ov = CheckCollisionRecs(body, d->zone);
        if (!ov) continue;

        if (!gate_satisfied(d->need)) {
            if (!g_prev_in_door[i]) {
                const char *msg = gate_denial_text(d->need);
                if (msg) {
                    g_gate_msg = msg;
                    g_gate_msg_timer = 2.0f;
                }
            }
            g_prev_in_door[i] = true;
            continue;
        }

        /* Rising edge only — no fade spam while standing in the doorway */
        if (!g_prev_in_door[i]) {
            begin_fade_to(d->to_room, d->dest_x, d->dest_y);
            return;
        }
        continue;
    }

    for (int i = 0; i < g_ndoors; i++)
        g_prev_in_door[i] = CheckCollisionRecs(body, g_doors[i].zone);
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

static bool point_in_rect(Vector2 p, Rectangle r)
{
    return p.x >= r.x && p.x <= (r.x + r.width) && p.y >= r.y && p.y <= (r.y + r.height);
}

static bool touch_down_in(Rectangle r)
{
    int tc = GetTouchPointCount();
    for (int i = 0; i < tc; i++) {
        Vector2 tp = GetTouchPosition(i);
        if (point_in_rect(tp, r)) return true;
    }
    return false;
}

static void layout_touch_controls(TouchUi *touch_ui, int screen_w, int screen_h)
{
    float short_side = (float)((screen_w < screen_h) ? screen_w : screen_h);
    float scale = short_side / 540.0f;
    if (scale < 0.85f) scale = 0.85f;
    if (scale > 1.25f) scale = 1.25f;

    float btn = TOUCH_BTN_SIZE * scale;
    float gap = TOUCH_BTN_GAP * scale;
    float pad = 20.0f * scale;
    bool landscape = (screen_w >= screen_h);

    if (landscape) {
        float y0 = (float)screen_h - pad - btn;
        touch_ui->left_btn = (Rectangle){ pad, y0, btn, btn };
        touch_ui->right_btn = (Rectangle){ pad + btn + gap, y0, btn, btn };
        touch_ui->run_btn = (Rectangle){ pad + 0.5f * (btn + gap), y0 - btn - gap, btn, btn };

        touch_ui->jump_btn = (Rectangle){ (float)screen_w - pad - btn, y0, btn, btn };
        touch_ui->attack_btn = (Rectangle){ (float)screen_w - pad - (btn * 2.0f) - gap, y0, btn, btn };
        touch_ui->interact_btn = (Rectangle){ (float)screen_w - pad - (btn * 2.0f) - gap, y0 - btn - gap, btn, btn };
        touch_ui->map_btn = (Rectangle){ (float)screen_w - pad - btn, y0 - btn - gap, btn, btn };
    } else {
        float y0 = (float)screen_h - pad - btn;
        touch_ui->left_btn = (Rectangle){ pad, y0, btn, btn };
        touch_ui->right_btn = (Rectangle){ pad + btn + gap, y0, btn, btn };
        touch_ui->run_btn = (Rectangle){ pad, y0 - btn - gap, btn, btn };

        touch_ui->jump_btn = (Rectangle){ (float)screen_w - pad - btn, y0 - btn - gap, btn, btn };
        touch_ui->attack_btn = (Rectangle){ (float)screen_w - pad - (btn * 2.0f) - gap, y0 - btn - gap, btn, btn };
        touch_ui->interact_btn = (Rectangle){ (float)screen_w - pad - (btn * 2.0f) - gap, y0, btn, btn };
        touch_ui->map_btn = (Rectangle){ (float)screen_w - pad - btn, y0, btn, btn };
    }
}

static void gather_input(InputState *in, TouchUi *touch_ui, int screen_w, int screen_h)
{
    memset(in, 0, sizeof *in);

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) in->move_x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) in->move_x += 1.0f;
    in->run_held = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    in->jump_pressed = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP);
    in->jump_released = IsKeyReleased(KEY_SPACE) || IsKeyReleased(KEY_W) || IsKeyReleased(KEY_UP);
    in->attack_pressed = IsKeyPressed(KEY_J) || IsKeyPressed(KEY_K);
    in->interact_pressed = IsKeyPressed(KEY_E);
    in->reset_pressed = IsKeyPressed(KEY_R);
    in->toggle_debug_pressed = IsKeyPressed(KEY_F1);
    in->toggle_map_pressed = IsKeyPressed(KEY_M);

    if (MOBILE_BUILD || GetTouchPointCount() > 0) {
        bool left_down;
        bool right_down;
        bool run_down;
        bool jump_down;
        bool attack_down;
        bool interact_down;
        bool map_down;

        layout_touch_controls(touch_ui, screen_w, screen_h);
        left_down = touch_down_in(touch_ui->left_btn);
        right_down = touch_down_in(touch_ui->right_btn);
        run_down = touch_down_in(touch_ui->run_btn);
        jump_down = touch_down_in(touch_ui->jump_btn);
        attack_down = touch_down_in(touch_ui->attack_btn);
        interact_down = touch_down_in(touch_ui->interact_btn);
        map_down = touch_down_in(touch_ui->map_btn);

        if (left_down) in->move_x -= 1.0f;
        if (right_down) in->move_x += 1.0f;
        if (run_down && !touch_ui->run_down_prev) g_mobile_run_mode = !g_mobile_run_mode;
        in->jump_pressed = in->jump_pressed || (jump_down && !touch_ui->jump_down_prev);
        in->jump_released = in->jump_released || (!jump_down && touch_ui->jump_down_prev);
        in->attack_pressed = in->attack_pressed || (attack_down && !touch_ui->attack_down_prev);
        in->interact_pressed = in->interact_pressed || (interact_down && !touch_ui->interact_down_prev);
        in->toggle_map_pressed = in->toggle_map_pressed || (map_down && !touch_ui->map_down_prev);
        in->using_touch = left_down || right_down || run_down || jump_down || attack_down || interact_down || map_down;

        touch_ui->left_down_prev = left_down;
        touch_ui->right_down_prev = right_down;
        touch_ui->run_down_prev = run_down;
        touch_ui->jump_down_prev = jump_down;
        touch_ui->attack_down_prev = attack_down;
        touch_ui->interact_down_prev = interact_down;
        touch_ui->map_down_prev = map_down;
    }

    if (in->move_x > 1.0f) in->move_x = 1.0f;
    if (in->move_x < -1.0f) in->move_x = -1.0f;

    /* Mobile: auto-run while moving so move+attack is 2-finger friendly. */
    if (MOBILE_BUILD && fabsf(in->move_x) > 0.1f) in->run_held = g_mobile_run_mode;
}

static void draw_touch_button(Rectangle btn, const char *label, bool down)
{
    Color fill = down ? (Color){ 180, 190, 230, TOUCH_BTN_ALPHA + 30 } : (Color){ 75, 78, 105, TOUCH_BTN_ALPHA };
    DrawRectangleRounded(btn, 0.20f, 8, fill);
    DrawRectangleRoundedLinesEx(btn, 0.20f, 8, 2.0f, (Color){ 210, 216, 240, 130 });
    int tw = MeasureText(label, 22);
    DrawText(label, (int)(btn.x + btn.width * 0.5f - (float)tw * 0.5f), (int)(btn.y + btn.height * 0.5f - 11.0f),
             22, (Color){ 240, 242, 255, TOUCH_BTN_TEXT_ALPHA });
}

static void draw_touch_controls(const TouchUi *touch_ui)
{
    draw_touch_button(touch_ui->left_btn, "L", touch_ui->left_down_prev);
    draw_touch_button(touch_ui->right_btn, "R", touch_ui->right_down_prev);
    draw_touch_button(touch_ui->run_btn, g_mobile_run_mode ? "Run" : "Walk", g_mobile_run_mode);
    draw_touch_button(touch_ui->jump_btn, "Jump", touch_ui->jump_down_prev);
    draw_touch_button(touch_ui->attack_btn, "Atk", touch_ui->attack_down_prev);
    draw_touch_button(touch_ui->interact_btn, "Use", touch_ui->interact_down_prev);
    draw_touch_button(touch_ui->map_btn, "Map", touch_ui->map_down_prev);
}

static void physics_player(Player *p, const InputState *in, float dt)
{
    if (p->iframes_timer > 0.0f) p->iframes_timer -= dt;
    if (p->attack_cd_timer > 0.0f) p->attack_cd_timer -= dt;

    float move_x = in->move_x;
    if (move_x > 0.1f) p->facing = 1;
    else if (move_x < -0.1f) p->facing = -1;

    float move_speed = in->run_held ? RUN_SPEED : MOVE_SPEED;

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

        if (in->jump_pressed) p->jump_buffer_timer = JUMP_BUFFER_TIME;
        if (p->jump_buffer_timer > 0.0f) p->jump_buffer_timer -= dt;

        bool can_jump = (p->on_ground || p->coyote_timer > 0.0f);
        if (can_jump && p->jump_buffer_timer > 0.0f) {
            p->velocity.y = JUMP_VELOCITY;
            p->on_ground = false;
            p->coyote_timer = 0.0f;
            p->jump_buffer_timer = 0.0f;
        }

        if (in->jump_released && p->velocity.y < 0.0f)
            p->velocity.y *= JUMP_CUT_MULT;

        if (in->attack_pressed && p->attack_cd_timer <= 0.0f && p->state == PLAYER_NORMAL) {
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

    /*
     * Shaft safety clamp: even if a tiny collision seam remains, keep the player on the
     * lower deck instead of letting them drop into kill-line respawns.
     */
    if (g_room == ROOM_SHAFT) {
        const float deck_y = (float)((MAP_HEIGHT - 4) * TILE_SIZE) - PLAYER_DRAW_H - 0.5f;
        if (p->position.y > deck_y) {
            p->position.y = deck_y;
            if (p->velocity.y > 0.0f) p->velocity.y = 0.0f;
            p->on_ground = true;
        }
    }
}

static void push_toast(const char *msg, float sec)
{
    g_toast = msg;
    g_toast_timer = sec;
}

static void try_respawn_fall(Player *p)
{
    /* Shaft uses explicit floor safety; skip global pit snap there. */
    if (g_room == ROOM_SHAFT) return;

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
    push_toast("Fell into the pit — respawned at last mercy point", 2.0f);
}

static void try_hazard_veil(Player *p)
{
    if (!g_has_hazard || g_has_veil_drift) return;
    /* Bottom deck (row fl_bot+): never treat as mist — avoids edge overlap with the hazard AABB */
    if (g_room == ROOM_SHAFT) {
        const float deck_top_y = (float)((MAP_HEIGHT - 4) * TILE_SIZE);
        if (p->position.y + PLAYER_DRAW_H >= deck_top_y - 4.0f)
            return;
    }
    Rectangle b = player_collider(p->position);
    if (!CheckCollisionRecs(b, g_hazard)) return;
    /*
     * Hub spawn is centered in the mist band (x). While you fall, every frame overlapped the
     * AABB → respawn to the top → felt like “run left/right and reset”. Only punish *standing*
     * in the veil without Veil Drift; falling/jumping through is allowed.
     */
    if (!p->on_ground) return;

    p->position = g_spawn;
    p->velocity = (Vector2){ 0.0f, 0.0f };
    p->on_ground = false;
    p->coyote_timer = 0.0f;
    p->jump_buffer_timer = 0.0f;
    p->state = PLAYER_NORMAL;
    p->state_timer = 0.0f;
    p->attack_cd_timer = 0.0f;
    p->iframes_timer = 0.45f;
    push_toast("Choir hazard — need Veil Drift (defeat Cantor)", 2.2f);
}

static float v2dist(Vector2 a, Vector2 b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

static void try_pickups(Player *p)
{
    Vector2 c = { p->position.x + PLAYER_DRAW_W * 0.5f, p->position.y + PLAYER_DRAW_H * 0.5f };

    if (!g_pick_key.collected && g_room == ROOM_ENTRANCE &&
        v2dist(c, g_pick_key.pos) < PICKUP_RADIUS) {
        g_pick_key.collected = true;
        g_has_cathedral_key = true;
        push_toast("Cathedral Key", 1.6f);
    }
    if (!g_pick_cling.collected && g_room == ROOM_EAST &&
        v2dist(c, g_pick_cling.pos) < PICKUP_RADIUS) {
        g_pick_cling.collected = true;
        g_has_chord_cling = true;
        push_toast("Chord Cling", 1.6f);
    }
    if (!g_pick_map.collected && g_room == ROOM_SHAFT &&
        v2dist(c, g_pick_map.pos) < PICKUP_RADIUS) {
        g_pick_map.collected = true;
        g_has_map = true;
        g_map_open = true;
        push_toast("Choir Chart — M / Map button toggles", 2.0f);
    }
}

static void try_bench(Player *p, const InputState *in)
{
    if (!g_has_bench_here) return;
    if (!in->interact_pressed) return;
    Rectangle b = player_collider(p->position);
    if (!CheckCollisionRecs(b, g_bench_zone)) return;

    g_spawn = g_bench_spawn;
    push_toast("Mercy Bench — respawn set", 1.8f);
}

static Rectangle east_enemy_spawn(void)
{
    const int fg = EAST_FLOOR_ROW;
    const int perch = fg - 1;
    float ex = 28.0f * (float)TILE_SIZE + 4.0f;
    float ey = (float)(perch * TILE_SIZE) - ENEMY_DRAW_H;
    return (Rectangle){ ex, ey, ENEMY_DRAW_W, ENEMY_DRAW_H };
}

static void sync_dummy_for_room(DummyEnemy *e)
{
    if (g_room != ROOM_EAST) return;
    if (g_dummy_defeated) {
        e->dead = true;
        return;
    }
    e->dead = false;
    e->hp = ENEMY_MAX_HP;
    e->hurt_iframes = 0.0f;
    e->hurt_flash = 0.0f;
    e->bounds = east_enemy_spawn();
}

/* Full run reset (R) — progression, rooms, dummy */
static void reset_run(Player *p, DummyEnemy *e, float *hitstop)
{
    g_has_cathedral_key = false;
    g_has_chord_cling = false;
    g_has_veil_drift = false;
    g_has_map = false;
    g_map_open = false;
    g_dummy_defeated = false;
    g_shortcut_unlocked = false;
    g_pick_key.collected = false;
    g_pick_cling.collected = false;
    g_pick_map.collected = false;
    g_fade = FADE_IDLE;
    g_fade_alpha = 0.0f;
    g_gate_msg = NULL;
    g_gate_msg_timer = 0.0f;
    g_toast = NULL;
    g_toast_timer = 0.0f;
    g_transition_cd = 0.0f;

    memset(g_room_seen, 0, sizeof g_room_seen);
    g_room = ROOM_ENTRANCE;
    g_room_seen[ROOM_ENTRANCE] = true;
    build_room(ROOM_ENTRANCE);

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

    e->bounds = east_enemy_spawn();
    e->hp = ENEMY_MAX_HP;
    e->hurt_iframes = 0.0f;
    e->hurt_flash = 0.0f;
    e->dead = true;
    *hitstop = 0.0f;
    latch_doors_from_player(p);
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
        if (e->hp <= 0) {
            e->dead = true;
            g_dummy_defeated = true;
            g_has_veil_drift = true;
            g_shortcut_unlocked = true;
            push_toast("Cantor falls — Veil Drift; shortcut opens", 2.5f);
        }
    }
}

static void draw_pickups_world(void)
{
    if (!g_pick_key.collected && g_room == ROOM_ENTRANCE)
        DrawCircleV(g_pick_key.pos, 10.0f, (Color){ 230, 200, 80, 255 });
    if (!g_pick_cling.collected && g_room == ROOM_EAST)
        DrawCircleV(g_pick_cling.pos, 10.0f, (Color){ 120, 200, 255, 255 });
    if (!g_pick_map.collected && g_room == ROOM_SHAFT)
        DrawCircleV(g_pick_map.pos, 10.0f, (Color){ 90, 200, 190, 255 });
}

static void draw_minimap(int screen_w)
{
    if (!g_has_map || !g_map_open) return;
    const int cell = 22;
    const int pad = 14;
    int ox = screen_w - 2 * cell - 28 - pad;
    int oy = pad;
    const char *lbl[4] = { "E", "H", "A", "S" };
    for (int i = 0; i < ROOM_COUNT; i++) {
        int cx = ox + (i % 2) * (cell + 6);
        int cy = oy + (i / 2) * (cell + 6);
        Color fill = g_room_seen[i] ? (Color){ 90, 110, 160, 220 } : (Color){ 35, 38, 48, 220 };
        if (i == (int)g_room) fill = (Color){ 210, 190, 120, 255 };
        DrawRectangle(cx, cy, cell, cell, fill);
        DrawRectangleLines(cx, cy, cell, cell, (Color){ 20, 22, 30, 255 });
        DrawText(lbl[i], cx + 6, cy + 4, 16, WHITE);
    }
    DrawText("Choir chart", ox, oy + 2 * cell + 20, 16, (Color){ 170, 168, 190, 255 });
}

int main(void)
{
    const int start_w = BASE_SCREEN_W;
    const int start_h = BASE_SCREEN_H;

    memset(g_room_seen, 0, sizeof g_room_seen);
#if PLAYTEST_START_IN_SHAFT
    g_has_chord_cling = true; /* same as after picking up Cling — south portal is open */
    g_room = ROOM_SHAFT;
    g_room_seen[ROOM_ENTRANCE] = true;
    g_room_seen[ROOM_SHAFT] = true;
#else
    g_room = ROOM_ENTRANCE;
    g_room_seen[ROOM_ENTRANCE] = true;
#endif
    build_room(g_room);

    if (!MOBILE_BUILD) SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(start_w, start_h, "The Sunken Choir — Phase 2");

    Texture2D player_tex = {0};
    Texture2D tile_tex = {0};
    Texture2D enemy_tex = {0};
    bool use_player_sprite = false;
    bool use_tile_sprite = false;
    bool use_enemy_sprite = false;

    if (!MOBILE_BUILD) {
        use_player_sprite = load_first_hero(&player_tex);
        if (use_player_sprite)
            SetTextureFilter(player_tex, TEXTURE_FILTER_POINT);

        use_tile_sprite = load_first_tile(&tile_tex);
        if (use_tile_sprite)
            SetTextureFilter(tile_tex, TEXTURE_FILTER_POINT);

        use_enemy_sprite = load_texture_asset(&enemy_tex, ASSET_ENEMY);
        if (use_enemy_sprite)
            SetTextureFilter(enemy_tex, TEXTURE_FILTER_POINT);
    }

    int sprite_frame_w = (int)PLAYER_DRAW_W;
    int sprite_frame_h = (int)PLAYER_DRAW_H;
    int sprite_frame_count = 1;
    if (use_player_sprite)
        compute_horizontal_strip(player_tex, &sprite_frame_w, &sprite_frame_h, &sprite_frame_count);

    Player player = {0};
    player.position = g_spawn;
    player.facing = 1;
    latch_doors_from_player(&player);

    DummyEnemy enemy = {0};
    enemy.bounds = east_enemy_spawn();
    enemy.hp = ENEMY_MAX_HP;
    enemy.dead = true;
    sync_dummy_for_room(&enemy);

    Camera2D cam = {0};
    cam.offset = (Vector2){ start_w / 2.0f, start_h / 2.0f };
    cam.zoom = MOBILE_BUILD ? MOBILE_CAM_ZOOM : 1.0f;

    float hitstop_timer = 0.0f;
    bool debug_draw = false;
    TouchUi touch_ui = {0};
    InputState input = {0};
    float mobile_boot_hint = 3.0f;
    bool mobile_deferred_texture_load = MOBILE_BUILD;
    float mobile_texture_load_timer = 0.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        int screen_w = GetScreenWidth();
        int screen_h = GetScreenHeight();
        float raw_dt = GetFrameTime();
        if (mobile_deferred_texture_load) {
            mobile_texture_load_timer += raw_dt;
            if (mobile_texture_load_timer > 0.40f) {
                use_player_sprite = load_first_hero(&player_tex);
                if (use_player_sprite) {
                    SetTextureFilter(player_tex, TEXTURE_FILTER_POINT);
                    compute_horizontal_strip(player_tex, &sprite_frame_w, &sprite_frame_h, &sprite_frame_count);
                }
                use_tile_sprite = load_first_tile(&tile_tex);
                if (use_tile_sprite) SetTextureFilter(tile_tex, TEXTURE_FILTER_POINT);
                use_enemy_sprite = load_texture_asset(&enemy_tex, ASSET_ENEMY);
                if (use_enemy_sprite) SetTextureFilter(enemy_tex, TEXTURE_FILTER_POINT);
                mobile_deferred_texture_load = false;
            }
        }
        if (MOBILE_BUILD && mobile_boot_hint > 0.0f) mobile_boot_hint -= raw_dt;
        gather_input(&input, &touch_ui, screen_w, screen_h);

        if (input.toggle_debug_pressed) debug_draw = !debug_draw;
        if (input.reset_pressed)
            reset_run(&player, &enemy, &hitstop_timer);
        if (input.toggle_map_pressed && g_has_map)
            g_map_open = !g_map_open;

        if (hitstop_timer > 0.0f) {
            hitstop_timer -= raw_dt;
            if (hitstop_timer < 0.0f) hitstop_timer = 0.0f;
        }
        if (g_gate_msg_timer > 0.0f) g_gate_msg_timer -= raw_dt;
        if (g_toast_timer > 0.0f) g_toast_timer -= raw_dt;
        if (g_transition_cd > 0.0f) g_transition_cd -= raw_dt;

        bool sim = (g_fade == FADE_IDLE);
        float dt = raw_dt;
        if (hitstop_timer > 0.0f) dt = 0.0f;
        if (!sim) dt = 0.0f;

        if (dt > 0.0f) physics_player(&player, &input, dt);
        if (dt > 0.0f) try_respawn_fall(&player);
        if (dt > 0.0f) try_hazard_veil(&player);
        if (dt > 0.0f) try_transition(&player);
        if (dt > 0.0f) try_pickups(&player);
        if (dt > 0.0f) try_bench(&player, &input);

        tick_fade(&player, &enemy, raw_dt);

        Rectangle atk = attack_hitbox(&player);
        bool atk_active = attack_is_active(&player);
        if (dt > 0.0f && g_room == ROOM_EAST)
            update_enemy(&enemy, &atk, atk_active, &hitstop_timer, player.position, dt);

        float move_in = input.move_x;
        {
            bool walk_cycle = (player.state == PLAYER_NORMAL) && fabsf(move_in) > 0.1f && player.on_ground;
            bool run_in_air = (player.state == PLAYER_NORMAL) && fabsf(move_in) > 0.1f && !player.on_ground;
            if (walk_cycle || run_in_air) {
                player.anim_time += dt;
                float step = walk_cycle ? 0.1f : 0.12f;
                if (walk_cycle && input.run_held) step = 0.065f;
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
        cam.offset = (Vector2){ screen_w / 2.0f, screen_h / 2.0f };
        float half_vw = (screen_w / cam.zoom) * 0.5f;
        float half_vh = (screen_h / cam.zoom) * 0.5f;
        float px = player.position.x + PLAYER_DRAW_W * 0.5f;
        float py = player.position.y + PLAYER_DRAW_H * 0.5f;
        cam.target.x = fmaxf(half_vw, fminf(world_w - half_vw, px));
        cam.target.y = fmaxf(half_vh, fminf(world_h - half_vh, py));

        BeginDrawing();
        if (MOBILE_BUILD) ClearBackground((Color){ 34, 38, 54, 255 });
        else ClearBackground((Color){ 18, 20, 28, 255 });

        BeginMode2D(cam);
            /* Void behind tiles — flat dark (no parallax / gradient / PNG for now) */
            if (MOBILE_BUILD)
                DrawRectangle(0, 0, (int)world_w + 800, (int)world_h + 800, (Color){ 24, 28, 42, 255 });
            else
                DrawRectangle(0, 0, (int)world_w + 800, (int)world_h + 800, (Color){ 10, 10, 14, 255 });

            for (int y = 0; y < MAP_HEIGHT; y++) {
                for (int x = 0; x < MAP_WIDTH; x++) {
                    if (level_map[y][x] != 1) continue;
                    int px0 = x * TILE_SIZE;
                    int py0 = y * TILE_SIZE;
                    if (use_tile_sprite)
                        draw_tile_texture(tile_tex, px0, py0);
                    else
                        DrawRectangle(px0, py0, TILE_SIZE, TILE_SIZE,
                                      MOBILE_BUILD ? (Color){ 92, 98, 120, 255 } : (Color){ 55, 52, 62, 255 });
                }
            }

            if (g_has_bench_here) {
                DrawRectangleRec(g_bench_zone, (Color){ 72, 64, 52, 120 });
                DrawRectangleLinesEx(g_bench_zone, 2.0f, (Color){ 110, 98, 82, 255 });
            }

            if (g_room == ROOM_HUB && g_has_hub_shaft_mark) {
                DrawRectangleRec(g_hub_shaft_mark, (Color){ 45, 95, 88, 75 });
                DrawRectangleLinesEx(g_hub_shaft_mark, 2.0f, (Color){ 120, 215, 195, 230 });
                DrawText("Shaft — Chord Cling", (int)(g_hub_shaft_mark.x - 20),
                         (int)(g_hub_shaft_mark.y - 22), 16, (Color){ 180, 220, 210, 255 });
            }

            draw_pickups_world();

            if (g_has_hazard && !g_has_veil_drift)
                DrawRectangleRec(g_hazard, (Color){ 180, 40, 50, 70 });

            if (g_room == ROOM_EAST && !enemy.dead) {
                if (use_enemy_sprite) {
                    Rectangle src = { 0.0f, 0.0f, (float)enemy_tex.width, (float)enemy_tex.height };
                    DrawTexturePro(enemy_tex, src, enemy.bounds, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
                    if (enemy.hurt_flash > 0.0f)
                        DrawRectangleRec(enemy.bounds, (Color){ 255, 255, 255, 90 });
                } else {
                    Color ec = (enemy.hurt_flash > 0.0f) ? WHITE : (Color){ 160, 72, 96, 255 };
                    DrawRectangleRec(enemy.bounds, ec);
                }
                if (!MOBILE_BUILD)
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
                if (g_room == ROOM_EAST && !enemy.dead) DrawRectangleLinesEx(enemy.bounds, 1.0f, ORANGE);
                if (g_has_hazard) DrawRectangleLinesEx(g_hazard, 1.0f, RED);
            }
        EndMode2D();

        if (MOBILE_BUILD)
            draw_touch_controls(&touch_ui);

        if (MOBILE_BUILD) {
            DrawRectangle(8, 8, 600, 36, (Color){ 0, 0, 0, 150 });
            DrawText("Touch: L/R + Run/Walk toggle + jump atk use map", 16, 14, 22, (Color){ 245, 245, 255, 255 });
            if (mobile_boot_hint > 0.0f) {
                DrawRectangle(40, screen_h / 2 - 70, screen_w - 80, 140, (Color){ 245, 245, 255, 225 });
                DrawText("Sunken Choir Mobile Test Build", 70, screen_h / 2 - 30, 34, (Color){ 20, 24, 35, 255 });
                DrawText("If you see this, rendering works.", 70, screen_h / 2 + 8, 24, (Color){ 35, 42, 60, 255 });
            }
        } else {
            DrawText("A/D  Space  Shift sprint  J attack  E bench  R reset  F1 debug  M chart", 12, 10,
                     17, (Color){ 200, 198, 210, 255 });
        }
        int ty = 32;
        if (g_gate_msg_timer > 0.0f && g_gate_msg != NULL) {
            DrawText(g_gate_msg, 12, ty, 17, (Color){ 255, 160, 120, 255 });
            ty += 22;
        }
        if (g_toast_timer > 0.0f && g_toast != NULL)
            DrawText(g_toast, 12, ty, 18, (Color){ 210, 215, 240, 255 });

        draw_minimap(screen_w);

        if (g_fade_alpha > 0.001f) {
            unsigned char a = (unsigned char)(fminf(g_fade_alpha, 1.0f) * 255.0f);
            DrawRectangle(0, 0, screen_w, screen_h, (Color){ 0, 0, 0, a });
        }

        EndDrawing();
    }

    if (use_player_sprite) UnloadTexture(player_tex);
    if (use_tile_sprite) UnloadTexture(tile_tex);
    if (use_enemy_sprite) UnloadTexture(enemy_tex);
    CloseWindow();
    return 0;
}
