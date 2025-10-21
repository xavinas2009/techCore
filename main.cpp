#include <iostream>
#include <raylib.h>
#include "gui.h"

int main () {
    // Window size; raylib helpers need a window before many calls, so pick a safe default.
    const int SCREEN_WIDTH = 1280;
    const int SCREEN_HEIGHT = 720;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Techcore - PC Components Store");
    // Optionally start fullscreen on primary monitor. Commented so it's easier to test.
    // ToggleFullscreen();

    // Run the Techcore UI (blocks until window should close)
    techcore::RunTechcoreUI(SCREEN_WIDTH, SCREEN_HEIGHT);

    CloseWindow();
    return 0;
}