#include <raylib.h>
#include "gui.h"

bool RunLoginUI(int screenWidth, int screenHeight);

int main() {
    // HD resolution for fullscreen
    const int SCREEN_WIDTH = 1920;
    const int SCREEN_HEIGHT = 1080;

    // Set configuration flags for high quality rendering
    SetConfigFlags(FLAG_FULLSCREEN_MODE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Techcore - PC Components Store");
    SetTargetFPS(60);

    techcore::RunTechcoreUI(SCREEN_WIDTH, SCREEN_HEIGHT, RunLoginUI);

    CloseWindow();
    return 0;
}
