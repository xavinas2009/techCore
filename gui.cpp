#include "gui.h"
#include <raylib.h>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cmath>

namespace techcore {

static std::vector<Product> defaultProducts() {
    return {
        // Processadores
        {1, "CPU: Ryzen 5 5600X", "Processador 6 núcleos para desktop.", 199.99f, ORANGE},
        {2, "CPU: Ryzen 7 5800X", "Processador 8 núcleos para desktop.", 349.99f, ORANGE},
        {3, "CPU: Ryzen 9 5950X", "Processador 16 núcleos para desktop.", 549.99f, ORANGE},
        {4, "CPU: Intel i5-12600K", "Processador 10 núcleos para desktop.", 279.99f, BLUE},
        {5, "CPU: Intel i7-12700K", "Processador 12 núcleos para desktop.", 389.99f, BLUE},
        {6, "CPU: Intel i9-12900K", "Processador 16 núcleos para desktop.", 589.99f, BLUE},
        
        // Placas de Vídeo
        {7, "GPU: RTX 3060", "Placa gráfica 12GB GDDR6.", 299.99f, DARKBLUE},
        {8, "GPU: RTX 3070", "Placa gráfica 8GB GDDR6.", 499.99f, DARKBLUE},
        {9, "GPU: RTX 3080", "Placa gráfica 10GB GDDR6X.", 699.99f, DARKBLUE},
        {10, "GPU: RX 6600 XT", "Placa gráfica AMD 8GB GDDR6.", 379.99f, RED},
        {11, "GPU: RX 6700 XT", "Placa gráfica AMD 12GB GDDR6.", 479.99f, RED},
        {12, "GPU: RX 6800 XT", "Placa gráfica AMD 16GB GDDR6.", 649.99f, RED},
        
        // Memória RAM
        {13, "RAM: 16GB DDR4 3200", "Kit memória 16GB (2x8GB) 3200MHz.", 79.99f, LIME},
        {14, "RAM: 32GB DDR4 3600", "Kit memória 32GB (2x16GB) 3600MHz.", 159.99f, LIME},
        {15, "RAM: 64GB DDR4 3200", "Kit memória 64GB (4x16GB) 3200MHz.", 299.99f, LIME},
        {16, "RAM: 16GB DDR5 5200", "Kit memória 16GB (2x8GB) 5200MHz.", 149.99f, GREEN},
        {17, "RAM: 32GB DDR5 5600", "Kit memória 32GB (2x16GB) 5600MHz.", 249.99f, GREEN},
        
        // Armazenamento
        {18, "SSD: 500GB NVMe", "SSD NVMe PCIe 4.0 500GB.", 69.99f, SKYBLUE},
        {19, "SSD: 1TB NVMe", "SSD NVMe PCIe 4.0 1TB.", 119.99f, SKYBLUE},
        {20, "SSD: 2TB NVMe", "SSD NVMe PCIe 4.0 2TB.", 219.99f, SKYBLUE},
        {21, "HDD: 2TB", "HD Sata 3 7200RPM 2TB.", 59.99f, DARKGRAY},
        {22, "HDD: 4TB", "HD Sata 3 7200RPM 4TB.", 89.99f, DARKGRAY},
        
        // Fontes
        {23, "PSU: 650W Gold", "Fonte 650W 80+ Gold Modular.", 89.99f, PURPLE},
        {24, "PSU: 750W Gold", "Fonte 750W 80+ Gold Modular.", 109.99f, PURPLE},
        {25, "PSU: 850W Gold", "Fonte 850W 80+ Gold Modular.", 129.99f, PURPLE},
        {26, "PSU: 1000W Platinum", "Fonte 1000W 80+ Platinum Modular.", 199.99f, PURPLE},
        
        // Refrigeração
        {27, "Cooler: Air 120mm", "Cooler para CPU 120mm silencioso.", 29.99f, MAROON},
        {28, "Cooler: Air 240mm", "Cooler para CPU duplo 240mm.", 49.99f, MAROON},
        {29, "Water: 240mm AIO", "Water cooler 240mm All-in-One.", 119.99f, MAROON},
        {30, "Water: 360mm AIO", "Water cooler 360mm All-in-One.", 169.99f, MAROON},
        
        // Placas-Mãe
        {31, "MB: B550 ATX", "Placa-mãe AMD B550 ATX.", 139.99f, BEIGE},
        {32, "MB: X570 ATX", "Placa-mãe AMD X570 ATX.", 249.99f, BEIGE},
        {33, "MB: B660 ATX", "Placa-mãe Intel B660 ATX.", 149.99f, BEIGE},
        {34, "MB: Z690 ATX", "Placa-mãe Intel Z690 ATX.", 299.99f, BEIGE},
        
        // Gabinetes
        {35, "Case: Mid Tower", "Gabinete ATX Mid Tower.", 69.99f, GRAY},
        {36, "Case: Full Tower", "Gabinete ATX Full Tower.", 129.99f, GRAY},
        {37, "Case: Mini ITX", "Gabinete Mini ITX Compacto.", 89.99f, GRAY},
        
        // Periféricos
        {38, "Monitor: 1080p 144Hz", "Monitor Gaming 24' 1080p 144Hz.", 199.99f, DARKPURPLE},
        {39, "Monitor: 1440p 165Hz", "Monitor Gaming 27' 1440p 165Hz.", 349.99f, DARKPURPLE},
        {40, "Monitor: 4K 60Hz", "Monitor 27' 4K 60Hz IPS.", 399.99f, DARKPURPLE}
    };
}

void DrawHeader(int screenW) {
    // header background
    DrawRectangle(0, 0, screenW, 72, Fade(SKYBLUE, 0.85f));
    DrawText("Techcore", 20, 18, 32, RAYWHITE);
    DrawText("Loja de Componentes", 20, 44, 14, Fade(RAYWHITE, 0.9f));
}

void DrawProductCard(const Product &p, Rectangle r) {
    DrawRectangleRec(r, Fade(GRAY, 0.12f));
    Rectangle thumb = { r.x + 8, r.y + 8, 84, r.height - 16 };
    DrawRectangleRec(thumb, p.color);
    DrawText(p.name.c_str(), (int)thumb.x + (int)thumb.width + 12, (int)r.y + 8, 16, WHITE);
    DrawText(p.desc.c_str(), (int)thumb.x + (int)thumb.width + 12, (int)r.y + 28, 12, LIGHTGRAY);
    char priceBuf[64];
    sprintf(priceBuf, "R$ %.2f", p.price);
    DrawText(priceBuf, (int)r.x + (int)r.width - 110, (int)r.y + 12, 18, GOLD);
}

// Helper: draw a small button and return true if clicked
static bool DrawButton(const char* label, Rectangle r, Color bg = BLUE) {
    DrawRectangleRec(r, bg);
    int txtW = MeasureText(label, 14);
    DrawText(label, (int)(r.x + (r.width - txtW)/2), (int)(r.y + (r.height - 14)/2), 14, RAYWHITE);
    if (CheckCollisionPointRec(GetMousePosition(), r) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return true;
    return false;
}

void RunTechcoreUI(int screenWidth, int screenHeight) {
    std::vector<Product> products = defaultProducts();
    std::vector<CartItem> cart;

    int selectedIndex = -1;

    // Scroll offset for product list
    float scrollOffset = 0.0f;
    const int cardH = 100;

    bool showingConfirm = false;
    CartMenuState cartMenuState = CartMenuState::Hidden;
    // Search / typing state
    std::string search = "";
    bool searchActive = false;
    double caretTimer = 0.0;
    bool caretShown = true;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
    // Input: mouse wheel for scroll
    float wheel = GetMouseWheelMove();
    if (wheel != 0) scrollOffset -= wheel * 28.0f; // faster scroll

        BeginDrawing();
        ClearBackground(RAYWHITE);

    DrawHeader(screenWidth);

    // Cart icon in header (top-right)
    char cartCount[8]; sprintf(cartCount, "%d", (int)cart.size());
    Rectangle cartBox{ (float)screenWidth - 120, 18, 96, 36 };
    DrawRectangleRec(cartBox, Fade(DARKGRAY, cartMenuState == CartMenuState::Showing ? 0.3f : 0.1f));
    DrawText("Carrinho", (int)cartBox.x + 8, (int)cartBox.y + 6, 14, RAYWHITE);
    DrawCircle((int)cartBox.x + 76, (int)cartBox.y + 18, 12, RED);
    DrawText(cartCount, (int)cartBox.x + 70, (int)cartBox.y + 10, 12, RAYWHITE);

    // Cart menu toggle
    if (CheckCollisionPointRec(GetMousePosition(), cartBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        cartMenuState = cartMenuState == CartMenuState::Hidden ? CartMenuState::Showing : CartMenuState::Hidden;
    }

    // Cart dropdown menu
    if (cartMenuState == CartMenuState::Showing && cart.size() > 0) {
        Rectangle menuBox{ cartBox.x, cartBox.y + cartBox.height + 4, cartBox.width + 40, 80 };
        DrawRectangleRec(menuBox, Fade(DARKGRAY, 0.95f));
        
        // Continue shopping button
        Rectangle continueBtn{ menuBox.x + 8, menuBox.y + 8, menuBox.width - 16, 28 };
        if (DrawButton("Continuar Comprando", continueBtn, BLUE)) {
            cartMenuState = CartMenuState::Hidden;
        }
        
        // Checkout button
        Rectangle checkoutBtn{ menuBox.x + 8, menuBox.y + 44, menuBox.width - 16, 28 };
        if (DrawButton("Finalizar Compra", checkoutBtn, GREEN)) {
            showingConfirm = true;
            cartMenuState = CartMenuState::Hidden;
        }
    }

    // Close cart menu if clicked outside
    if (cartMenuState == CartMenuState::Showing && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Rectangle menuArea{ cartBox.x, cartBox.y, cartBox.width + 40, cartBox.height + 84 };
        if (!CheckCollisionPointRec(GetMousePosition(), menuArea)) {
            cartMenuState = CartMenuState::Hidden;
        }
    }

    int padding = 16;
    // Use most of the window for the product list to keep UI clean
    int leftW = screenWidth - padding * 2;

    Rectangle listArea{(float)padding, 88.0f, (float)leftW, (float)screenHeight - 120.0f};
    DrawRectangleRec(listArea, Fade(RAYWHITE, 1.0f));

        // Search bar
        Rectangle searchBar{ listArea.x + 12, listArea.y + 12, listArea.width - 24, 36 };
        DrawRectangleRec(searchBar, Fade(LIGHTGRAY, 0.08f));
        DrawRectangleLinesEx(searchBar, 2, Fade(GRAY, 0.2f));
        DrawText("Pesquisar...", (int)searchBar.x + 10, (int)searchBar.y + 8, 14, Fade(DARKGRAY, 0.6f));
        if (CheckCollisionPointRec(GetMousePosition(), searchBar) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) searchActive = true;

        // If active, capture keyboard chars
        if (searchActive) {
            int key = GetKeyPressed();
            while (key > 0) { // this gets last pressed key if multiple
                int c = GetCharPressed();
                if (c > 0) {
                    if (c == 13 || c == 10) { searchActive = false; break; }
                    if (c == 8) { if (!search.empty()) search.pop_back(); }
                    else search.push_back((char)c);
                }
                key = GetKeyPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE)) { if (!search.empty()) search.pop_back(); }
        }

        // Draw search text and caret
        DrawText(search.c_str(), (int)searchBar.x + 10, (int)searchBar.y + 8, 14, DARKGRAY);
        caretTimer += GetFrameTime();
        if (caretTimer >= 0.5) { caretTimer = 0; caretShown = !caretShown; }
        if (searchActive && caretShown) {
            int tx = MeasureText(search.c_str(), 14);
            DrawRectangle((int)searchBar.x + 12 + tx, (int)searchBar.y + 8, 2, 16, DARKGRAY);
        }

        // Filter products by search
        std::vector<int> visibleIdx;
        for (size_t i = 0; i < products.size(); ++i) {
            std::string name = products[i].name;
            std::string desc = products[i].desc;
            std::string combined = name + " " + desc;
            // to lower
            std::string low = combined;
            std::transform(low.begin(), low.end(), low.begin(), ::tolower);
            std::string q = search; std::transform(q.begin(), q.end(), q.begin(), ::tolower);
            if (q.empty() || low.find(q) != std::string::npos) visibleIdx.push_back((int)i);
        }

        // Grid layout (2 columns)
        int cols = 2;
        float gutter = 12.0f;
        float cardW = (listArea.width - 24 - (cols - 1) * gutter) / (float)cols;
        float contentH = ((int)std::ceil((float)visibleIdx.size() / cols)) * (cardH + gutter);
        // Clamp scroll
        if (contentH > listArea.height) {
            if (scrollOffset < 0) scrollOffset = 0;
            if (scrollOffset > contentH - listArea.height) scrollOffset = contentH - listArea.height;
        } else {
            scrollOffset = 0;
        }
        if (contentH > listArea.height) {
            if (scrollOffset < 0) scrollOffset = 0;
            if (scrollOffset > contentH - listArea.height) scrollOffset = contentH - listArea.height;
        } else {
            scrollOffset = 0;
        }

        // Scissor / BeginClipping to list area
        BeginScissorMode((int)listArea.x, (int)listArea.y, (int)listArea.width, (int)listArea.height);
        for (size_t k = 0; k < visibleIdx.size(); ++k) {
            int i = visibleIdx[k];
            int row = (int)k / cols;
            int col = (int)k % cols;
            float x = listArea.x + 12 + col * (cardW + gutter);
            float y = listArea.y + 56 + row * (cardH + gutter) - scrollOffset;
            Rectangle card{ x, y, cardW, (float)cardH };
            if (card.y + card.height < listArea.y || card.y > listArea.y + listArea.height) continue;

            // Hover and selection
            if (CheckCollisionPointRec(GetMousePosition(), card)) {
                DrawRectangleRec(card, Fade(LIGHTGRAY, 0.06f));
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedIndex = i;
            } else {
                DrawRectangleRec(card, Fade(LIGHTGRAY, 0.02f));
            }

            // Draw thumbnail and texts
            Rectangle thumb{ card.x + 10, card.y + 10, 76, card.height - 20 };
            DrawRectangleRec(thumb, products[i].color);
            DrawText(products[i].name.c_str(), (int)thumb.x + (int)thumb.width + 10, (int)card.y + 12, 14, DARKGRAY);
            DrawText(products[i].desc.c_str(), (int)thumb.x + (int)thumb.width + 10, (int)card.y + 36, 12, GRAY);
            char priceBuf[64]; sprintf(priceBuf, "R$ %.2f", products[i].price);
            DrawText(priceBuf, (int)card.x + (int)card.width - 100, (int)card.y + 14, 16, GOLD);

            // Add button
            Rectangle addBtn{ card.x + card.width - 120, card.y + card.height - 40, 108, 30 };
            DrawRectangleRec(addBtn, Fade(BLUE, 0.9f));
            DrawText("Adicionar", (int)addBtn.x + 16, (int)addBtn.y + 6, 14, RAYWHITE);
            if (CheckCollisionPointRec(GetMousePosition(), addBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                auto it = std::find_if(cart.begin(), cart.end(), [&](const CartItem &ci){ return ci.product.id == products[i].id; });
                if (it != cart.end()) it->qty += 1; else cart.push_back({products[i], 1});
            }
        }
        EndScissorMode();

        // Scrollbar visual on the right of list
        if (contentH > listArea.height) {
            float barX = listArea.x + listArea.width - 8;
            float barY = listArea.y + 56;
            float barH = listArea.height - 64;
            DrawRectangle((int)barX, (int)barY, 6, (int)barH, Fade(GRAY, 0.12f));
            float thumbH = std::max(32.0f, (listArea.height / contentH) * barH);
            float thumbY = barY + (scrollOffset / (contentH - listArea.height)) * (barH - thumbH);
            DrawRectangle((int)barX, (int)thumbY, 6, (int)thumbH, Fade(DARKGRAY, 0.6f));
        }

        // Right pane removed to keep UI clean. Cart is accessible via the top-right cart menu.
        float total = 0.0f;
        for (size_t i = 0; i < cart.size(); ++i) {
            total += cart[i].product.price * cart[i].qty;
        }

        // Product details + add to cart
        if (selectedIndex >= 0 && selectedIndex < (int)products.size()) {
            const Product &p = products[selectedIndex];
            Rectangle det{ (float)padding + 12, screenHeight - 180.0f, (float)leftW - 24, 156 };
            DrawRectangleRec(det, Fade(LIGHTGRAY, 0.03f));
            DrawText(p.name.c_str(), (int)det.x + 8, (int)det.y + 8, 18, DARKGRAY);
            DrawText(p.desc.c_str(), (int)det.x + 8, (int)det.y + 36, 14, GRAY);
            char priceS[64]; sprintf(priceS, "R$ %.2f", p.price);
            DrawText(priceS, (int)det.x + 8, (int)det.y + 64, 16, GOLD);

            Rectangle addBtn{ det.x + det.width - 140, det.y + det.height - 48, 120, 36 };
            if (DrawButton("Adicionar ao Carrinho", addBtn, BLUE)) {
                // if exists, increment qty
                auto it = std::find_if(cart.begin(), cart.end(), [&](const CartItem &ci){ return ci.product.id == p.id; });
                if (it != cart.end()) it->qty += 1; else cart.push_back({p, 1});
            }
        }

        // Confirmation modal
        if (showingConfirm) {
            Rectangle modal{ screenWidth/2 - 220, screenHeight/2 - 90, 440, 180 };
            DrawRectangleRec(modal, Fade(BLACK, 0.6f));
            DrawRectangleRec({modal.x+4, modal.y+4, modal.width-8, modal.height-8}, RAYWHITE);
            DrawText("Confirmar compra?", (int)modal.x + 20, (int)modal.y + 20, 20, DARKGRAY);
            char msg[128]; sprintf(msg, "Total: R$ %.2f", total);
            DrawText(msg, (int)modal.x + 20, (int)modal.y + 56, 16, GRAY);

            Rectangle ok{ modal.x + 40, modal.y + modal.height - 64, 140, 36 };
            Rectangle cancel{ modal.x + modal.width - 180, modal.y + modal.height - 64, 140, 36 };
            if (DrawButton("Confirmar", ok, GREEN)) {
                // simple checkout: clear cart and show a tiny notification
                cart.clear();
                showingConfirm = false;
            }
            if (DrawButton("Cancelar", cancel, RED)) {
                showingConfirm = false;
            }
        }

        EndDrawing();
    }
}

} // namespace techcore
