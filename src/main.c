#include "game.h"

int main(void) {
    GameState game;

    Game_Init(&game);

    while (!game.should_close) {
        Game_Update(&game);
        Game_Draw(&game);
    }

    Game_Shutdown(&game);

    return 0;
}
