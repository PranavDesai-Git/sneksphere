#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdbool.h>
#include <stdlib.h>

#define WIDTH 900
#define HEIGHT 800

#define TARGET_FPS 60

static bool running = true;

typedef struct node {
    Vector3 pos;
    struct node *next;
    struct node *prev;
} Node;

typedef struct {
    Node *head;
    Node *tail;
} Snake;

static Vector3 SpawnApple(float sphereRadius) {
    Vector3 dir;
    do {
        dir.x = (float)GetRandomValue(-1000, 1000);
        dir.y = (float)GetRandomValue(-1000, 1000);
        dir.z = (float)GetRandomValue(-1000, 1000);
    } while (Vector3LengthSqr(dir) < 0.001F);

    return Vector3Scale(Vector3Normalize(dir), sphereRadius);
}

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

    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model skybox = LoadModelFromMesh(cube);

    skybox.materials[0].shader =
        LoadShader("assets/shaders/skybox.vs", "assets/shaders/skybox.fs");

    int envMap = MATERIAL_MAP_CUBEMAP;
    SetShaderValue(
        skybox.materials[0].shader,
        GetShaderLocation(skybox.materials[0].shader, "environmentMap"),
        &envMap, SHADER_UNIFORM_INT);

    Image img = LoadImage("assets/skybox.png");
    skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture =
        LoadTextureCubemap(img, CUBEMAP_LAYOUT_AUTO_DETECT);
    UnloadImage(img);

    const float sphereRadius = 10.0F;

    Mesh sphereMesh = GenMeshSphere(sphereRadius, 64, 64);
    Model planet = LoadModelFromMesh(sphereMesh);

    Shader triplanarShader = LoadShader("assets/shaders/triplanar.vs",
                                        "assets/shaders/triplanar.fs");

    float texScale = 0.4F;
    SetShaderValue(triplanarShader,
                   GetShaderLocation(triplanarShader, "texScale"), &texScale,
                   SHADER_UNIFORM_FLOAT);

    planet.materials[0].shader = triplanarShader;

    Texture2D planetTexture = LoadTexture("assets/planet.png");
    SetTextureWrap(planetTexture, TEXTURE_WRAP_REPEAT);
    planet.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = planetTexture;

    Texture2D cursorTexture = {0};
    if (FileExists("assets/cursor.png")) {
        cursorTexture = LoadTexture("assets/cursor.png");
        SetTextureFilter(cursorTexture, TEXTURE_FILTER_POINT);
    }


    Snake snake;

    const float minDist = 1.0F;
    const float appleRadius = 0.5F;
    Vector3 applePos = SpawnApple(sphereRadius);
    int score = 0;


    Node *snakeHead = malloc(sizeof(Node));
    snakeHead->next = NULL;
    snakeHead->prev = NULL;
    snakeHead->pos = (Vector3){0.0F, sphereRadius, 0.0F};

    snake.head = snakeHead;
    snake.tail = snakeHead;

    Vector3 forward = (Vector3){0.0f, 0.0f, -1.0f};

    int const initSnakeSize = 500;

    const float moveDist = 0.15F;
    float slideSpeed = 5.0F;
    const float turnSpeed = 5.0F;

    for (int i = 0; i < initSnakeSize; ++i) {
        Node *newNode = malloc(sizeof(Node));
        newNode->next = snake.tail;
        newNode->next->prev = newNode;
        newNode->prev = NULL;
        newNode->pos =
            Vector3Scale(Vector3Normalize(Vector3Add(
                             snake.tail->pos, (Vector3){0.0F, 0.0F, minDist})),
                         sphereRadius);
        snake.tail = newNode;
    }

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
        Node *head = snake.head;
        float deltaTime = GetFrameTime();
        Vector3 normal = Vector3Normalize(head->pos);

        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
            forward = Vector3RotateByAxisAngle(forward, normal,
                                               turnSpeed * deltaTime);
        }
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
            forward = Vector3RotateByAxisAngle(forward, normal,
                                               -turnSpeed * deltaTime);
        }

        head->pos = Vector3Add(head->pos,
                               Vector3Scale(forward, slideSpeed * deltaTime));
        head->pos = Vector3Scale(Vector3Normalize(head->pos), sphereRadius);

        normal = Vector3Normalize(head->pos);
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, normal));
        forward = Vector3Normalize(Vector3CrossProduct(normal, right));

        Node *temp = snake.head->prev;

        while (temp != NULL) {
            if (Vector3Distance(temp->pos, temp->next->pos) > minDist) {
                temp->pos =
                    Vector3MoveTowards(temp->pos, temp->next->pos, moveDist);
                temp->pos =
                    Vector3Scale(Vector3Normalize(temp->pos), sphereRadius);
            }
            temp = temp->prev;
        }

        // Check apple collection
        if (Vector3Distance(head->pos, applePos) < (0.2F + appleRadius)) {
            applePos = SpawnApple(sphereRadius);
            score++;

            // Grow the snake
            for (int g = 0; g < 5; ++g) {
                Node *newNode = malloc(sizeof(Node));
                newNode->next = snake.tail;
                newNode->next->prev = newNode;
                newNode->prev = NULL;
                newNode->pos = snake.tail->pos;
                snake.tail = newNode;
            }
            slideSpeed += 0.1F;
        }

        const float camHeight = 6.0F;
        const float camDistance = 12.0F;
        Vector3 camUp = Vector3Normalize(snake.head->pos);
        Vector3 upOffset = Vector3Scale(camUp, camHeight);
        Vector3 backOffset = Vector3Scale(forward, -camDistance);

        camera.position =
            Vector3Add(snake.head->pos, Vector3Add(upOffset, backOffset));
        camera.target = snake.head->pos;
        camera.up = camUp;

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);
        {
            rlDisableBackfaceCulling();
            rlDisableDepthMask();
            DrawModel(skybox, (Vector3){0, 0, 0}, 1.0f, WHITE);
            rlEnableBackfaceCulling();
            rlEnableDepthMask();
            temp = snake.tail;
            while (temp != NULL) {
                DrawSphere(temp->pos, 0.2F, RED);
                temp = temp->next;
            }

            DrawSphere(applePos, appleRadius, GOLD);
            DrawSphereWires(applePos, appleRadius + 0.02F, 8, 8, ORANGE);

            DrawModel(planet, (Vector3){0, 0, 0}, 1.0f, WHITE);
        }
        EndMode3D();

        // 2D Circular Navigation Compass pointing toward the apple
        Vector3 toApple = Vector3Subtract(applePos, snake.head->pos);
        Vector3 tangentDir = Vector3Subtract(
            toApple, Vector3Scale(normal, Vector3DotProduct(toApple, normal)));

        if (Vector3LengthSqr(tangentDir) > 0.0001F) {
            Vector3 normTangent = Vector3Normalize(tangentDir);
            Vector3 camForward =
                Vector3Normalize(Vector3Subtract(camera.target, camera.position));
            Vector3 camRight =
                Vector3Normalize(Vector3CrossProduct(camForward, camera.up));
            Vector3 cUp =
                Vector3Normalize(Vector3CrossProduct(camRight, camForward));

            float screenX = Vector3DotProduct(normTangent, camRight);
            float screenY = -Vector3DotProduct(normTangent, cUp);
            float angle = atan2f(screenY, screenX);

            Vector2 center = GetWorldToScreen(snake.head->pos, camera);
            if (center.x < 0 || center.x > (float)GetScreenWidth() ||
                center.y < 0 || center.y > (float)GetScreenHeight()) {
                center = (Vector2){(float)GetScreenWidth() / 2.0F,
                                   (float)GetScreenHeight() / 2.0F};
            }

            float ringRadius = 100.0F;

            // Position of the cursor along the circular orbit around the snake
            Vector2 cursorPos = {center.x + cosf(angle) * ringRadius,
                                 center.y + sinf(angle) * ringRadius};

            // Draw custom pixel art cursor if loaded
            if (cursorTexture.id > 0) {
                float scale = 2.0F;
                Rectangle srcRec = {0.0F, 0.0F, (float)cursorTexture.width,
                                    (float)cursorTexture.height};
                Rectangle destRec = {cursorPos.x, cursorPos.y,
                                     (float)cursorTexture.width * scale,
                                     (float)cursorTexture.height * scale};
                Vector2 origin = {destRec.width / 2.0F, destRec.height / 2.0F};
                DrawTexturePro(cursorTexture, srcRec, destRec, origin,
                               angle * RAD2DEG, WHITE);
            }
        }
        DrawFPS(10, 10);
        DrawText(TextFormat("Apples: %i", score), 10, 35, 20, GOLD);
        float distToApple = Vector3Distance(snake.head->pos, applePos);
        DrawText(TextFormat("Distance: %.1f", (double)distToApple), 10, 60, 18,
                 LIGHTGRAY);
        EndDrawing();
    }
    if (cursorTexture.id > 0) {
        UnloadTexture(cursorTexture);
    }
    UnloadTexture(planetTexture);
    UnloadShader(triplanarShader);
    UnloadModel(planet);
    UnloadShader(skybox.materials[0].shader);
    UnloadTexture(skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
    UnloadModel(skybox);
    CloseWindow();
    return 0;
}
