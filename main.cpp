#include <raylib.h>
#include "gui.h"

bool RunLoginUI(int screenWidth, int screenHeight);

int main() {
    const int SCREEN_WIDTH = 1280;
    const int SCREEN_HEIGHT = 720;

    // Set fullscreen configuration flag before InitWindow
    SetConfigFlags(FLAG_FULLSCREEN_MODE);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Techcore - PC Components Store");
    SetTargetFPS(60);

    techcore::RunTechcoreUI(SCREEN_WIDTH, SCREEN_HEIGHT, RunLoginUI);

    CloseWindow();
    return 0;
}
