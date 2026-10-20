#include <raylib.h>
#include <stdbool.h>

#define WIDTH 900
#define HEIGHT 800

#define TARGET_FPS 60

static bool running = true;

int main(void) {
    InitWindow(WIDTH, HEIGHT, "sneksphere");
    SetTargetFPS(TARGET_FPS);

    Camera3D camera = {0};
    camera.position = (Vector3){0.0F, 10.0F, 25.0F};
    camera.target = (Vector3){0};
    camera.up = (Vector3){0.0F, 1.0F, 0.0F};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Vector3 initPos = {0, 0, 0};
    const float radius = 2.0F;
    while (!WindowShouldClose() && running) {
        if (IsKeyPressed(KEY_Q)) {
            running = false;
        }
        UpdateCamera(&camera, CAMERA_ORBITAL);
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);
        {
            DrawSphere(initPos, radius, RED);
            DrawGrid(10, 25.0);
        }
        EndMode3D();
        DrawFPS(10, 10);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
