#include <raylib.h>
#include "gui.h"

// Função principal para iniciar e rodar o programa
int main() {
    const int SCREEN_WIDTH = 1280;             // Largura da janela
    const int SCREEN_HEIGHT = 720;             // Altura da janela

    SetConfigFlags(FLAG_FULLSCREEN_MODE);      // Configura janela para fullscreen

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Techcore - PC Components Store"); // Inicializa janela
    SetTargetFPS(60);                           // Define FPS alvo para renderização

    techcore::RunTechcoreUI(SCREEN_WIDTH, SCREEN_HEIGHT, RunLoginUI); // Roda a interface da loja com UI de login

    CloseWindow();                             // Fecha janela ao terminar
    return 0;
}
