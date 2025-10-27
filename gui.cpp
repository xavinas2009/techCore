#include "gui.h"
#include <raylib.h>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

static const Color METAL_BG = {18, 18, 20, 255};
static const Color METAL_PANEL = {36, 36, 40, 255};
static const Color METAL_ACCENT = {96, 100, 104, 255};
static const Color METAL_HIGHLIGHT = {180, 180, 184, 255};
static const Color METAL_BRONZE = {120, 116, 112, 255};
static const Color BUTTON_BLUE = {50, 115, 220, 255};
static const Color BUTTON_RED = {190, 50, 50, 255};
static const Color TEXT_WHITE = {220, 220, 220, 255};

bool DrawButton(const char *label, Rectangle r, Color bg) {
    DrawRectangleRec(r, bg);
    int txtW = MeasureText(label, 18);
    DrawText(label, (int)(r.x + (r.width - txtW) / 2), (int)(r.y + (r.height - 18) / 2), 18, TEXT_WHITE);
    return (CheckCollisionPointRec(GetMousePosition(), r) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
}

namespace techcore {

static std::vector<Product> defaultProducts() {
    return {
        {1, "CPU: Ryzen 5 5600X", "Processador 6 núcleos para desktop.", 199.99f, ORANGE},
        {2, "CPU: Ryzen 7 5800X", "Processador 8 núcleos para desktop.", 349.99f, ORANGE},
        {3, "CPU: Ryzen 9 5950X", "Processador 16 núcleos para desktop.", 549.99f, ORANGE},
        {4, "CPU: Intel i5-12600K", "Processador 10 núcleos para desktop.", 279.99f, BLUE},
        {5, "CPU: Intel i7-12700K", "Processador 12 núcleos para desktop.", 389.99f, BLUE},
        {6, "CPU: Intel i9-12900K", "Processador 16 núcleos para desktop.", 589.99f, BLUE}
    };
}

void DrawHeader(int screenW) {
    DrawRectangle(0, 0, screenW, 72, METAL_BG);
    Rectangle logoBox{12.0f, 12.0f, 56.0f, 48.0f};
    DrawRectangleRec(logoBox, METAL_PANEL);
    DrawRectangleLinesEx(logoBox, 2, METAL_ACCENT);
    DrawLine((int)logoBox.x + 8, (int)logoBox.y + 14, (int)logoBox.x + 8, (int)logoBox.y + 38, METAL_HIGHLIGHT);
    DrawLine((int)logoBox.x + 18, (int)logoBox.y + 14, (int)logoBox.x + 18, (int)logoBox.y + 38, METAL_HIGHLIGHT);
    DrawLine((int)logoBox.x + 26, (int)logoBox.y + 14, (int)logoBox.x + 26, (int)logoBox.y + 38, METAL_HIGHLIGHT);
    DrawText("Techcore", 80, 24, 28, METAL_HIGHLIGHT);
    DrawText("Componentes", 80, 54, 15, METAL_BRONZE);
}

void RunTechcoreUI(int screenWidth, int screenHeight, bool (*LoginFunc)(int, int)) {
    std::vector<Product> products = defaultProducts();
    std::vector<CartItem> cart;
    static bool isLoggedIn = false;
    bool showLoginPrompt = false;

    std::vector<std::string> categories = {"All","CPU","GPU","RAM","Storage"};
    std::string selectedCategory = "All";
    std::string search;
    int cols = 2;
    float gutter = 18.f;
    float cardW = (screenWidth - 80 - (cols - 1)*gutter)/cols;
    float cardH = 112.f;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(METAL_BG);

        DrawHeader(screenWidth);

        float chipX = 32, chipY = 80;
        for (size_t ci=0; ci < categories.size(); ++ci) {
            Rectangle chip{chipX, chipY, 90, 28};
            bool active = (categories[ci] == selectedCategory);
            DrawRectangleRec(chip, active ? METAL_ACCENT : METAL_PANEL);
            DrawText(categories[ci].c_str(), chip.x + 14, chip.y + 6, 16, active ? METAL_BG : METAL_HIGHLIGHT);
            if (CheckCollisionPointRec(GetMousePosition(), chip) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                selectedCategory = categories[ci];
            chipX += 100;
        }

        DrawRectangle(40, 120, 320, 32, METAL_PANEL);
        DrawText("Pesquisar:", 48, 126, 16, METAL_BRONZE);
        DrawText(search.c_str(), 160, 126, 16, METAL_HIGHLIGHT);

        if (CheckCollisionPointRec(GetMousePosition(), Rectangle{40,120,320,32}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int key = GetCharPressed();
            while(key > 0) {
                if (key == KEY_BACKSPACE) { if(!search.empty()) search.pop_back(); }
                else if (key >= 32 && key <= 126) search.push_back((char)key);
                key = GetCharPressed();
            }
        }

        std::vector<int> visible;
        for (size_t i = 0; i < products.size(); ++i) {
            bool shown = true;
            if (selectedCategory != "All" && products[i].name.find(selectedCategory) == std::string::npos)
                shown = false;
            std::string low = products[i].name + products[i].desc;
            std::transform(low.begin(), low.end(), low.begin(), ::tolower);
            std::string q = search;
            std::transform(q.begin(), q.end(), q.begin(), ::tolower);
            if (!q.empty() && low.find(q) == std::string::npos)
                shown = false;
            if (shown)
                visible.push_back((int)i);
        }

        for (size_t k = 0; k < visible.size(); ++k) {
            int i = visible[k];
            int row = k / cols, col = k % cols;
            float x = 32 + col * (cardW + gutter);
            float y = 180 + row * (cardH + gutter);
            Rectangle card{ x, y, cardW, cardH };

            DrawRectangleRec(card, METAL_PANEL);
            DrawRectangleLinesEx(card, 2, METAL_ACCENT);
            DrawText(products[i].name.c_str(), x + 18, y + 18, 18, METAL_HIGHLIGHT);
            DrawText(products[i].desc.c_str(), x + 18, y + 48, 15, METAL_BRONZE);
            char priceBuf[32]; snprintf(priceBuf, sizeof(priceBuf), "€ %.2f", products[i].price);
            DrawText(priceBuf, (int)(x + cardW - 100), (int)(y + 18), 16, GOLD);

            Rectangle btn = { x + cardW - 140, y + cardH - 36, 110, 28 };
            if (DrawButton("Adicionar", btn, BUTTON_BLUE)) {
                if (!isLoggedIn)
                    showLoginPrompt = true;
                else {
                    auto it = std::find_if(cart.begin(), cart.end(), [&](const CartItem& c) { return c.product.id == products[i].id; });
                    if (it != cart.end()) it->qty += 1;
                    else cart.push_back({products[i], 1});
                }
            }
        }

        if (showLoginPrompt) {
            if (LoginFunc(screenWidth, screenHeight)) isLoggedIn = true;
            showLoginPrompt = false;
        }

        DrawText(TextFormat("Carrinho: %d", (int)cart.size()), 40, screenHeight - 44, 18, GOLD);

        EndDrawing();
    }
}

} // namespace techcore

bool RunLoginUI(int screenWidth, int screenHeight) {
    enum Mode { LOGIN, REGISTER };
    Mode mode = LOGIN;

    std::string username, password, message;
    bool typingUser = true, typingPass = false;
    bool finished = false, logged = false;

    while (!finished && !WindowShouldClose()) {
        BeginDrawing();
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.7f));
        Rectangle modal{screenWidth/2 - 170, screenHeight/2 - 100, 340, 200};
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

        Rectangle actBtn = {modal.x + 20, modal.y + 150, 100, 32};
        Rectangle switchBtn = {modal.x + 130, modal.y + 150, 100, 32};
        Rectangle cancelBtn = {modal.x + 240, modal.y + 150, 70, 32};

        if (DrawButton(mode == LOGIN ? "Login" : "Registar", actBtn, BUTTON_BLUE)) {
            if (username.empty() || password.empty()) message = "Preencha ambos campos!";
            else if (mode == LOGIN) {
                std::ifstream fi("users.txt");
                std::string u, p;
                bool ok = false;
                while (fi >> u >> p)
                    if (u == username && p == password) ok = true;
                if (ok) { logged = true; finished = true; }
                else message = "Usuário ou senha inválidos!";
            } else {
                std::ifstream fi("users.txt");
                std::string u, p;
                bool exists = false;
                while (fi >> u >> p)
                    if (u == username) exists = true;
                if (!exists) {
                    std::ofstream fo("users.txt", std::ios::app);
                    fo << username << " " << password << "\n";
                    message = "Registrado! Faça login."; 
                    mode = LOGIN; 
                    username.clear();
                    password.clear();
                } else message = "Usuário já existe!";
            }
        }

        if (DrawButton(mode == LOGIN ? "Registar" : "Login", switchBtn, METAL_ACCENT)) {
            mode = (mode == LOGIN ? REGISTER : LOGIN);
            message.clear();
            username.clear();
            password.clear();
        }

        if (DrawButton("Cancelar", cancelBtn, BUTTON_RED)) {
            finished = true;
            logged = false;
        }

        DrawText(message.c_str(), modal.x + 20, modal.y + modal.height - 34, 16, BUTTON_RED);

        Vector2 mp = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mp, userBox)) { typingUser = true; typingPass = false; }
            else if (CheckCollisionPointRec(mp, passBox)) { typingUser = false; typingPass = true; }
        }

        int key = GetCharPressed();
        while (key > 0) {
            if (typingUser && key >= 32 && key <= 126) username.push_back((char)key);
            if (typingPass && key >= 32 && key <= 126) password.push_back((char)key);
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (typingUser && !username.empty()) username.pop_back();
            if (typingPass && !password.empty()) password.pop_back();
        }

        if (IsKeyPressed(KEY_TAB)) {
            typingUser = !typingUser;
            typingPass = !typingPass;
        }

        EndDrawing();
    }
    return logged;
}
