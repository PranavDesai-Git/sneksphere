#include "game.h"
#include "render/renderer.h"
#include "raylib.h"

void Game_Init(GameState *game) {
    game->screen_width = 800;
    game->screen_height = 600;
    game->should_close = false;

    game->player.x = (float)game->screen_width / 2.0f;
    game->player.y = (float)game->screen_height / 2.0f;
    game->player.radius = 24.0f;
    game->player.speed = 300.0f;

    InitWindow(game->screen_width, game->screen_height, "Sneksphere");
    SetTargetFPS(60);
}

void Game_Update(GameState *game) {
    if (WindowShouldClose()) {
        game->should_close = true;
        return;
    }

    float dt = GetFrameTime();
    float move_step = game->player.speed * dt;

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) game->player.x += move_step;
    if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) game->player.x -= move_step;
    if (IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) game->player.y += move_step;
    if (IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W)) game->player.y -= move_step;

    // Keep sphere inside window boundaries
    float r = game->player.radius;
    if (game->player.x < r) game->player.x = r;
    if (game->player.x > (float)game->screen_width - r) game->player.x = (float)game->screen_width - r;
    if (game->player.y < r) game->player.y = r;
    if (game->player.y > (float)game->screen_height - r) game->player.y = (float)game->screen_height - r;
}

void Game_Draw(const GameState *game) {
    Renderer_DrawScene(game);
}

void Game_Shutdown(GameState *game) {
    (void)game;
    CloseWindow();
}
