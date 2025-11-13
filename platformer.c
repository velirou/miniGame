#include "raylib.h"
#include <math.h>

#define MAP_WIDTH  20
#define MAP_HEIGHT 10
#define TILE_SIZE  64

int levelMap[MAP_HEIGHT][MAP_WIDTH] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,0,0,0,1,1,1,0,0,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

typedef struct Player {
    Vector2 position;
    Vector2 velocity;
    bool isOnGround;
    int frame;
    float frameTime;
} Player;

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "2D Platformer - raylib");

    // Load textures
    Texture2D playerTex = LoadTexture("player_spritesheet.png");
    Texture2D background = LoadTexture("background.png");
    Texture2D tileTex = LoadTexture("tile.png");

    // Player setup
    Player player = {0};
    player.position = (Vector2){100, 300};
    player.isOnGround = false;
    player.frame = 0;
    player.frameTime = 0;

    const float gravity = 1000;
    const float jumpForce = -500;
    const float moveSpeed = 200;

    // Animation info
    int frameWidth = playerTex.width / 4;   // assuming 4 frames
    int frameHeight = playerTex.height;

    // Camera setup
    Camera2D camera = {0};
    camera.target = player.position;
    camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
    camera.zoom = 1.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // --- INPUT ---
        float moveX = 0;
        if (IsKeyDown(KEY_A)) moveX = -1;
        if (IsKeyDown(KEY_D)) moveX = 1;

        // Jump
        if (IsKeyPressed(KEY_SPACE) && player.isOnGround)
        {
            player.velocity.y = jumpForce;
            player.isOnGround = false;
        }

        // --- PHYSICS ---
        player.velocity.x = moveX * moveSpeed;
        player.velocity.y += gravity * dt;
        player.position.x += player.velocity.x * dt;
        player.position.y += player.velocity.y * dt;

        // --- COLLISION ---
        player.isOnGround = false;
        Rectangle playerRect = {player.position.x, player.position.y, frameWidth, frameHeight};

        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            for (int x = 0; x < MAP_WIDTH; x++)
            {
                if (levelMap[y][x] == 1)
                {
                    Rectangle tileRect = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                    if (CheckCollisionRecs(playerRect, tileRect))
                    {
                        if (player.velocity.y > 0) // falling
                        {
                            player.position.y = tileRect.y - frameHeight;
                            player.velocity.y = 0;
                            player.isOnGround = true;
                        }
                    }
                }
            }
        }

        // --- ANIMATION ---
        if (fabs(moveX) > 0.1f && player.isOnGround)
        {
            player.frameTime += dt;
            if (player.frameTime > 0.1f)
            {
                player.frame = (player.frame + 1) % 4;
                player.frameTime = 0;
            }
        }
        else
        {
            player.frame = 0;
        }

        // --- CAMERA ---
        camera.target = (Vector2){player.position.x + frameWidth / 2, player.position.y + frameHeight / 2};

        // --- DRAW ---
        BeginDrawing();
            ClearBackground(SKYBLUE);

            BeginMode2D(camera);
                // Parallax background (moves slower)
                DrawTextureEx(background, (Vector2){camera.target.x * -0.2f, 0}, 0.0f, 2.0f, WHITE);

                // Draw tiles
                for (int y = 0; y < MAP_HEIGHT; y++)
                {
                    for (int x = 0; x < MAP_WIDTH; x++)
                    {
                        if (levelMap[y][x] == 1)
                            DrawTexture(tileTex, x * TILE_SIZE, y * TILE_SIZE, WHITE);
                    }
                }

                // Draw player
                Rectangle frameRec = {player.frame * frameWidth, 0, frameWidth, frameHeight};
                DrawTextureRec(playerTex, frameRec, player.position, WHITE);

            EndMode2D();

            DrawText("Use A/D to move, SPACE to jump", 10, 10, 20, DARKGRAY);
        EndDrawing();
    }

    // Cleanup
    UnloadTexture(playerTex);
    UnloadTexture(tileTex);
    UnloadTexture(background);
    CloseWindow();
    return 0;
}
