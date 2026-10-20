#include "render/renderer.h"
#include "raylib.h"

void Renderer_DrawScene(const GameState *game) {
    BeginDrawing();
    ClearBackground((Color){ 24, 24, 28, 255 });

    // Draw grid background
    for (int i = 0; i < game->screen_width; i += 40) {
        DrawLine(i, 0, i, game->screen_height, (Color){ 36, 36, 44, 255 });
    }
    for (int j = 0; j < game->screen_height; j += 40) {
        DrawLine(0, j, game->screen_width, j, (Color){ 36, 36, 44, 255 });
    }

    // Draw sphere / player
    DrawCircleGradient(
        (Vector2){ game->player.x, game->player.y },
        game->player.radius,
        (Color){ 0, 228, 130, 255 },
        (Color){ 0, 117, 65, 255 }
    );

    // Draw UI overlay
    DrawText("Sneksphere - Professional C Raylib Setup", 20, 20, 20, RAYWHITE);
    DrawText("Use WASD or Arrow Keys to move the sphere", 20, 50, 16, GRAY);
    DrawFPS(game->screen_width - 90, 20);

    EndDrawing();
}
