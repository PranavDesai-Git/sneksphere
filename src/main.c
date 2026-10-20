#include <raylib.h>
#include <stdbool.h>

#define WIDTH 900
#define HEIGHT 800

#define TARGET_FPS 60

static bool running = true;

int main(void) {
    InitWindow(WIDTH, HEIGHT, "sneksphere");
    SetTargetFPS(TARGET_FPS);
    DisableCursor();

    Camera3D camera = {0};
    camera.position = (Vector3){0.0F, 10.0F, 25.0F};
    camera.target = (Vector3){0};
    camera.up = (Vector3){0.0F, 1.0F, 0.0F};
    camera.fovy = 45.0F;
    camera.projection = CAMERA_PERSPECTIVE;

    Vector3 currPos = {0, 0, 0};
    const float radius = 2.0F;

    while (!WindowShouldClose() && running) {
        if (IsKeyPressed(KEY_Q)) {
            running = false;
        }
        if (IsKeyPressed(KEY_E)) {
            if (IsCursorHidden()) {
                EnableCursor();
            } else {
                DisableCursor();
            }
        }
        if (IsCursorHidden()) {
            camera.target = currPos;
            UpdateCamera(&camera, CAMERA_THIRD_PERSON);
        }
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);
        {
            DrawSphere(currPos, radius, RED);
            DrawGrid(10, 2.0F);
        }
        EndMode3D();
        DrawFPS(10, 10);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
