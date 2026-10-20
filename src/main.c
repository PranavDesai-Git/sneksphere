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

    Snake snake;

    const float minDist = 1.0F;

    Node *snakeHead = malloc(sizeof(Node));
    snakeHead->next = NULL;
    snakeHead->prev = NULL;
    snakeHead->pos = (Vector3){0.0F, sphereRadius, 0.0F};

    snake.head = snakeHead;
    snake.tail = snakeHead;

    Vector3 forward = (Vector3){0.0f, 0.0f, -1.0f};

    int const initSnakeSize = 500;

    const float moveDist = 0.15F;
    const float slideSpeed = 5.0F;
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

            DrawModel(planet, (Vector3){0, 0, 0}, 1.0f, WHITE);
        }
        EndMode3D();
        DrawFPS(10, 10);
        EndDrawing();
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
