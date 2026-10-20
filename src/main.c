#include <raylib.h>
#include <raymath.h>
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

    Snake snake;

    const float minDist = 1.0F;

    Node *snakeHead = malloc(sizeof(Node));
    snakeHead->next = NULL;
    snakeHead->pos = (Vector3){0};

    snake.head = snakeHead;
    snake.tail = snakeHead;

    Vector3 forward = (Vector3){0.0f, 0.0f, -1.0f};

    int const initSnakeSize = 80;

    const float moveDist = 0.15F;
    const float slideSpeed = 5.0F;
    const float turnSpeed = 2.0F;

    for (int i = 0; i < initSnakeSize; ++i) {
        Node *newNode = malloc(sizeof(Node));
        newNode->next = snake.tail;
        newNode->next->prev = newNode;
        newNode->prev = NULL;
        newNode->pos = Vector3Transform(snake.tail->pos,
                                        MatrixTranslate(minDist, 0.0F, 0.0F));
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
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
            forward = Vector3RotateByAxisAngle(forward, (Vector3){0, 1, 0},
                                               turnSpeed * deltaTime);
        }
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
            forward = Vector3RotateByAxisAngle(forward, (Vector3){0, 1, 0},
                                               -turnSpeed * deltaTime);
        }

        head->pos = Vector3Add(head->pos,
                               Vector3Scale(forward, slideSpeed * deltaTime));

        Node *temp = snake.head->prev;

        while (temp != NULL) {
            if (Vector3Distance(temp->pos, temp->next->pos) > minDist) {
                temp->pos =
                    Vector3MoveTowards(temp->pos, temp->next->pos, moveDist);
            }
            temp = temp->prev;
        }

        camera.position = snake.head->pos;
        camera.position.y += 10.0F;
        camera.position.z += 25.0F;
        camera.target = snake.head->pos;

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);
        {
            temp = snake.tail;
            while (temp != NULL) {
                DrawSphere(temp->pos, 0.2F, RED);
                temp = temp->next;
            }
            DrawGrid(10, 2.0F);
        }
        EndMode3D();
        DrawFPS(10, 10);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
