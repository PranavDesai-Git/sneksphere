#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

typedef struct {
    float x;
    float y;
    float radius;
    float speed;
} Player;

typedef struct {
    int screen_width;
    int screen_height;
    bool should_close;
    Player player;
} GameState;

void Game_Init(GameState *game);
void Game_Update(GameState *game);
void Game_Draw(const GameState *game);
void Game_Shutdown(GameState *game);

#endif // GAME_H
