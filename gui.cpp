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

// Dark metallic palette inspired by the provided logo
static const Color METAL_BG = { 18, 18, 20, 255 };
static const Color METAL_PANEL = { 36, 36, 40, 255 };
static const Color METAL_ACCENT = { 96, 100, 104, 255 };
static const Color METAL_HIGHLIGHT = { 180, 180, 184, 255 };
static const Color METAL_BRONZE = { 120, 116, 112, 255 };

// Global logo texture (optional). Place a file named "logo.png" in the executable working folder
// (project root) to have it shown in the header. We'll load/unload it in RunTechcoreUI.
static Texture2D g_logoTex = {0};
static bool g_logoLoaded = false;

void DrawHeader(int screenW) {
    // header background (dark metallic gradient feel)
    DrawRectangle(0, 0, screenW, 72, METAL_BG);

    // If a logo texture is available, draw it. Otherwise draw a small placeholder badge.
    Rectangle logoBox{ 12.0f, 12.0f, 56.0f, 48.0f };
    if (g_logoLoaded && g_logoTex.id > 0) {
        // Draw the texture scaled into logoBox
        DrawTexturePro(g_logoTex,
            (Rectangle){ 0, 0, (float)g_logoTex.width, (float)g_logoTex.height },
            logoBox,
            (Vector2){0,0}, 0.0f, WHITE);
    } else {
        DrawRectangleRec(logoBox, METAL_PANEL);
        DrawRectangleLinesEx(logoBox, 2, METAL_ACCENT);
        DrawLine((int)logoBox.x + 8, (int)logoBox.y + 10, (int)logoBox.x + 8, (int)logoBox.y + 38, METAL_HIGHLIGHT);
        DrawLine((int)logoBox.x + 18, (int)logoBox.y + 10, (int)logoBox.x + 18, (int)logoBox.y + 38, METAL_HIGHLIGHT);
        DrawLine((int)logoBox.x + 26, (int)logoBox.y + 10, (int)logoBox.x + 26, (int)logoBox.y + 38, METAL_HIGHLIGHT);
    }

    DrawText("TechCore", 80, 20, 28, METAL_HIGHLIGHT);
    DrawText("Loja de Componentes", 80, 46, 14, METAL_BRONZE);
}

void DrawProductCard(const Product &p, Rectangle r) {
    DrawRectangleRec(r, METAL_PANEL);
    Rectangle thumb = { r.x + 8, r.y + 8, 84, r.height - 16 };
    DrawRectangleRec(thumb, p.color);
    DrawRectangleLinesEx(thumb, 2, METAL_ACCENT);
    DrawText(p.name.c_str(), (int)thumb.x + (int)thumb.width + 12, (int)r.y + 8, 16, RAYWHITE);
    DrawText(p.desc.c_str(), (int)thumb.x + (int)thumb.width + 12, (int)r.y + 28, 12, RAYWHITE);
    char priceBuf[64];
    sprintf(priceBuf, "€ %.2f", p.price);
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
    // Scrollbar dragging state
    bool draggingThumb = false;
    float dragStartY = 0.0f;
    float dragStartScroll = 0.0f;
    // Category filters
    std::vector<std::string> categories = { "All", "CPU", "GPU", "RAM", "Storage", "PSU", "Cooler", "MB", "Case", "Monitor" };
    std::string selectedCategory = "All";

    SetTargetFPS(60);

    // Attempt to load logo texture (optional)
    if (!g_logoLoaded) {
        if (FileExists("logo.png")) {
            g_logoTex = LoadTexture("logo.png");
            g_logoLoaded = true;
        }
    }

    while (!WindowShouldClose()) {
    // Input: mouse wheel for scroll
    float wheel = GetMouseWheelMove();
    if (wheel != 0) scrollOffset -= wheel * 28.0f; // faster scroll

    BeginDrawing();
    ClearBackground(METAL_BG);

    DrawHeader(screenWidth);

    // Cart icon in header (top-right)
    char cartCount[8]; sprintf(cartCount, "%d", (int)cart.size());
    Rectangle cartBox{ (float)screenWidth - 220, 18, 96, 36 };
    DrawRectangleRec(cartBox, Fade(METAL_PANEL, cartMenuState == CartMenuState::Showing ? 0.85f : 0.7f));
    DrawText("Carrinho", (int)cartBox.x + 8, (int)cartBox.y + 6, 14, METAL_HIGHLIGHT);
    DrawCircle((int)cartBox.x + 76, (int)cartBox.y + 18, 12, METAL_BRONZE);
    DrawText(cartCount, (int)cartBox.x + 70, (int)cartBox.y + 10, 12, METAL_HIGHLIGHT);

    // Filters button next to cart
    static bool filterDropdownOpen = false;
    static std::vector<std::string> selectedCategoriesMulti; // multi-select categories
    Rectangle filterBtn{ (float)screenWidth - 112, 18, 96, 36 };
    DrawRectangleRec(filterBtn, METAL_PANEL);
    DrawRectangleLinesEx(filterBtn, 1, METAL_ACCENT);
    DrawText("Filtros", (int)filterBtn.x + 18, (int)filterBtn.y + 6, 14, METAL_HIGHLIGHT);
    if (CheckCollisionPointRec(GetMousePosition(), filterBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        filterDropdownOpen = !filterDropdownOpen;
    }

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

    // Filters dropdown
    if (filterDropdownOpen) {
        Rectangle fbox{ filterBtn.x - 8, filterBtn.y + filterBtn.height + 6, 220, 220 };
        DrawRectangleRec(fbox, METAL_PANEL);
        DrawRectangleLinesEx(fbox, 1, METAL_ACCENT);
        DrawText("Filtros", (int)fbox.x + 12, (int)fbox.y + 8, 16, METAL_HIGHLIGHT);
        float fy = fbox.y + 36;
        for (size_t ci = 0; ci < categories.size(); ++ci) {
            std::string &cat = categories[ci];
            Rectangle chk{ fbox.x + 12, fy, 16, 16 };
            bool sel = (std::find(selectedCategoriesMulti.begin(), selectedCategoriesMulti.end(), cat) != selectedCategoriesMulti.end());
            DrawRectangleRec(chk, sel ? METAL_ACCENT : METAL_PANEL);
            DrawRectangleLinesEx(chk, 1, METAL_HIGHLIGHT);
            DrawText(cat.c_str(), (int)chk.x + 24, (int)fy - 2, 14, METAL_HIGHLIGHT);
            if (CheckCollisionPointRec(GetMousePosition(), chk) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (sel) {
                    selectedCategoriesMulti.erase(std::remove(selectedCategoriesMulti.begin(), selectedCategoriesMulti.end(), cat), selectedCategoriesMulti.end());
                } else {
                    selectedCategoriesMulti.push_back(cat);
                }
                scrollOffset = 0;
            }
            fy += 28;
        }
        // Close filters if clicked outside
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mp = GetMousePosition();
            if (!(mp.x >= fbox.x && mp.x <= fbox.x + fbox.width && mp.y >= fbox.y && mp.y <= fbox.y + fbox.height) &&
                !(mp.x >= filterBtn.x && mp.x <= filterBtn.x + filterBtn.width && mp.y >= filterBtn.y && mp.y <= filterBtn.y + filterBtn.height)) {
                filterDropdownOpen = false;
            }
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
    DrawRectangleRec(listArea, METAL_BG);

        // Search bar: hide when scrolled down to free space
        Rectangle searchBar{ listArea.x + 12, listArea.y + 12, listArea.width - 24, 36 };
        bool searchVisible = (scrollOffset < 40.0f); // hide after small scroll
        if (searchVisible) {
            DrawRectangleRec(searchBar, METAL_PANEL);
            DrawRectangleLinesEx(searchBar, 2, METAL_ACCENT);
            DrawText("Pesquisar...", (int)searchBar.x + 10, (int)searchBar.y + 8, 14, METAL_BRONZE);
            if (CheckCollisionPointRec(GetMousePosition(), searchBar) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) searchActive = true;
        }

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

    // Draw search text and caret (if visible)
    if (searchVisible) DrawText(search.c_str(), (int)searchBar.x + 10, (int)searchBar.y + 8, 14, DARKGRAY);
        caretTimer += GetFrameTime();
        if (caretTimer >= 0.5) { caretTimer = 0; caretShown = !caretShown; }
        if (searchActive && caretShown) {
            int tx = MeasureText(search.c_str(), 14);
            DrawRectangle((int)searchBar.x + 12 + tx, (int)searchBar.y + 8, 2, 16, DARKGRAY);
        }

        // Draw category filter chips (when search bar visible)
        float chipX = listArea.x + 12;
        float chipY = listArea.y + 56;
        if (searchVisible) chipY = listArea.y + 56 + 44; // push chips down a bit when search visible
        for (size_t ci = 0; ci < categories.size(); ++ci) {
            std::string &cat = categories[ci];
            Rectangle chipR{ chipX, chipY, 84, 28 };
            bool active = (cat == selectedCategory);
            DrawRectangleRec(chipR, active ? METAL_ACCENT : METAL_PANEL);
            DrawRectangleLinesEx(chipR, 1, METAL_HIGHLIGHT);
            int tx = MeasureText(cat.c_str(), 14);
            DrawText(cat.c_str(), (int)(chipR.x + (chipR.width - tx)/2), (int)(chipR.y + 6), 14, active ? METAL_BG : METAL_HIGHLIGHT);
            if (CheckCollisionPointRec(GetMousePosition(), chipR) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                selectedCategory = cat;
                // reset scroll to top whenever category changes
                scrollOffset = 0;
            }
            chipX += 92;
            if (chipX + 84 > listArea.x + listArea.width) break;
        }

        // Filter products by search and category
        std::vector<int> visibleIdx;
        for (size_t i = 0; i < products.size(); ++i) {
            std::string name = products[i].name;
            std::string desc = products[i].desc;
            std::string combined = name + " " + desc;
            // to lower
            std::string low = combined;
            std::transform(low.begin(), low.end(), low.begin(), ::tolower);
            std::string q = search; std::transform(q.begin(), q.end(), q.begin(), ::tolower);
            bool searchMatch = (q.empty() || low.find(q) != std::string::npos);
            bool categoryMatch = true;
            // multi-select filters take precedence if any selected
            if (!selectedCategoriesMulti.empty()) {
                categoryMatch = false;
                std::string lowName = name; std::transform(lowName.begin(), lowName.end(), lowName.begin(), ::tolower);
                for (auto &scat : selectedCategoriesMulti) {
                    std::string sc = scat; std::transform(sc.begin(), sc.end(), sc.begin(), ::tolower);
                    if (lowName.find(sc) != std::string::npos) { categoryMatch = true; break; }
                }
            } else if (selectedCategory != "All") {
                std::string lowName = name; std::transform(lowName.begin(), lowName.end(), lowName.begin(), ::tolower);
                std::string sc = selectedCategory; std::transform(sc.begin(), sc.end(), sc.begin(), ::tolower);
                if (lowName.find(sc) == std::string::npos) categoryMatch = false;
            }
            if (searchMatch && categoryMatch) visibleIdx.push_back((int)i);
        }

    // Grid layout (2 columns)
        int cols = 2;
        float gutter = 12.0f;
        float cardW = (listArea.width - 24 - (cols - 1) * gutter) / (float)cols;
    float contentH = ((int)std::ceil((float)visibleIdx.size() / cols)) * (cardH + gutter);
    // If search bar hidden, shift content up by its height so more cards visible
    float contentStartY = listArea.y + (searchVisible ? 56.0f : 20.0f);
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
            float y = contentStartY + row * (cardH + gutter) - scrollOffset;
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
            DrawText(products[i].name.c_str(), (int)thumb.x + (int)thumb.width + 10, (int)card.y + 12, 14, RAYWHITE);
            DrawText(products[i].desc.c_str(), (int)thumb.x + (int)thumb.width + 10, (int)card.y + 36, 12, RAYWHITE);
            char priceBuf[64]; sprintf(priceBuf, "€ %.2f", products[i].price);
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

        // Scrollbar visual on the right of list with dragging support
        if (contentH > listArea.height) {
            float barX = listArea.x + listArea.width - 8;
            float barY = listArea.y + 20;
            float barH = listArea.height - 40;
            DrawRectangle((int)barX, (int)barY, 6, (int)barH, Fade(GRAY, 0.12f));
            float thumbH = std::max(32.0f, (listArea.height / contentH) * barH);
            float maxScroll = contentH - listArea.height;
            float thumbY = barY + (scrollOffset / maxScroll) * (barH - thumbH);
            Rectangle thumbRec{ barX, thumbY, 6, thumbH };
            DrawRectangleRec(thumbRec, Fade(DARKGRAY, 0.6f));

            // Start dragging
            if (!draggingThumb && CheckCollisionPointRec(GetMousePosition(), thumbRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                draggingThumb = true;
                dragStartY = GetMouseY();
                dragStartScroll = scrollOffset;
            }
            // Continue dragging
            if (draggingThumb) {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    float dy = GetMouseY() - dragStartY;
                    float range = barH - thumbH;
                    if (range > 0) {
                        float ratio = dy / range;
                        scrollOffset = dragStartScroll + ratio * maxScroll;
                        if (scrollOffset < 0) scrollOffset = 0;
                        if (scrollOffset > maxScroll) scrollOffset = maxScroll;
                    }
                } else {
                    draggingThumb = false;
                }
            }
            // Click track to jump
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !CheckCollisionPointRec(GetMousePosition(), thumbRec)) {
                Vector2 mp = GetMousePosition();
                if (mp.x >= barX && mp.x <= barX + 6 && mp.y >= barY && mp.y <= barY + barH) {
                    float t = (mp.y - barY) / (barH - thumbH);
                    if (t < 0) t = 0; if (t > 1) t = 1;
                    scrollOffset = t * maxScroll;
                }
            }
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
            DrawText(p.name.c_str(), (int)det.x + 8, (int)det.y + 8, 18, RAYWHITE);
            DrawText(p.desc.c_str(), (int)det.x + 8, (int)det.y + 36, 14, RAYWHITE);
            char priceS[64]; sprintf(priceS, "€ %.2f", p.price);
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
                char msg[128]; sprintf(msg, "Total: € %.2f", total);
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

    // unload logo texture if loaded
    if (g_logoLoaded) {
        UnloadTexture(g_logoTex);
        g_logoLoaded = false;
    }
}

} // namespace techcore
