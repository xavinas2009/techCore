#include "gui.h"
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

// Cores do tema da interface
static const Color METAL_BG = {18, 18, 20, 255};       // Cor de fundo principal
static const Color METAL_PANEL = {36, 36, 40, 255};    // Cor dos painéis e áreas secundárias
static const Color METAL_ACCENT = {96, 100, 104, 255}; // Cor para bordas e destaques
static const Color METAL_HIGHLIGHT = {180, 180, 184, 255}; // Cor para textos e elementos em destaque
static const Color METAL_BRONZE = {120, 116, 112, 255};// Cor para textos auxiliares
static const Color BUTTON_BLUE = {50, 115, 220, 255};  // Cor do botão padrão
static const Color BUTTON_RED = {190, 50, 50, 255};    // Cor de botões de ação crítica (ex: cancelar)
static const Color TEXT_WHITE = {220, 220, 220, 255};  // Cor padrão do texto
static const Color GOLD = {255, 215, 0, 255};          // Cor para preços e destaques em amarelo


// Função que desenha um botão com um rótulo, retângulo e cor de fundo.
// Retorna true se o mouse clicar no botão.
bool DrawButton(const char *label, Rectangle r, Color bg) {
    DrawRectangleRec(r, bg);  // Desenha botão com a cor BG
    int txtW = MeasureText(label, 18);  // Mede largura do texto
    DrawText(label, (int)(r.x + (r.width - txtW) / 2), (int)(r.y + (r.height - 18) / 2), 18, TEXT_WHITE);  // Desenha texto centrado
    // Verifica se mouse está sobre o botão e clicou
    return (CheckCollisionPointRec(GetMousePosition(), r) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
}

namespace techcore {

// Retorna uma lista padrão de produtos disponíveis
std::vector<Product> defaultProducts() {
    return {
        {1, "CPU: Ryzen 5 5600X", "Processador 6 núcleos para desktop.", 199.99f, BUTTON_BLUE},
        {2, "CPU: Ryzen 7 5800X", "Processador 8 núcleos para desktop.", 349.99f, BUTTON_BLUE},
        {3, "GPU: RTX 3060", "Placa gráfica 12GB GDDR6.", 299.99f, BUTTON_BLUE},
        {4, "RAM: 16GB DDR4 3200", "Memória RAM DDR4 16GB 3200MHz.", 79.99f, BUTTON_BLUE}
    };
}

// Desenha o cabeçalho com nome da loja e botão carrinho com contagem
void DrawHeader(int screenW, int cartCount, bool highlightCartBtn) {
    DrawRectangle(0, 0, screenW, 72, METAL_BG);  // Fundo barra superior
    DrawText("Techcore", 20, 20, 32, METAL_HIGHLIGHT);  // Nome da loja

    float btnW = 120, btnH = 40;
    float btnX = screenW - btnW - 32, btnY = 16;
    Rectangle cartBtn = {btnX, btnY, btnW, btnH};

    DrawRectangleRec(cartBtn, highlightCartBtn ? BUTTON_BLUE : METAL_PANEL); // Botão carrinho com destaque
    DrawRectangleLinesEx(cartBtn, 2, METAL_ACCENT);

    std::string text = "Carrinho";
    if(cartCount > 0) text += " (" + std::to_string(cartCount) + ")";
    int txW = MeasureText(text.c_str(), 18);
    DrawText(text.c_str(), (int)(btnX + btnW / 2 - txW / 2), (int)(btnY + btnH / 2 - 9), 18, TEXT_WHITE);  // Texto no botão
}

// Exibe o modal do carrinho com itens, total e botões de ação
void ShowCartModal(int screenWidth, int screenHeight, std::vector<CartItem>& cart, bool& showModal, std::string& cartMessage) {
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.72f));  // Fundo escuro atrás do modal

    int mw = 480, mh = 380;
    Rectangle modal{(float)(screenWidth / 2 - mw / 2), (float)(screenHeight / 2 - mh / 2), (float)mw, (float)mh};
    
    DrawRectangleRec(modal, METAL_PANEL);  // Fundo do modal
    DrawRectangleLinesEx(modal, 2, METAL_ACCENT);  // Borda do modal

    DrawText("Carrinho de Compras", modal.x + 20, modal.y + 12, 24, METAL_HIGHLIGHT);  // Título

    int y = (int)modal.y + 50;
    float total = 0.0f;

    // Mostra mensagem se carrinho vazio
    if(cart.empty()) {
        DrawText("O carrinho está vazio.", modal.x + 20, y, 18, METAL_BRONZE);
    } else {
        // Mostra cada item do carrinho
        for(size_t i = 0; i < cart.size(); ++i) {
            const CartItem& item = cart[i];
            char line[256];
            snprintf(line, sizeof(line), "%dx %s  -  €%.2f", item.qty, item.product.name.c_str(), item.product.price * item.qty);
            DrawText(line, modal.x + 24, y, 18, METAL_HIGHLIGHT);
            y += 28;  // Linha seguinte
            total += item.product.price * item.qty;
        }
    }

    char totalBuf[64];
    snprintf(totalBuf, sizeof(totalBuf), "Total: € %.2f", total);
    DrawText(totalBuf, modal.x + 24, modal.y + mh - 90, 20, METAL_BRONZE);  // Total da compra

    Rectangle btnFinalizar{modal.x + 30, modal.y + mh - 48, 160, 32};
    Rectangle btnContinuar{modal.x + modal.width - 190, modal.y + mh - 48, 160, 32};

    if(DrawButton("Finalizar compra", btnFinalizar, BUTTON_BLUE)) {
        cartMessage = "Compra registada! (Simulação)";
        showModal = false;
        cart.clear();
    }
    if(DrawButton("Continuar a comprar", btnContinuar, METAL_ACCENT)) {
        showModal = false;
    }

    if(!cartMessage.empty()) {
        DrawText(cartMessage.c_str(), modal.x + 24, modal.y + mh - 110, 18, BUTTON_RED);
    }
}

// Função que executa a interface completa da loja
void RunTechcoreUI(int screenWidth, int screenHeight, bool (*LoginFunc)(int, int)) {
    std::vector<Product> products = defaultProducts();
    std::vector<CartItem> cart;
    static bool isLoggedIn = false;
    bool showLoginPrompt = false;

    std::vector<std::string> categories = {"All","CPU","GPU","RAM","Storage"};
    std::string selectedCategory = "All";
    std::string search;
    bool isSearchActive = false;
    bool showCart = false;
    std::string cartMessage;
    int cols = 2;
    float gutter = 18.f;
    float cardW = (screenWidth - 80 - (cols - 1) * gutter) / cols;
    float cardH = 112.f;

    // Loop principal para desenhar e processar eventos
    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(METAL_BG);

        float btnW = 120, btnH = 40;
        float btnX = screenWidth - btnW - 32, btnY = 16;
        Rectangle cartBtn = {btnX, btnY, btnW, btnH};
        Vector2 mp = GetMousePosition();
        bool highlightCart = CheckCollisionPointRec(mp, cartBtn);

        DrawHeader(screenWidth, (int)cart.size(), highlightCart);

        if(CheckCollisionPointRec(mp, cartBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            showCart = true;
            cartMessage.clear();
        }

        // Aqui pode continuar com barra de busca, lista de produtos e interações

        if(showCart) {
            ShowCartModal(screenWidth, screenHeight, cart, showCart, cartMessage);
        }

        EndDrawing();
    }
}

} // namespace techcore

// Modal de login para usuário autenticar ou registrar
bool RunLoginUI(int screenWidth, int screenHeight) {
    enum Mode { LOGIN, REGISTER };
    Mode mode = LOGIN;
    std::string username, password, message;
    bool typingUser = true, typingPass = false;
    bool finished = false, logged = false;

    while(!finished && !WindowShouldClose()) {
        BeginDrawing();
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.72f));

        Rectangle modal{(float)(screenWidth / 2 - 170), (float)(screenHeight / 2 - 100), 340, 200};
        DrawRectangleRec(modal, METAL_PANEL);
        DrawText(mode == LOGIN ? "Login" : "Registar", modal.x + 24, modal.y + 14, 20, METAL_HIGHLIGHT);

        Rectangle userBox = {modal.x + 30, modal.y + 60, 280, 28};
        DrawRectangleRec(userBox, typingUser ? METAL_ACCENT : METAL_PANEL);
        DrawText("Nome:", userBox.x - 18, userBox.y - 18, 16, METAL_BRONZE);
        DrawText(username.c_str(), userBox.x + 10, userBox.y + 8, 16, TEXT_WHITE);

        Rectangle passBox = {modal.x + 30, modal.y + 100, 280, 28};
        DrawRectangleRec(passBox, typingPass ? METAL_ACCENT : METAL_PANEL);
        DrawText("Senha:", passBox.x - 18, passBox.y - 18, 16, METAL_BRONZE);
        DrawText(std::string(password.size(), '*').c_str(), passBox.x + 10, passBox.y + 8, 16, TEXT_WHITE);

        Rectangle actBtn = {modal.x + 14, modal.y + 150, 100, 28};
        Rectangle switchBtn = {modal.x + 128, modal.y + 150, 90, 28};
        Rectangle cancelBtn = {modal.x + 230, modal.y + 150, 80, 28};

        if(DrawButton(mode == LOGIN ? "Login" : "Registar", actBtn, BUTTON_BLUE)) {
            if(username.empty() || password.empty()) message = "[translate:Preencha ambos campos!]";
            else if(mode == LOGIN){
                std::ifstream fi("users.txt"); std::string u, p; bool ok = false;
                while(fi >> u >> p) if(u == username && p == password) ok = true;
                if(ok) { logged = true; finished = true; }
                else message = "[translate:Usuário ou senha inválidos!]";
            } else {
                std::ifstream fi("users.txt"); std::string u, p; bool exists = false;
                while(fi >> u >> p) if(u == username) exists = true;
                if(!exists){
                    std::ofstream fo("users.txt", std::ios::app);
                    fo << username << " " << password << "\n";
                    message = "[translate:Registrado! Faça login.]";
                    mode = LOGIN; username.clear(); password.clear();
                } else message = "[translate:Usuário já existe!]";
            }
        }

        if(DrawButton(mode == LOGIN ? "Registar" : "Login", switchBtn, METAL_ACCENT)) {
            mode = (mode == LOGIN ? REGISTER : LOGIN);
            message.clear(); username.clear(); password.clear();
        }

        if(DrawButton("[translate:Cancelar]", cancelBtn, BUTTON_RED)) {
            finished = true; logged = false;
        }

        DrawText(message.c_str(), modal.x + 22, modal.y + modal.height - 34, 16, BUTTON_RED);

        Vector2 mp = GetMousePosition();
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            if(CheckCollisionPointRec(mp, userBox)) typingUser = true, typingPass = false;
            else if(CheckCollisionPointRec(mp, passBox)) typingUser = false, typingPass = true;
        }

        int key = GetCharPressed();
        while(key > 0){
            if(typingUser && key >= 32 && key <= 126) username.push_back((char)key);
            if(typingPass && key >= 32 && key <= 126) password.push_back((char)key);
            key = GetCharPressed();
        }

        if(IsKeyPressed(KEY_BACKSPACE)){
            if(typingUser && !username.empty()) username.pop_back();
            if(typingPass && !password.empty()) password.pop_back();
        }

        if(IsKeyPressed(KEY_TAB)) typingUser = !typingUser, typingPass = !typingPass;

        EndDrawing();
    }

    return logged;
}
