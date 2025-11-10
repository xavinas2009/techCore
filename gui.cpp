#include "gui.h"
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <map>
#include <direct.h> // For _mkdir on Windows

// Global font variable
static Font customFont;
static bool fontLoaded = false;

// Image cache for product thumbnails
static std::map<std::string, Texture2D> imageCache;
static Texture2D placeholderImage;
static bool placeholderLoaded = false;

// Product quantity selector cache (for selecting quantity before adding to cart)
static std::map<int, int> productQuantitySelector;

// Particle system for ambient background effects
struct Particle {
    Vector2 position;
    Vector2 velocity;
    float size;
    float alpha;
    Color color;
};

static std::vector<Particle> particles;
static bool particlesInitialized = false;

void InitParticles(int screenWidth, int screenHeight) {
    if(particlesInitialized) return;
    particles.clear();
    for(int i = 0; i < 50; i++) {
        Particle p;
        p.position = {(float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight)};
        p.velocity = {(float)GetRandomValue(-20, 20) / 100.0f, (float)GetRandomValue(-30, -10) / 100.0f};
        p.size = (float)GetRandomValue(1, 4);
        p.alpha = (float)GetRandomValue(10, 40) / 100.0f;
        p.color = {100, 150, 255, (unsigned char)(p.alpha * 255)};
        particles.push_back(p);
    }
    particlesInitialized = true;
}

void UpdateParticles(int screenWidth, int screenHeight) {
    for(auto& p : particles) {
        p.position.x += p.velocity.x;
        p.position.y += p.velocity.y;
        
        // Wrap around screen
        if(p.position.x < 0) p.position.x = screenWidth;
        if(p.position.x > screenWidth) p.position.x = 0;
        if(p.position.y < 0) p.position.y = screenHeight;
        if(p.position.y > screenHeight) p.position.y = 0;
    }
}

void DrawParticles() {
    for(const auto& p : particles) {
        DrawCircleV(p.position, p.size, Fade(p.color, p.alpha));
        // Subtle glow
        DrawCircleV(p.position, p.size * 1.5f, Fade(p.color, p.alpha * 0.3f));
    }
}

// Modern Premium Theme colors - Enhanced visuals
static const Color METAL_BG = {12, 12, 15, 255};            // Darker, richer background
static const Color METAL_PANEL = {24, 24, 28, 255};         // Elevated surface
static const Color METAL_ACCENT = {55, 60, 72, 255};        // Refined accent
static const Color METAL_HIGHLIGHT = {245, 245, 250, 255};  // Bright highlights
static const Color METAL_BRONZE = {185, 180, 175, 255};     // Soft secondary text
static const Color BUTTON_BLUE = {37, 99, 235, 255};        // Vibrant primary blue
static const Color BUTTON_BLUE_HOVER = {59, 130, 246, 255}; // Lighter hover
static const Color BUTTON_RED = {239, 68, 68, 255};         // Bright red
static const Color TEXT_WHITE = {255, 255, 255, 255};       // Pure white
static const Color TEXT_GRAY = {156, 163, 175, 255};        // Subtle gray
static const Color SHADOW_COLOR = {0, 0, 0, 120};           // Deeper shadows
static const Color CARD_HOVER = {32, 35, 42, 255};          // Hover state
static const Color SUCCESS_GREEN = {16, 185, 129, 255};     // Fresh green
static const Color SUCCESS_GREEN_HOVER = {52, 211, 153, 255}; // Lighter green
static const Color PREMIUM_GOLD = {251, 191, 36, 255};      // Gold accent
static const Color PREMIUM_PURPLE = {147, 51, 234, 255};    // Premium purple

// Helper function to draw text with custom font (HD rendering)
void DrawTextCustom(const char* text, int posX, int posY, int fontSize, Color color) {
    if(fontLoaded) {
        // Use higher spacing for better kerning
        float spacing = fontSize * 0.08f;
        DrawTextEx(customFont, text, {(float)posX, (float)posY}, (float)fontSize, spacing, color);
    } else {
        DrawTextCustom(text, posX, posY, fontSize, color);
    }
}

// Helper function to measure text with custom font
int MeasureTextCustom(const char* text, int fontSize) {
    if(fontLoaded) {
        float spacing = fontSize * 0.08f;
        Vector2 size = MeasureTextEx(customFont, text, (float)fontSize, spacing);
        return (int)size.x;
    } else {
        return MeasureTextCustom(text, fontSize);
    }
}

// Helper function to draw text with shadow for better readability
void DrawTextWithShadow(const char* text, int posX, int posY, int fontSize, Color color, int shadowOffset = 2) {
    // Draw shadow
    DrawTextCustom(text, posX + shadowOffset, posY + shadowOffset, fontSize, SHADOW_COLOR);
    // Draw main text
    DrawTextCustom(text, posX, posY, fontSize, color);
}

// Premium gradient text for special elements
void DrawTextGradient(const char* text, int posX, int posY, int fontSize, Color topColor, Color bottomColor) {
    if(fontLoaded) {
        float spacing = fontSize * 0.08f;
        Vector2 textSize = MeasureTextEx(customFont, text, (float)fontSize, spacing);
        
        // Create gradient by drawing text multiple times with different alpha
        for(int i = 0; i < textSize.y; i++) {
            float ratio = (float)i / textSize.y;
            Color blendColor = {
                (unsigned char)(topColor.r * (1-ratio) + bottomColor.r * ratio),
                (unsigned char)(topColor.g * (1-ratio) + bottomColor.g * ratio),
                (unsigned char)(topColor.b * (1-ratio) + bottomColor.b * ratio),
                (unsigned char)(topColor.a * (1-ratio) + bottomColor.a * ratio)
            };
            
            // Draw line by line with scissor to create gradient effect
            BeginScissorMode(posX, posY + i, textSize.x, 1);
            DrawTextEx(customFont, text, {(float)posX, (float)posY}, (float)fontSize, spacing, blendColor);
            EndScissorMode();
        }
    } else {
        DrawTextCustom(text, posX, posY, fontSize, topColor);
    }
}

// Ultra-enhanced Button utility with smooth transitions
bool DrawButton(const char *label, Rectangle r, Color bg) {
    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, r);
    bool isPressed = isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    // Smooth scale animation on hover
    static std::map<std::string, float> buttonScales;
    std::string btnKey = std::string(label) + std::to_string((int)r.x);
    if(buttonScales.find(btnKey) == buttonScales.end()) buttonScales[btnKey] = 1.0f;
    float targetScale = isHovered ? 1.05f : 1.0f;
    buttonScales[btnKey] += (targetScale - buttonScales[btnKey]) * 0.25f;
    
    // Apply scale
    float scaleOffset = (r.width * buttonScales[btnKey] - r.width) / 2;
    Rectangle scaledR = {r.x - scaleOffset, r.y - scaleOffset, r.width * buttonScales[btnKey], r.height * buttonScales[btnKey]};
    
    Color btnColor = bg;
    if (isHovered && bg.r == BUTTON_BLUE.r && bg.g == BUTTON_BLUE.g && bg.b == BUTTON_BLUE.b) {
        btnColor = BUTTON_BLUE_HOVER;
    } else if (isHovered) {
        btnColor = ColorBrightness(bg, 0.2f);
    }
    
    // Ultra-enhanced multi-layer shadow with blur
    DrawRectangleRounded({scaledR.x + 3, scaledR.y + 6, scaledR.width, scaledR.height}, 0.25f, 8, Fade(SHADOW_COLOR, 0.5f));
    DrawRectangleRounded({scaledR.x + 2, scaledR.y + 3, scaledR.width, scaledR.height}, 0.25f, 8, Fade(SHADOW_COLOR, 0.3f));
    DrawRectangleRounded({scaledR.x + 1, scaledR.y + 1, scaledR.width, scaledR.height}, 0.25f, 8, Fade(SHADOW_COLOR, 0.15f));
    
    // Animated outer glow on hover
    if(isHovered) {
        static float glowPulse = 0.0f;
        glowPulse += 0.08f;
        float glowSize = 3.0f + sin(glowPulse) * 1.5f;
        DrawRectangleRounded({scaledR.x - glowSize, scaledR.y - glowSize, 
                             scaledR.width + glowSize*2, scaledR.height + glowSize*2}, 
                            0.25f, 8, Fade(btnColor, 0.2f + sin(glowPulse) * 0.1f));
    }
    
    // Button with gradient
    DrawRectangleGradientV(scaledR.x, scaledR.y, scaledR.width, scaledR.height,
                          btnColor, ColorBrightness(btnColor, -0.15f));
    
    // Premium top shine
    Rectangle topShine = {scaledR.x, scaledR.y, scaledR.width, scaledR.height / 2.5f};
    DrawRectangleRounded(topShine, 0.25f, 8, Fade(WHITE, 0.15f));
    
    // Animated border
    if(isHovered) {
        static float borderPulse = 0.0f;
        borderPulse += 0.1f;
        DrawRectangleLinesEx(scaledR, 3.0f, Fade(METAL_HIGHLIGHT, 0.7f + sin(borderPulse) * 0.2f));
    }
    DrawRectangleLinesEx(scaledR, 2.0f, isHovered ? METAL_HIGHLIGHT : METAL_ACCENT);
    
    int txtW = MeasureTextCustom(label, 20);
    DrawTextWithShadow(label, (int)(scaledR.x + (scaledR.width-txtW)/2), (int)(scaledR.y+(scaledR.height-20)/2), 20, TEXT_WHITE, 2);
    
    return isPressed;
}

// Helper function to create a placeholder image
Texture2D CreatePlaceholderTexture() {
    Image img = GenImageColor(100, 100, METAL_ACCENT);
    ImageDrawRectangle(&img, 10, 10, 80, 80, METAL_PANEL);
    ImageDrawRectangle(&img, 40, 30, 20, 40, METAL_HIGHLIGHT);
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}

// Helper function to load product image with caching
Texture2D GetProductImage(const std::string& imagePath) {
    if(imagePath.empty() || imagePath == "none") {
        if(!placeholderLoaded) {
            placeholderImage = CreatePlaceholderTexture();
            placeholderLoaded = true;
        }
        return placeholderImage;
    }
    
    // Check cache first
    if(imageCache.find(imagePath) != imageCache.end()) {
        return imageCache[imagePath];
    }
    
    // Try to load the image
    if(FileExists(imagePath.c_str())) {
        Image img = LoadImage(imagePath.c_str());
        ImageResize(&img, 100, 100); // Resize to thumbnail
        Texture2D texture = LoadTextureFromImage(img);
        UnloadImage(img);
        imageCache[imagePath] = texture;
        return texture;
    }
    
    // Return placeholder if file doesn't exist
    if(!placeholderLoaded) {
        placeholderImage = CreatePlaceholderTexture();
        placeholderLoaded = true;
    }
    return placeholderImage;
}

namespace techcore {

// Helper function to convert category to string
std::string CategoryToString(ProductCategory cat) {
    switch(cat) {
        case ProductCategory::CPU: return "CPU";
        case ProductCategory::GPU: return "GPU";
        case ProductCategory::RAM: return "RAM";
        case ProductCategory::STORAGE: return "Storage";
        case ProductCategory::MOTHERBOARD: return "Motherboard";
        case ProductCategory::PSU: return "PSU";
        case ProductCategory::COOLING: return "Cooling";
        case ProductCategory::CASE: return "Case";
        case ProductCategory::PERIPHERAL: return "Peripheral";
        default: return "Unknown";
    }
}

// Helper function to convert string to category
ProductCategory StringToCategory(const std::string& str) {
    if(str == "CPU") return ProductCategory::CPU;
    if(str == "GPU") return ProductCategory::GPU;
    if(str == "RAM") return ProductCategory::RAM;
    if(str == "Storage") return ProductCategory::STORAGE;
    if(str == "Motherboard") return ProductCategory::MOTHERBOARD;
    if(str == "PSU") return ProductCategory::PSU;
    if(str == "Cooling") return ProductCategory::COOLING;
    if(str == "Case") return ProductCategory::CASE;
    if(str == "Peripheral") return ProductCategory::PERIPHERAL;
    return ProductCategory::CPU;
}

std::vector<Product> defaultProducts() {
    // Return empty vector - products should be added via admin panel
    return {};
}

void DrawHeader(int screenW, int cartCount, bool highlightCartBtn, bool highlightAdminBtn, bool isLoggedIn, bool highlightUserBtn, const std::string& username) {
    // Load logo image (once)
    static Texture2D logoImage = {0};
    static bool logoLoaded = false;
    if(!logoLoaded) {
        if(FileExists("thumbnails/logo.jpg")) {
            Image img = LoadImage("thumbnails/logo.jpg");
            ImageResize(&img, 120, 64);
            logoImage = LoadTextureFromImage(img);
            UnloadImage(img);
            logoLoaded = true;
        }
    }
    
    // Premium header with enhanced gradient
    DrawRectangleGradientV(0, 0, screenW, 82, {18, 18, 22, 255}, METAL_BG);
    // Subtle top highlight
    DrawRectangle(0, 0, screenW, 1, Fade(METAL_HIGHLIGHT, 0.1f));
    // Bottom border with glow
    DrawRectangle(0, 82, screenW, 3, BUTTON_BLUE);
    DrawRectangle(0, 85, screenW, 1, Fade(BUTTON_BLUE, 0.5f));
    
    // Logo box with shadow and modern design
    Rectangle logoBox{16.0f, 8.0f, 120.0f, 64.0f};
    
    // Draw logo image if loaded, otherwise draw text
    if(logoLoaded && logoImage.id > 0) {
        DrawTexture(logoImage, logoBox.x, logoBox.y, WHITE);
    } else {
        DrawRectangle(logoBox.x + 2, logoBox.y + 2, logoBox.width, logoBox.height, Fade(SHADOW_COLOR, 0.4f));
        DrawRectangleRounded(logoBox, 0.15f, 8, BUTTON_BLUE);
        DrawRectangleLinesEx(logoBox, 2.0f, BUTTON_BLUE_HOVER);
        // Logo text fallback
        DrawTextCustom("TC", logoBox.x + 14, logoBox.y + 14, 24, TEXT_WHITE);
    }
    
    // Title removed (logo-only header)

    // Admin button (only if logged in)
    if(isLoggedIn) {
        float adminBtnW = 140, adminBtnH = 44;
        float adminBtnX = screenW - adminBtnW - 340, adminBtnY = 18;
        Rectangle adminBtn = {adminBtnX, adminBtnY, adminBtnW, adminBtnH};
        
        Color adminBg = highlightAdminBtn ? ColorBrightness(PURPLE, 0.2f) : PURPLE;
        
        DrawRectangle(adminBtn.x + 2, adminBtn.y + 2, adminBtn.width, adminBtn.height, Fade(SHADOW_COLOR, 0.3f));
        DrawRectangleRounded(adminBtn, 0.25f, 8, adminBg);
        DrawRectangleLinesEx(adminBtn, 2.0f, METAL_HIGHLIGHT);
        
        const char* adminText = "[*] Admin";
        int adminTxW = MeasureTextCustom(adminText, 18);
        DrawTextCustom(adminText, (int)(adminBtnX + adminBtnW/2 - adminTxW/2), (int)(adminBtnY + adminBtnH/2 - 9), 18, TEXT_WHITE);
    }
    
    // User profile button (logged in) or Login button (not logged in)
    float userBtnW = 150, userBtnH = 44;
    float userBtnX = screenW - userBtnW - 180, userBtnY = 18;
    Rectangle userBtn = {userBtnX, userBtnY, userBtnW, userBtnH};
    
    Color userBg = highlightUserBtn ? SUCCESS_GREEN_HOVER : SUCCESS_GREEN;
    if(!isLoggedIn) userBg = highlightUserBtn ? METAL_ACCENT : METAL_PANEL;
    
    DrawRectangle(userBtn.x + 2, userBtn.y + 2, userBtn.width, userBtn.height, Fade(SHADOW_COLOR, 0.3f));
    DrawRectangleRounded(userBtn, 0.25f, 8, userBg);
    DrawRectangleLinesEx(userBtn, 2.0f, METAL_HIGHLIGHT);
    
    std::string userText = isLoggedIn ? "[@] " + username : "[!] Login";
    int userTxW = MeasureTextCustom(userText.c_str(), 18);
    DrawTextCustom(userText.c_str(), (int)(userBtnX + userBtnW/2 - userTxW/2), (int)(userBtnY + userBtnH/2 - 9), 18, TEXT_WHITE);

    // Cart button with modern styling
    float btnW = 140, btnH = 44;
    float btnX = screenW - btnW - 24, btnY = 18;
    Rectangle cartBtn = {btnX, btnY, btnW, btnH};
    
    Color cartBg = highlightCartBtn ? BUTTON_BLUE_HOVER : BUTTON_BLUE;
    
    // Shadow
    DrawRectangle(cartBtn.x + 2, cartBtn.y + 2, cartBtn.width, cartBtn.height, Fade(SHADOW_COLOR, 0.3f));
    
    // Button
    DrawRectangleRounded(cartBtn, 0.25f, 8, cartBg);
    DrawRectangleLinesEx(cartBtn, 2.0f, METAL_HIGHLIGHT);
    
    std::string text = "[$] Carrinho";
    if(cartCount > 0) {
        text += " (" + std::to_string(cartCount) + ")";
        // Badge for item count
        Rectangle badge = {cartBtn.x + cartBtn.width - 28, cartBtn.y + 4, 24, 16};
        DrawRectangleRounded(badge, 0.5f, 8, BUTTON_RED);
        DrawTextCustom(std::to_string(cartCount).c_str(), badge.x + 8, badge.y + 2, 12, TEXT_WHITE);
        text = "[$] Carrinho";
    }
    int txW = MeasureTextCustom(text.c_str(),18);
    DrawTextCustom(text.c_str(), (int)(btnX+btnW/2-txW/2), (int)(btnY+btnH/2-9), 18, TEXT_WHITE);
}

void ShowCartModal(int screenWidth, int screenHeight, std::vector<CartItem>& cart, bool& showModal, std::string& cartMessage, bool& showCheckout, bool isLoggedIn, bool& showLoginPrompt) {
    // Ultra-premium backdrop with radial gradient
    DrawRectangle(0,0,screenWidth,screenHeight, Fade(BLACK,0.85f));
    
    // Animated vignette effect
    static float vignetteTime = 0.0f;
    vignetteTime += GetFrameTime() * 0.5f;
    float vignettePulse = 0.3f + sin(vignetteTime) * 0.05f;
    DrawCircleGradient(screenWidth/2, screenHeight/2, screenWidth * 0.8f, Fade(BLACK, 0.0f), Fade(BLACK, vignettePulse));
    
    // Consume all clicks on backdrop and modal to prevent click-through
    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Click consumed - prevents any background interaction
    }
    
    int mw = 650, mh = 500;
    Rectangle modal{(float)(screenWidth/2-mw/2), (float)(screenHeight/2-mh/2), (float)mw, (float)mh};
    
    // Ultra-enhanced multi-layer shadow with huge depth
    DrawRectangleRounded({modal.x + 5, modal.y + 10, modal.width, modal.height}, 0.03f, 8, Fade(SHADOW_COLOR, 0.8f));
    DrawRectangleRounded({modal.x + 3, modal.y + 6, modal.width, modal.height}, 0.03f, 8, Fade(SHADOW_COLOR, 0.5f));
    DrawRectangleRounded({modal.x + 1, modal.y + 3, modal.width, modal.height}, 0.03f, 8, Fade(SHADOW_COLOR, 0.3f));
    
    // Animated outer glow
    static float modalGlow = 0.0f;
    modalGlow += 0.05f;
    float glowIntensity = 0.3f + sin(modalGlow) * 0.1f;
    DrawRectangleRounded({modal.x - 5, modal.y - 5, modal.width + 10, modal.height + 10}, 0.03f, 8, Fade(BUTTON_BLUE, glowIntensity));
    
    // Modal background with premium gradient
    DrawRectangleGradientV(modal.x, modal.y, modal.width, modal.height, METAL_PANEL, ColorBrightness(METAL_PANEL, -0.1f));
    // Top shine with gradient
    Rectangle topShine = {modal.x, modal.y, modal.width, 120};
    DrawRectangleGradientV(topShine.x, topShine.y, topShine.width, topShine.height, 
                          Fade(METAL_HIGHLIGHT, 0.05f), Fade(METAL_HIGHLIGHT, 0.0f));
    // Premium border stack
    DrawRectangleLinesEx(modal, 3.0f, BUTTON_BLUE);
    DrawRectangleLinesEx(modal, 1.5f, Fade(BUTTON_BLUE_HOVER, glowIntensity));

    // Header
    DrawRectangle(modal.x, modal.y, modal.width, 50, METAL_ACCENT);
    char headerText[64];
    snprintf(headerText, sizeof(headerText), "[$] Carrinho (%d itens)", (int)cart.size());
    DrawTextCustom(headerText, modal.x+24, modal.y+15, 24, TEXT_WHITE);

    int y = (int)modal.y + 70;
    float total = 0.0f;
    
    if(cart.empty()){
        DrawTextCustom("O carrinho está vazio.", modal.x+24, y + 60, 20, METAL_BRONZE);
        DrawTextCustom("Adicione produtos para continuar.", modal.x+24, y + 90, 16, METAL_BRONZE);
    } else {
        // Scroll area for items
        for(size_t i=0; i<cart.size() && i<6; ++i){
            CartItem& item = cart[i];
            
            // Calculate actual price (with discount if applicable)
            float actualPrice = item.product.price;
            if(item.product.isOnDiscount && item.product.discountPercent > 0) {
                actualPrice = item.product.price * (1.0f - item.product.discountPercent / 100.0f);
            }
            
            // Item card
            Rectangle itemCard = {modal.x + 20, (float)y, modal.width - 40, 50};
            DrawRectangleRounded(itemCard, 0.1f, 8, i % 2 == 0 ? METAL_BG : Fade(METAL_BG, 0.5f));
            
            // Color indicator
            DrawRectangleRounded({itemCard.x + 5, itemCard.y + 5, 4, itemCard.height - 10}, 0.5f, 4, item.product.color);
            
            // Product name - MAIOR E MAIS VISÍVEL
            DrawTextWithShadow(item.product.name.c_str(), itemCard.x + 15, itemCard.y + 6, 17, TEXT_WHITE);
            
            // Discount badge if applicable
            if(item.product.isOnDiscount && item.product.discountPercent > 0) {
                char discountText[16];
                snprintf(discountText, sizeof(discountText), "-%0.f%%", item.product.discountPercent);
                Rectangle discountBadge = {itemCard.x + itemCard.width - 65, itemCard.y + 6, 50, 18};
                DrawRectangleRounded(discountBadge, 0.3f, 6, BUTTON_RED);
                int discW = MeasureTextCustom(discountText, 12);
                DrawTextCustom(discountText, discountBadge.x + (discountBadge.width - discW)/2, discountBadge.y + 3, 12, TEXT_WHITE);
            }
            
            // Quantity controls
            Rectangle btnMinus = {itemCard.x + 15, itemCard.y + 28, 30, 18};
            Rectangle btnPlus = {itemCard.x + 75, itemCard.y + 28, 30, 18};
            
            if(DrawButton("-", btnMinus, BUTTON_RED)) {
                if(item.qty > 1) item.qty--;
                else cart.erase(cart.begin() + i);
            }
            
            char qtyText[16];
            snprintf(qtyText, sizeof(qtyText), "%d", item.qty);
            int qtyW = MeasureTextCustom(qtyText, 17);
            DrawTextCustom(qtyText, itemCard.x + 45 + (30 - qtyW)/2, itemCard.y + 28, 17, TEXT_WHITE);
            
            if(DrawButton("+", btnPlus, SUCCESS_GREEN)) {
                item.qty++;
            }
            
            // Unit price - MAIS VISÍVEL (show discounted price or original)
            char unitPrice[32];
            if(item.product.isOnDiscount && item.product.discountPercent > 0) {
                snprintf(unitPrice, sizeof(unitPrice), "EUR %.2f/un", actualPrice);
            } else {
                snprintf(unitPrice, sizeof(unitPrice), "EUR %.2f/un", item.product.price);
            }
            DrawTextCustom(unitPrice, itemCard.x + 120, itemCard.y + 30, 14, TEXT_GRAY);
            
            // Total price for this item - MAIOR E COM SOMBRA (using discounted price)
            char priceBuf[32];
            snprintf(priceBuf, sizeof(priceBuf), "EUR %.2f", actualPrice * item.qty);
            int priceW = MeasureTextCustom(priceBuf, 20);
            Color priceColor = (item.product.isOnDiscount && item.product.discountPercent > 0) ? BUTTON_RED : GOLD;
            DrawTextWithShadow(priceBuf, itemCard.x + itemCard.width - priceW - 12, itemCard.y + 14, 20, priceColor);
            
            y += 58;
            total += actualPrice * item.qty;
        }
    }

    // Total section with highlight
    Rectangle totalBox = {modal.x + 20, modal.y + mh - 130, modal.width - 40, 40};
    DrawRectangleRounded(totalBox, 0.15f, 8, METAL_ACCENT);
    char totalBuf[64];
    int totalItems = 0;
    for(const auto& item : cart) totalItems += item.qty;
    snprintf(totalBuf, sizeof(totalBuf), "Total (%d produtos): EUR %.2f", totalItems, total);
    DrawTextCustom(totalBuf, totalBox.x + 20, totalBox.y + 11, 20, SUCCESS_GREEN);

    // Buttons with spacing
    Rectangle btnFinalizar{modal.x+24, modal.y+mh-70, 160, 40};
    Rectangle btnLimpar{modal.x+194, modal.y+mh-70, 140, 40};
    Rectangle btnContinuar{modal.x+modal.width-174, modal.y+mh-70, 150, 40};

    // Show button always, but disable if cart is empty
    bool canFinalize = !cart.empty();
    if(DrawButton("Finalizar Compra", btnFinalizar, canFinalize ? SUCCESS_GREEN : DARKGRAY)) {
        if(canFinalize) {
            if(!isLoggedIn) {
                showLoginPrompt = true;
                cartMessage = "Faca login para finalizar a compra!";
            } else {
                showCheckout = true;
                showModal = false;
            }
        }
    }
    
    // Show clear button always, but disable if cart is empty
    if(DrawButton("Limpar", btnLimpar, !cart.empty() ? BUTTON_RED : DARKGRAY)) {
        if(!cart.empty()) {
            cart.clear();
        }
    }
    
    if(DrawButton("Continuar", btnContinuar, METAL_ACCENT)) {
        showModal = false;
    }

    // Success message
    if(!cartMessage.empty()){
        Rectangle msgBox = {modal.x + 24, modal.y + mh - 115, modal.width - 48, 30};
        DrawRectangleRounded(msgBox, 0.2f, 8, Fade(SUCCESS_GREEN, 0.2f));
        DrawTextCustom(cartMessage.c_str(), msgBox.x + 12, msgBox.y + 7, 16, SUCCESS_GREEN);
    }
}

// Helper function to draw input field
void DrawInputField(Rectangle box, const std::string& label, const std::string& value, bool active, int maxChars) {
    DrawRectangleRounded(box, 0.15f, 6, active ? METAL_ACCENT : METAL_BG);
    Color borderColor = active ? BUTTON_BLUE : METAL_ACCENT;
    DrawRectangleLinesEx(box, 2.0f, borderColor);
    DrawTextCustom(label.c_str(), box.x, box.y - 26, 17, TEXT_GRAY);  // Label maior e mais claro
    
    // Draw text with scissor
    BeginScissorMode(box.x + 4, box.y, box.width - 8, box.height);
    DrawTextCustom(value.c_str(), box.x + 12, box.y + 9, 18, TEXT_WHITE);  // Texto maior
    
    // Cursor
    if(active && ((GetTime() * 2) - (int)(GetTime() * 2) < 0.5f)) {
        int textW = MeasureTextCustom(value.c_str(), 18);
        DrawRectangle(box.x + 12 + textW + 2, box.y + 9, 2, 20, BUTTON_BLUE);
    }
    EndScissorMode();
}

// Save products to file
void SaveProducts(const std::vector<Product>& products) {
    std::ofstream file("products.txt");
    if(file.is_open()) {
        for(const auto& p : products) {
            file << p.id << "|" << p.name << "|" << p.desc << "|" 
                 << p.price << "|" << (int)p.color.r << "," << (int)p.color.g 
                 << "," << (int)p.color.b << "|" << CategoryToString(p.category) 
                 << "|" << p.imagePath << "|" << (p.inStock ? "1" : "0") 
                 << "|" << (p.isOnDiscount ? "1" : "0") << "|" << p.discountPercent 
                 << "|" << p.rating << "\n";
        }
        file.close();
    }
}

// Load products from file
std::vector<Product> LoadProducts() {
    std::vector<Product> products;
    std::ifstream file("products.txt");
    if(file.is_open()) {
        std::string line;
        while(std::getline(file, line)) {
            // Skip empty lines
            if(line.empty()) continue;
            
            try {
                Product p;
                size_t pos = 0;
                
                // Parse ID
                size_t nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue; // Invalid line
                std::string idStr = line.substr(pos, nextPos - pos);
                if(idStr.empty()) continue; // Empty ID
                p.id = std::stoi(idStr);
                pos = nextPos + 1;
            
                // Parse name
                nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue; // Invalid line
                p.name = line.substr(pos, nextPos - pos);
                pos = nextPos + 1;
                
                // Parse desc
                nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue; // Invalid line
                p.desc = line.substr(pos, nextPos - pos);
                pos = nextPos + 1;
                
                // Parse price
                nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue; // Invalid line
                std::string priceStr = line.substr(pos, nextPos - pos);
                if(priceStr.empty()) continue; // Empty price
                p.price = std::stof(priceStr);
                pos = nextPos + 1;
            
            // Parse color
            nextPos = line.find('|', pos);
            int r, g, b;
            sscanf(line.substr(pos, nextPos - pos).c_str(), "%d,%d,%d", &r, &g, &b);
            p.color = {(unsigned char)r, (unsigned char)g, (unsigned char)b, 255};
            pos = nextPos + 1;
            
            // Parse category
            nextPos = line.find('|', pos);
            std::string catStr = line.substr(pos, nextPos - pos);
            p.category = StringToCategory(catStr);
            pos = nextPos + 1;
            
            // Parse imagePath (optional - might not exist in old files)
            if(pos < line.length()) {
                nextPos = line.find('|', pos);
                if(nextPos != std::string::npos) {
                    p.imagePath = line.substr(pos, nextPos - pos);
                    pos = nextPos + 1;
                    
                    // Parse inStock (optional)
                    nextPos = line.find('|', pos);
                    if(nextPos != std::string::npos) {
                        p.inStock = (line.substr(pos, nextPos - pos) == "1");
                        pos = nextPos + 1;
                        
                        // Parse isOnDiscount (optional)
                        nextPos = line.find('|', pos);
                        if(nextPos != std::string::npos) {
                            p.isOnDiscount = (line.substr(pos, nextPos - pos) == "1");
                            pos = nextPos + 1;
                            
                            // Parse discountPercent (optional)
                            nextPos = line.find('|', pos);
                            if(nextPos != std::string::npos) {
                                p.discountPercent = std::stof(line.substr(pos, nextPos - pos));
                                pos = nextPos + 1;
                                
                                // Parse rating (optional)
                                if(pos < line.length()) {
                                    p.rating = std::stof(line.substr(pos));
                                } else {
                                    p.rating = 4.0f; // Default rating
                                }
                            } else if(pos < line.length()) {
                                p.discountPercent = std::stof(line.substr(pos));
                                p.rating = 4.0f; // Default rating
                            } else {
                                p.discountPercent = 0.0f;
                                p.rating = 4.0f; // Default rating
                            }
                        } else {
                            p.isOnDiscount = false;
                            p.discountPercent = 0.0f;
                            p.rating = 4.0f; // Default rating
                        }
                    } else {
                        p.inStock = true;
                        p.isOnDiscount = false;
                        p.discountPercent = 0.0f;
                        p.rating = 4.0f; // Default rating
                    }
                } else {
                    p.imagePath = line.substr(pos);
                    p.inStock = true;
                    p.isOnDiscount = false;
                    p.discountPercent = 0.0f;
                    p.rating = 4.0f; // Default rating
                }
                } else {
                    p.imagePath = "none";
                    p.inStock = true;
                    p.isOnDiscount = false;
                    p.discountPercent = 0.0f;
                    p.rating = 4.0f; // Default rating
                }
                
                products.push_back(p);
            } catch(const std::exception& e) {
                // Skip malformed lines (invalid number format, etc.)
                continue;
            }
        }
        file.close();
    }
    return products;
}

// Admin Panel for CRUD operations
void ShowAdminPanel(int screenWidth, int screenHeight, std::vector<Product>& products, bool& showAdmin) {
    static AdminAction action = AdminAction::NONE;
    static int selectedProductIdx = -1;
    static std::string editName, editDesc, editPrice, editImagePath;
    static int activeField = 0; // 0=name, 1=desc, 2=price, 3=search, 4=imagePath
    static std::string message;
    static int selectedColorIdx = 0;
    static ProductCategory selectedCategory = ProductCategory::CPU;
    static float adminScrollOffset = 0.0f;
    static std::string adminSearch = ""; // Search in admin panel
    static ProductCategory adminFilterCategory = ProductCategory::CPU;
    static bool showAllCategories = true; // Filter toggle
    static Color colors[] = {ORANGE, BLUE, DARKBLUE, LIME, RED, PURPLE, GOLD, GREEN, BROWN, MAROON, PINK, SKYBLUE, DARKGRAY, GREEN};
    static const char* categoryNames[] = {"CPU", "GPU", "RAM", "Storage", "Motherboard", "PSU", "Cooling", "Case", "Peripheral"};
    
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.8f));
    
    // Consume all clicks to prevent click-through
    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Click consumed - prevents any background interaction
    }
    
    int mw = 800, mh = 650;
    Rectangle modal{(float)(screenWidth/2-mw/2), (float)(screenHeight/2-mh/2), (float)mw, (float)mh};
    
    DrawRectangle(modal.x + 4, modal.y + 4, modal.width, modal.height, Fade(SHADOW_COLOR, 0.5f));
    DrawRectangleRounded(modal, 0.02f, 8, METAL_PANEL);
    DrawRectangleLinesEx(modal, 2.0f, METAL_ACCENT);
    
    // Header
    DrawRectangle(modal.x, modal.y, modal.width, 50, METAL_ACCENT);
    DrawTextWithShadow("[*] Painel de Administracao - CRUD Produtos", modal.x+20, modal.y+12, 28, TEXT_WHITE);
    
    // Action buttons - Top row
    Rectangle btnCreate = {modal.x + 20, modal.y + 60, 120, 36};
    Rectangle btnClose = {modal.x + modal.width - 140, modal.y + 60, 120, 36};
    
    // Export buttons (Extras) - Second row
    Rectangle btnExportCSV = {modal.x + 20, modal.y + 105, 110, 36};
    Rectangle btnReport = {modal.x + 140, modal.y + 105, 110, 36};
    Rectangle btnBackup = {modal.x + 260, modal.y + 105, 110, 36};
    
    if(DrawButton("+ Criar Novo", btnCreate, SUCCESS_GREEN)) {
        action = AdminAction::CREATE;
        selectedProductIdx = -1;
        editName = "";
        editDesc = "";
        editPrice = "";
        editImagePath = "";
        selectedColorIdx = 0;
        selectedCategory = ProductCategory::CPU;
        message = "";
    }
    
    if(DrawButton("Fechar", btnClose, BUTTON_RED)) {
        showAdmin = false;
        action = AdminAction::NONE;
        adminSearch = "";
        showAllCategories = true;
        SaveProducts(products);
    }
    
    // Extras buttons (only in list view)
    if(action == AdminAction::NONE) {
        if(DrawButton("[$] CSV", btnExportCSV, METAL_ACCENT)) {
            ExportToCSV(products);
            message = "[OK] CSV exportado com sucesso!";
        }
        
        if(DrawButton("[R] Relatorio", btnReport, METAL_ACCENT)) {
            GenerateReport(products);
            message = "[OK] Relatorio gerado com sucesso!";
        }
        
        if(DrawButton("[B] Backup", btnBackup, METAL_ACCENT)) {
            CreateBackup(products);
            message = "[OK] Backup criado com sucesso!";
        }
    }
    
    // Search and filter area (only in list view)
    if(action == AdminAction::NONE) {
        // Search bar
        Rectangle searchBox = {modal.x + 380, modal.y + 105, 300, 36};
        bool searchActive = (activeField == 3);
        DrawRectangleRounded(searchBox, 0.15f, 6, searchActive ? METAL_ACCENT : METAL_BG);
        DrawRectangleLinesEx(searchBox, 2.0f, searchActive ? BUTTON_BLUE : METAL_ACCENT);
        
        const char* searchPlaceholder = "[?] Buscar produtos...";
        if(adminSearch.empty() && !searchActive) {
            DrawTextCustom(searchPlaceholder, searchBox.x + 12, searchBox.y + 10, 14, METAL_BRONZE);
        } else {
            DrawTextCustom(adminSearch.c_str(), searchBox.x + 12, searchBox.y + 10, 14, TEXT_WHITE);
        }
        
        if(CheckCollisionPointRec(GetMousePosition(), searchBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            activeField = 3;
        }
        
        // Category filter chips - moved to second row below search
        float chipX = modal.x + 20;
        float chipY = modal.y + 150;
        
        // "All" button
        Rectangle chipAll = {chipX, chipY, 70, 36};
        if(DrawButton("Todos", chipAll, showAllCategories ? BUTTON_BLUE : METAL_ACCENT)) {
            showAllCategories = true;
            adminScrollOffset = 0;
        }
        
        chipX += 80;
        
        // First row of category buttons
        for(int cat = 0; cat < 5; cat++) {
            Rectangle chip = {chipX, chipY, 85, 36};
            bool isActive = !showAllCategories && (int)adminFilterCategory == cat;
            
            if(DrawButton(categoryNames[cat], chip, isActive ? BUTTON_BLUE : METAL_ACCENT)) {
                showAllCategories = false;
                adminFilterCategory = (ProductCategory)cat;
                adminScrollOffset = 0;
            }
            chipX += 95;
        }
        
        // Second row of category buttons
        chipX = modal.x + 20;
        chipY += 46;
        for(int cat = 5; cat < 9; cat++) {
            Rectangle chip = {chipX, chipY, 85, 36};
            bool isActive = !showAllCategories && (int)adminFilterCategory == cat;
            
            if(DrawButton(categoryNames[cat], chip, isActive ? BUTTON_BLUE : METAL_ACCENT)) {
                showAllCategories = false;
                adminFilterCategory = (ProductCategory)cat;
                adminScrollOffset = 0;
            }
            chipX += 95;
        }
    }
    
    // Product list area - adjusted Y position to accommodate two rows of filters
    int listY = modal.y + 246;
    int itemHeight = 60;
    
    if(action == AdminAction::NONE) {
        // Filter products based on search and category
        std::vector<int> filteredIndices;
        for(size_t i = 0; i < products.size(); ++i) {
            bool matches = true;
            
            // Filter by category
            if(!showAllCategories && products[i].category != adminFilterCategory) {
                matches = false;
            }
            
            // Filter by search
            if(!adminSearch.empty()) {
                std::string searchLower = adminSearch;
                std::string nameLower = products[i].name;
                std::string descLower = products[i].desc;
                std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                std::transform(descLower.begin(), descLower.end(), descLower.begin(), ::tolower);
                
                if(nameLower.find(searchLower) == std::string::npos && 
                   descLower.find(searchLower) == std::string::npos) {
                    matches = false;
                }
            }
            
            if(matches) filteredIndices.push_back(i);
        }
        
        // Show count
        char countText[64];
        snprintf(countText, sizeof(countText), "Mostrando %d de %d produtos", (int)filteredIndices.size(), (int)products.size());
        DrawTextCustom(countText, modal.x + 20, listY - 25, 16, METAL_HIGHLIGHT);
        
        // Calculate max scroll for product list (reduced height to fit new layout)
        float listHeight = 270;
        float contentHeight = filteredIndices.size() * itemHeight;
        float maxAdminScroll = contentHeight > listHeight ? contentHeight - listHeight : 0;
        
        // Handle scroll wheel for product list
        Vector2 mp = GetMousePosition();
        Rectangle listArea = {modal.x + 20, (float)listY, modal.width - 40, listHeight};
        if(CheckCollisionPointRec(mp, listArea)) {
            float wheelMove = GetMouseWheelMove();
            if(wheelMove != 0) {
                adminScrollOffset -= wheelMove * 30.0f;
                if(adminScrollOffset < 0) adminScrollOffset = 0;
                if(adminScrollOffset > maxAdminScroll) adminScrollOffset = maxAdminScroll;
            }
        }
        
        // Handle search input
        if(activeField == 3) {
            int key = GetCharPressed();
            while(key > 0) {
                if(key >= 32 && key <= 126 && adminSearch.length() < 50) {
                    adminSearch += (char)key;
                }
                key = GetCharPressed();
            }
            if(IsKeyPressed(KEY_BACKSPACE) && !adminSearch.empty()) {
                adminSearch.pop_back();
            }
            if(IsKeyPressed(KEY_ESCAPE)) {
                activeField = 0;
            }
        }
        
        BeginScissorMode(modal.x + 20, listY, modal.width - 40, 270);
        for(size_t k = 0; k < filteredIndices.size(); ++k) {
            int i = filteredIndices[k];
            Rectangle itemBox = {modal.x + 20, (float)(listY + k * itemHeight - adminScrollOffset), modal.width - 40, 55};
            
            // Only render if visible
            if(itemBox.y + itemBox.height < listY || itemBox.y > listY + listHeight) continue;
            
            bool hovered = CheckCollisionPointRec(GetMousePosition(), itemBox);
            DrawRectangleRounded(itemBox, 0.1f, 6, hovered ? CARD_HOVER : METAL_BG);
            
            // Color indicator
            DrawRectangleRounded({itemBox.x + 5, itemBox.y + 5, 6, itemBox.height - 10}, 0.5f, 4, products[i].color);
            
            // Product info with category badge
            DrawTextCustom(products[i].name.c_str(), itemBox.x + 20, itemBox.y + 8, 15, METAL_HIGHLIGHT);
            
            // Category badge
            const char* catName = CategoryToString(products[i].category).c_str();
            int catW = MeasureTextCustom(catName, 11);
            Rectangle catBadge = {itemBox.x + 20, itemBox.y + 28, (float)catW + 16, 18};
            DrawRectangleRounded(catBadge, 0.3f, 4, Fade(products[i].color, 0.3f));
            DrawTextCustom(catName, catBadge.x + 8, catBadge.y + 3, 11, products[i].color);
            
            // Price
            char priceText[32];
            snprintf(priceText, sizeof(priceText), "EUR %.2f", products[i].price);
            DrawTextCustom(priceText, itemBox.x + catBadge.width + 35, itemBox.y + 30, 13, GOLD);
            
            // Edit button
            Rectangle btnEdit = {itemBox.x + itemBox.width - 180, itemBox.y + 12, 80, 30};
            if(DrawButton("[E] Editar", btnEdit, BUTTON_BLUE)) {
                action = AdminAction::EDIT;
                selectedProductIdx = i;
                editName = products[i].name;
                editDesc = products[i].desc;
                editPrice = std::to_string((int)(products[i].price * 100) / 100.0f);
                editImagePath = products[i].imagePath;
                selectedCategory = products[i].category;
                for(int c = 0; c < 14; c++) {
                    if(products[i].color.r == colors[c].r && products[i].color.g == colors[c].g && products[i].color.b == colors[c].b) {
                        selectedColorIdx = c;
                        break;
                    }
                }
                message = "";
            }
            
            // Delete button
            Rectangle btnDelete = {itemBox.x + itemBox.width - 90, itemBox.y + 12, 80, 30};
            if(DrawButton("[X] Excluir", btnDelete, BUTTON_RED)) {
                std::string deletedName = products[i].name;
                products.erase(products.begin() + i);
                message = "[OK] Produto excluido!";
                LogAction("DELETE", "Produto excluido: " + deletedName);
                SaveProducts(products);
            }
        }
        EndScissorMode();
        
        // Draw scrollbar for product list if needed
        if(maxAdminScroll > 0) {
            float scrollbarX = modal.x + modal.width - 30;
            float scrollbarY = listY;
            float scrollbarHeight = listHeight;
            float scrollbarWidth = 8;
            
            DrawRectangleRounded({scrollbarX, scrollbarY, scrollbarWidth, scrollbarHeight}, 0.5f, 4, METAL_ACCENT);
            
            float thumbHeight = (listHeight / contentHeight) * scrollbarHeight;
            if(thumbHeight < 30) thumbHeight = 30;
            float thumbY = scrollbarY + (adminScrollOffset / maxAdminScroll) * (scrollbarHeight - thumbHeight);
            DrawRectangleRounded({scrollbarX, thumbY, scrollbarWidth, thumbHeight}, 0.5f, 4, BUTTON_BLUE);
        }
    }
    
    // Create/Edit form
    if(action == AdminAction::CREATE || action == AdminAction::EDIT) {
        const char* title = action == AdminAction::CREATE ? "Criar Novo Produto" : "Editar Produto";
        DrawTextCustom(title, modal.x + 20, listY - 25, 18, METAL_HIGHLIGHT);
        
        Rectangle nameBox = {modal.x + 40, (float)listY + 20, 720, 38};
        Rectangle descBox = {modal.x + 40, (float)listY + 80, 720, 38};
        Rectangle priceBox = {modal.x + 40, (float)listY + 140, 200, 38};
        Rectangle imageBox = {modal.x + 260, (float)listY + 140, 500, 38};
        
        DrawInputField(nameBox, "Nome do Produto", editName, activeField == 0, 60);
        DrawInputField(descBox, "Descricao", editDesc, activeField == 1, 120);
        DrawInputField(priceBox, "Preco (EUR)", editPrice, activeField == 2, 10);
        DrawInputField(imageBox, "Imagem (thumbnails/nome.png)", editImagePath, activeField == 4, 100);
        
        // Category selector - Professional dropdown style
        DrawTextCustom("Categoria:", modal.x + 40, listY + 200, 14, METAL_BRONZE);
        int catY = listY + 220;
        for(int cat = 0; cat < 9; cat++) {
            int row = cat / 5;
            int col = cat % 5;
            Rectangle catBox = {modal.x + 40 + col * 145.0f, (float)(catY + row * 45), 140, 38};
            
            bool isSelected = (int)selectedCategory == cat;
            bool isHovered = CheckCollisionPointRec(GetMousePosition(), catBox);
            
            Color catBg = isSelected ? BUTTON_BLUE : (isHovered ? METAL_ACCENT : METAL_BG);
            DrawRectangleRounded(catBox, 0.2f, 6, catBg);
            DrawRectangleLinesEx(catBox, 2.0f, isSelected ? TEXT_WHITE : METAL_ACCENT);
            
            int txtW = MeasureTextCustom(categoryNames[cat], 14);
            DrawTextCustom(categoryNames[cat], catBox.x + (catBox.width - txtW)/2, catBox.y + 12, 14, isSelected ? TEXT_WHITE : METAL_HIGHLIGHT);
            
            if(CheckCollisionPointRec(GetMousePosition(), catBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                selectedCategory = (ProductCategory)cat;
            }
        }
        
        // Color selector
        DrawTextCustom("Cor:", modal.x + 40, listY + 320, 14, METAL_BRONZE);
        for(int c = 0; c < 14; c++) {
            int row = c / 7;
            int col = c % 7;
            Rectangle colorBox = {modal.x + 40 + col * 48.0f, (float)(listY + 340 + row * 48), 44, 44};
            DrawRectangleRounded(colorBox, 0.3f, 6, colors[c]);
            if(c == selectedColorIdx) {
                DrawRectangleLinesEx({colorBox.x - 3, colorBox.y - 3, colorBox.width + 6, colorBox.height + 6}, 3.0f, TEXT_WHITE);
            }
            if(CheckCollisionPointRec(GetMousePosition(), colorBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                selectedColorIdx = c;
            }
        }
        
        // Handle input
        Vector2 mp = GetMousePosition();
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if(CheckCollisionPointRec(mp, nameBox)) activeField = 0;
            else if(CheckCollisionPointRec(mp, descBox)) activeField = 1;
            else if(CheckCollisionPointRec(mp, priceBox)) activeField = 2;
            else if(CheckCollisionPointRec(mp, imageBox)) activeField = 4;
        }
        
        if(IsKeyPressed(KEY_TAB)) {
            activeField = (activeField + 1) % 5;
            if(activeField == 3) activeField = 4; // Skip search field
        }
        
        int key = GetCharPressed();
        while(key > 0) {
            if(activeField == 0 && editName.length() < 60 && key >= 32 && key <= 126) editName += (char)key;
            else if(activeField == 1 && editDesc.length() < 100 && key >= 32 && key <= 126) editDesc += (char)key;
            else if(activeField == 2 && editPrice.length() < 10 && ((key >= '0' && key <= '9') || key == '.')) editPrice += (char)key;
            else if(activeField == 4 && editImagePath.length() < 100 && key >= 32 && key <= 126) editImagePath += (char)key;
            key = GetCharPressed();
        }
        
        if(IsKeyPressed(KEY_BACKSPACE)) {
            if(activeField == 0 && !editName.empty()) editName.pop_back();
            else if(activeField == 1 && !editDesc.empty()) editDesc.pop_back();
            else if(activeField == 2 && !editPrice.empty()) editPrice.pop_back();
            else if(activeField == 4 && !editImagePath.empty()) editImagePath.pop_back();
        }
        
        // Save and Cancel buttons
        Rectangle btnSave = {modal.x + 40, (float)listY + 450, 120, 36};
        Rectangle btnCancel = {modal.x + 170, (float)listY + 450, 120, 36};
        
        if(DrawButton("[S] Salvar", btnSave, SUCCESS_GREEN)) {
            if(editName.empty() || editDesc.empty() || editPrice.empty()) {
                message = "[ERRO] Preencha todos os campos!";
            } else {
                float price = std::stof(editPrice);
                std::string imgPath = editImagePath.empty() ? "none" : editImagePath;
                if(action == AdminAction::CREATE) {
                    int newId = products.empty() ? 1 : products.back().id + 1;
                    products.push_back({newId, editName, editDesc, price, colors[selectedColorIdx], selectedCategory, imgPath, true, false, 0.0f, 4.0f});
                    message = "[OK] Produto criado!";
                    LogAction("CREATE", "Produto criado: " + editName + " (EUR " + editPrice + ")");
                } else {
                    products[selectedProductIdx].name = editName;
                    products[selectedProductIdx].desc = editDesc;
                    products[selectedProductIdx].price = price;
                    products[selectedProductIdx].color = colors[selectedColorIdx];
                    products[selectedProductIdx].category = selectedCategory;
                    products[selectedProductIdx].imagePath = imgPath;
                    message = "[OK] Produto atualizado!";
                    LogAction("UPDATE", "Produto atualizado: " + editName + " (EUR " + editPrice + ")");
                }
                SaveProducts(products);
                action = AdminAction::NONE;
            }
        }
        
        if(DrawButton("Cancelar", btnCancel, METAL_ACCENT)) {
            action = AdminAction::NONE;
            message = "";
        }
    }
    
    // Message
    if(!message.empty()) {
        bool isSuccess = message.find("[OK]") != std::string::npos;
        Color msgColor = isSuccess ? SUCCESS_GREEN : BUTTON_RED;
        Rectangle msgBox = {modal.x + 20, modal.y + mh - 50, modal.width - 40, 30};
        DrawRectangleRounded(msgBox, 0.2f, 6, Fade(msgColor, 0.15f));
        DrawTextCustom(message.c_str(), msgBox.x + 12, msgBox.y + 7, 15, msgColor);
    }
}

void RunTechcoreUI(int screenWidth, int screenHeight, bool (*LoginFunc)(int, int)) {
    // Initialize particle system
    if(!particlesInitialized) {
        InitParticles(screenWidth, screenHeight);
    }
    
    // Load high-quality font for crisp text rendering
    if(!fontLoaded) {
        // Try loading Segoe UI font (Windows), high resolution for crisp rendering
        customFont = LoadFontEx("C:/Windows/Fonts/segoeui.ttf", 128, 0, 250);
        if(customFont.texture.id > 0) {
            // Enable bilinear filtering for smooth scaling
            SetTextureFilter(customFont.texture, TEXTURE_FILTER_BILINEAR);
            GenTextureMipmaps(&customFont.texture);
            fontLoaded = true;
        } else {
            // Fallback: load default font at high resolution
            customFont = GetFontDefault();
            SetTextureFilter(customFont.texture, TEXTURE_FILTER_BILINEAR);
            fontLoaded = true;
        }
    }
    
    std::vector<Product> products = LoadProducts();
    std::vector<CartItem> cart;
    std::vector<User> users = LoadUsers();
    std::vector<Order> orders = LoadOrders();
    std::vector<Review> reviews = LoadReviews();
    
    static bool isLoggedIn = false;
    User* currentUser = nullptr;
    std::string currentUsername = "";
    bool showLoginPrompt = false;
    bool showUserProfile = false;
    bool showWishlist = false;
    bool showOrderHistory = false;
    bool showCheckout = false;
    bool showAdmin = false;

    std::vector<std::string> categories = {"All","CPU","GPU","RAM","Storage","Motherboard","PSU","Cooling","Case","Peripheral"};
    std::string selectedCategory = "All";
    std::string search;
    bool isSearchActive = false;
    bool showCart = false;
    bool showProductDetails = false;
    int selectedProductForDetails = -1;
    std::string cartMessage = "";
    std::string toastMessage = "";
    float toastTimer = 0.0f;
    bool showCategoryFilter = false; // Toggle for category dropdown
    
    // Filter sidebar state
    bool filterStockExpanded = true;
    bool filterManufacturerExpanded = true;
    bool filterPriceExpanded = true;
    bool filterStateExpanded = true;
    bool filterBrandExpanded = false;
    bool filterRatingExpanded = false;
    bool filterShippingExpanded = false;
    
    // Filter selections
    bool filterInStock = false;
    bool filterOutOfStock = false;
    bool filterAMD = false;
    bool filterIntel = false;
    bool filterNvidia = false;
    float filterMinPrice = 0.0f;
    float filterMaxPrice = 15000.0f;
    bool filterNew = false;
    bool filterPromotion = false;
    bool filterHighlight = false;
    // Brand filters
    bool filterAsus = false;
    bool filterMSI = false;
    bool filterGigabyte = false;
    bool filterCorsair = false;
    // Rating filters
    bool filter5Stars = false;
    bool filter4Plus = false;
    bool filter3Plus = false;
    // Shipping filters
    bool filterFreeShipping = false;
    bool filterExpressShipping = false;
    
    int cols = 2;
    float gutter = 18.f;
    float filterSidebarWidth = 350.0f; // Width reserved for filter sidebar
    float cardW = (screenWidth - filterSidebarWidth - 80 - (cols-1)*gutter)/cols;
    float cardH = 112.f;
    
    // Scroll variables
    float scrollOffset = 0.0f;
    float scrollSpeed = 30.0f;
    float maxScroll = 0.0f;
    bool isDraggingScrollbar = false;
    float dragScrollStartY = 0.0f;
    float dragScrollStartOffset = 0.0f;
    
    // Pagination
    int currentPage = 0;
    int productsPerPage = 20;
    
    // Sorting
    enum SortMode { SORT_NONE, SORT_PRICE_ASC, SORT_PRICE_DESC, SORT_NAME };
    SortMode sortMode = SORT_NONE;

    while (!WindowShouldClose()) {
        // Update particles
        UpdateParticles(screenWidth, screenHeight);
        
        BeginDrawing();
        ClearBackground(METAL_BG);
        
        // Draw particles as background layer
        DrawParticles();
        
        // Update toast timer
        if(toastTimer > 0) {
            toastTimer -= GetFrameTime();
            if(toastTimer < 0) toastTimer = 0;
        }
        
        // Keyboard shortcuts
        if(IsKeyPressed(KEY_ESCAPE)) {
            if(showProductDetails) showProductDetails = false;
            else if(showCart) showCart = false;
            else if(showAdmin) showAdmin = false;
            else if(isSearchActive) isSearchActive = false;
            else if(showCategoryFilter) showCategoryFilter = false;
        }
        if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_F)) {
            isSearchActive = true;
        }
        
        // Close category filter when clicking outside
        if(showCategoryFilter && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Rectangle dropdownArea = {850, 110, 500, 175};
            if(!CheckCollisionPointRec(GetMousePosition(), dropdownArea)) {
                showCategoryFilter = false;
            }
        }

        // Detecta se o cursor está sobre os botões do header
        Vector2 mp = GetMousePosition();
        
        float btnW = 140, btnH = 44;
        float btnX = screenWidth - btnW - 24, btnY = 18;
        Rectangle cartBtn = {btnX, btnY, btnW, btnH};
        bool highlightCart = CheckCollisionPointRec(mp, cartBtn);
        
        float adminBtnW = 140, adminBtnH = 44;
        float adminBtnX = screenWidth - adminBtnW - 340, adminBtnY = 18;
        Rectangle adminBtn = {adminBtnX, adminBtnY, adminBtnW, adminBtnH};
        bool highlightAdmin = isLoggedIn && CheckCollisionPointRec(mp, adminBtn);
        
        // User button
        float userBtnW = 150, userBtnH = 44;
        float userBtnX = screenWidth - userBtnW - 180, userBtnY = 18;
        Rectangle userBtn = {userBtnX, userBtnY, userBtnW, userBtnH};
        bool highlightUser = CheckCollisionPointRec(mp, userBtn);

        DrawHeader(screenWidth, (int)cart.size(), highlightCart, highlightAdmin, isLoggedIn, highlightUser, currentUsername);

        // Abre modal do carrinho
        if(CheckCollisionPointRec(mp, cartBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            showCart = true;
                cartMessage = "";
        }
        
        // Abre painel admin
        if(isLoggedIn && CheckCollisionPointRec(mp, adminBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            showAdmin = true;
        }
        
        // User button - show login modal or user profile
        if(CheckCollisionPointRec(mp, userBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if(isLoggedIn) {
                showUserProfile = !showUserProfile;
            } else {
                showLoginPrompt = true;
            }
        }

        // ========== FILTER SIDEBAR WITH SCROLL ==========
        float filterX = 10;
        float filterY = 90;
        float filterWidth = 320;
        static float filterScrollOffset = 0.0f;
        static bool isDraggingFilterScrollbar = false;
        static float dragFilterScrollStartY = 0.0f;
        static float dragFilterScrollStartOffset = 0.0f;
        
        // Filter scroll area
        Rectangle filterScrollArea = {0, filterY, filterWidth + 20, (float)screenHeight - filterY};
        BeginScissorMode(filterScrollArea.x, filterScrollArea.y, filterScrollArea.width, filterScrollArea.height);
        
        float filterItemY = filterY - filterScrollOffset;
        
        // Title
        DrawTextCustom("FILTROS", filterX, filterItemY, 20, TEXT_WHITE);
        filterItemY += 40;
        
        // Stock Filter
        Rectangle stockHeader = {filterX, filterItemY, filterWidth, 40};
        bool hoverStock = CheckCollisionPointRec(GetMousePosition(), stockHeader);
        DrawRectangleRounded(stockHeader, 0.1f, 4, hoverStock ? METAL_ACCENT : METAL_PANEL);
        DrawTextCustom("Stock", filterX + 10, filterItemY + 12, 18, TEXT_WHITE);
        DrawTextCustom(filterStockExpanded ? "^" : "v", filterX + filterWidth - 25, filterItemY + 12, 18, METAL_HIGHLIGHT);
        if(CheckCollisionPointRec(GetMousePosition(), stockHeader) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            filterStockExpanded = !filterStockExpanded;
        }
        filterItemY += 45;
        
        if(filterStockExpanded) {
            Rectangle inStockBox = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(inStockBox, 2, METAL_ACCENT);
            if(filterInStock) DrawRectangle(inStockBox.x + 4, inStockBox.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("Em Stock", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filterInStock = !filterInStock;
            }
            filterItemY += 30;
            
            Rectangle outStockBox = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(outStockBox, 2, METAL_ACCENT);
            if(filterOutOfStock) DrawRectangle(outStockBox.x + 4, outStockBox.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("Sem Stock", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filterOutOfStock = !filterOutOfStock;
            }
            filterItemY += 35;
        }
        
        // Manufacturer Filter
        DrawRectangle(filterX, filterItemY, filterWidth, 1, METAL_ACCENT);
        filterItemY += 10;
        Rectangle manuHeader = {filterX, filterItemY, filterWidth, 40};
        bool hoverManu = CheckCollisionPointRec(GetMousePosition(), manuHeader);
        DrawRectangleRounded(manuHeader, 0.1f, 4, hoverManu ? METAL_ACCENT : METAL_PANEL);
        DrawTextCustom("Fabricante", filterX + 10, filterItemY + 12, 18, TEXT_WHITE);
        DrawTextCustom(filterManufacturerExpanded ? "^" : "v", filterX + filterWidth - 25, filterItemY + 12, 18, METAL_HIGHLIGHT);
        if(CheckCollisionPointRec(GetMousePosition(), manuHeader) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            filterManufacturerExpanded = !filterManufacturerExpanded;
        }
        filterItemY += 45;
        
        if(filterManufacturerExpanded) {
            Rectangle amdBox = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(amdBox, 2, METAL_ACCENT);
            if(filterAMD) DrawRectangle(amdBox.x + 4, amdBox.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("AMD", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filterAMD = !filterAMD;
            }
            filterItemY += 30;
            
            Rectangle intelBox = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(intelBox, 2, METAL_ACCENT);
            if(filterIntel) DrawRectangle(intelBox.x + 4, intelBox.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("Intel", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filterIntel = !filterIntel;
            }
            filterItemY += 30;
            
            Rectangle nvidiaBox = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(nvidiaBox, 2, METAL_ACCENT);
            if(filterNvidia) DrawRectangle(nvidiaBox.x + 4, nvidiaBox.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("NVIDIA", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filterNvidia = !filterNvidia;
            }
            filterItemY += 35;
        }
        
        // Price Filter
        DrawRectangle(filterX, filterItemY, filterWidth, 1, METAL_ACCENT);
        filterItemY += 10;
        Rectangle priceHeader = {filterX, filterItemY, filterWidth, 40};
        bool hoverPrice = CheckCollisionPointRec(GetMousePosition(), priceHeader);
        DrawRectangleRounded(priceHeader, 0.1f, 4, hoverPrice ? METAL_ACCENT : METAL_PANEL);
        DrawTextCustom("Preço", filterX + 10, filterItemY + 12, 18, TEXT_WHITE);
        DrawTextCustom(filterPriceExpanded ? "^" : "v", filterX + filterWidth - 25, filterItemY + 12, 18, METAL_HIGHLIGHT);
        if(CheckCollisionPointRec(GetMousePosition(), priceHeader) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            filterPriceExpanded = !filterPriceExpanded;
        }
        filterItemY += 45;
        
        if(filterPriceExpanded) {
            char priceBuf[64];
            snprintf(priceBuf, sizeof(priceBuf), "%.0f EUR - %.0f EUR", filterMinPrice, filterMaxPrice);
            DrawTextCustom(priceBuf, filterX + 10, filterItemY, 16, GOLD);
            filterItemY += 30;
            
            // Min price slider
            Rectangle minSliderTrack = {filterX + 10, filterItemY, filterWidth - 20, 6};
            DrawRectangleRounded(minSliderTrack, 0.5f, 4, METAL_ACCENT);
            float minHandleX = filterX + 10 + (filterMinPrice / 15000.0f) * (filterWidth - 20);
            Rectangle minHandle = {minHandleX - 8, filterItemY - 6, 16, 18};
            bool hoverMinHandle = CheckCollisionPointRec(GetMousePosition(), minHandle);
            DrawCircle(minHandleX, filterItemY + 3, 8, hoverMinHandle ? BUTTON_BLUE_HOVER : BUTTON_BLUE);
            
            // Drag min slider
            static bool draggingMin = false;
            if(CheckCollisionPointRec(GetMousePosition(), minHandle) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                draggingMin = true;
            }
            if(draggingMin && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                float mouseX = GetMousePosition().x;
                float ratio = (mouseX - (filterX + 10)) / (filterWidth - 20);
                if(ratio < 0) ratio = 0;
                if(ratio > 1) ratio = 1;
                filterMinPrice = ratio * 15000.0f;
                if(filterMinPrice > filterMaxPrice - 100) filterMinPrice = filterMaxPrice - 100;
            }
            if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) draggingMin = false;
            
            filterItemY += 25;
            
            // Max price slider
            Rectangle maxSliderTrack = {filterX + 10, filterItemY, filterWidth - 20, 6};
            DrawRectangleRounded(maxSliderTrack, 0.5f, 4, METAL_ACCENT);
            float maxHandleX = filterX + 10 + (filterMaxPrice / 15000.0f) * (filterWidth - 20);
            Rectangle maxHandle = {maxHandleX - 8, filterItemY - 6, 16, 18};
            bool hoverMaxHandle = CheckCollisionPointRec(GetMousePosition(), maxHandle);
            DrawCircle(maxHandleX, filterItemY + 3, 8, hoverMaxHandle ? BUTTON_BLUE_HOVER : BUTTON_BLUE);
            
            // Drag max slider
            static bool draggingMax = false;
            if(CheckCollisionPointRec(GetMousePosition(), maxHandle) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                draggingMax = true;
            }
            if(draggingMax && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                float mouseX = GetMousePosition().x;
                float ratio = (mouseX - (filterX + 10)) / (filterWidth - 20);
                if(ratio < 0) ratio = 0;
                if(ratio > 1) ratio = 1;
                filterMaxPrice = ratio * 15000.0f;
                if(filterMaxPrice < filterMinPrice + 100) filterMaxPrice = filterMinPrice + 100;
            }
            if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) draggingMax = false;
            
            filterItemY += 35;
        }
        
        // Product State Filter
        DrawRectangle(filterX, filterItemY, filterWidth, 1, METAL_ACCENT);
        filterItemY += 10;
        Rectangle stateHeader = {filterX, filterItemY, filterWidth, 40};
        bool hoverState = CheckCollisionPointRec(GetMousePosition(), stateHeader);
        DrawRectangleRounded(stateHeader, 0.1f, 4, hoverState ? METAL_ACCENT : METAL_PANEL);
        DrawTextCustom("Estado do Produto", filterX + 10, filterItemY + 12, 18, TEXT_WHITE);
        DrawTextCustom(filterStateExpanded ? "^" : "v", filterX + filterWidth - 25, filterItemY + 12, 18, METAL_HIGHLIGHT);
        if(CheckCollisionPointRec(GetMousePosition(), stateHeader) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            filterStateExpanded = !filterStateExpanded;
        }
        filterItemY += 45;
        
        if(filterStateExpanded) {
            Rectangle newBox = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(newBox, 2, METAL_ACCENT);
            if(filterNew) DrawRectangle(newBox.x + 4, newBox.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("Novos", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filterNew = !filterNew;
            }
            filterItemY += 30;
            
            Rectangle promoBox = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(promoBox, 2, METAL_ACCENT);
            if(filterPromotion) DrawRectangle(promoBox.x + 4, promoBox.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("Promoções", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filterPromotion = !filterPromotion;
            }
            filterItemY += 30;
            
            Rectangle highlightBox = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(highlightBox, 2, METAL_ACCENT);
            if(filterHighlight) DrawRectangle(highlightBox.x + 4, highlightBox.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("Destaques", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filterHighlight = !filterHighlight;
            }
            filterItemY += 35;
        }
        
        // Brand Filter (more specific than manufacturer)
        DrawRectangle(filterX, filterItemY, filterWidth, 1, METAL_ACCENT);
        filterItemY += 10;
        Rectangle brandHeader = {filterX, filterItemY, filterWidth, 40};
        bool hoverBrand = CheckCollisionPointRec(GetMousePosition(), brandHeader);
        DrawRectangleRounded(brandHeader, 0.1f, 4, hoverBrand ? METAL_ACCENT : METAL_PANEL);
        DrawTextCustom("Marca", filterX + 10, filterItemY + 12, 18, TEXT_WHITE);
        DrawTextCustom(filterBrandExpanded ? "^" : "v", filterX + filterWidth - 25, filterItemY + 12, 18, METAL_HIGHLIGHT);
        if(CheckCollisionPointRec(GetMousePosition(), brandHeader) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            filterBrandExpanded = !filterBrandExpanded;
        }
        filterItemY += 45;
        
        if(filterBrandExpanded) {
            Rectangle asusBox = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(asusBox, 2, METAL_ACCENT);
            if(filterAsus) DrawRectangle(asusBox.x + 4, asusBox.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("ASUS", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filterAsus = !filterAsus;
            }
            filterItemY += 30;
            
            Rectangle msiBox = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(msiBox, 2, METAL_ACCENT);
            if(filterMSI) DrawRectangle(msiBox.x + 4, msiBox.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("MSI", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filterMSI = !filterMSI;
            }
            filterItemY += 30;
            
            Rectangle gigabyteBox = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(gigabyteBox, 2, METAL_ACCENT);
            if(filterGigabyte) DrawRectangle(gigabyteBox.x + 4, gigabyteBox.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("Gigabyte", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filterGigabyte = !filterGigabyte;
            }
            filterItemY += 30;
            
            Rectangle corsairBox = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(corsairBox, 2, METAL_ACCENT);
            if(filterCorsair) DrawRectangle(corsairBox.x + 4, corsairBox.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("Corsair", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filterCorsair = !filterCorsair;
            }
            filterItemY += 35;
        }
        
        // Rating Filter
        DrawRectangle(filterX, filterItemY, filterWidth, 1, METAL_ACCENT);
        filterItemY += 10;
        Rectangle ratingHeader = {filterX, filterItemY, filterWidth, 40};
        bool hoverRating = CheckCollisionPointRec(GetMousePosition(), ratingHeader);
        DrawRectangleRounded(ratingHeader, 0.1f, 4, hoverRating ? METAL_ACCENT : METAL_PANEL);
        DrawTextCustom("Avaliação", filterX + 10, filterItemY + 12, 18, TEXT_WHITE);
        DrawTextCustom(filterRatingExpanded ? "^" : "v", filterX + filterWidth - 25, filterItemY + 12, 18, METAL_HIGHLIGHT);
        if(CheckCollisionPointRec(GetMousePosition(), ratingHeader) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            filterRatingExpanded = !filterRatingExpanded;
        }
        filterItemY += 45;
        
        if(filterRatingExpanded) {
            Rectangle star5Box = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(star5Box, 2, METAL_ACCENT);
            if(filter5Stars) DrawRectangle(star5Box.x + 4, star5Box.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("5 Estrelas", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filter5Stars = !filter5Stars;
            }
            filterItemY += 30;
            
            Rectangle star4Box = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(star4Box, 2, METAL_ACCENT);
            if(filter4Plus) DrawRectangle(star4Box.x + 4, star4Box.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("4+ Estrelas", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filter4Plus = !filter4Plus;
            }
            filterItemY += 30;
            
            Rectangle star3Box = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(star3Box, 2, METAL_ACCENT);
            if(filter3Plus) DrawRectangle(star3Box.x + 4, star3Box.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("3+ Estrelas", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filter3Plus = !filter3Plus;
            }
            filterItemY += 35;
        }
        
        // Shipping Filter
        DrawRectangle(filterX, filterItemY, filterWidth, 1, METAL_ACCENT);
        filterItemY += 10;
        Rectangle shippingHeader = {filterX, filterItemY, filterWidth, 40};
        bool hoverShipping = CheckCollisionPointRec(GetMousePosition(), shippingHeader);
        DrawRectangleRounded(shippingHeader, 0.1f, 4, hoverShipping ? METAL_ACCENT : METAL_PANEL);
        DrawTextCustom("Envio", filterX + 10, filterItemY + 12, 18, TEXT_WHITE);
        DrawTextCustom(filterShippingExpanded ? "^" : "v", filterX + filterWidth - 25, filterItemY + 12, 18, METAL_HIGHLIGHT);
        if(CheckCollisionPointRec(GetMousePosition(), shippingHeader) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            filterShippingExpanded = !filterShippingExpanded;
        }
        filterItemY += 45;
        
        if(filterShippingExpanded) {
            Rectangle freeBox = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(freeBox, 2, METAL_ACCENT);
            if(filterFreeShipping) DrawRectangle(freeBox.x + 4, freeBox.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("Envio Grátis", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filterFreeShipping = !filterFreeShipping;
            }
            filterItemY += 30;
            
            Rectangle expressBox = {filterX + 10, filterItemY, 20, 20};
            DrawRectangleLinesEx(expressBox, 2, METAL_ACCENT);
            if(filterExpressShipping) DrawRectangle(expressBox.x + 4, expressBox.y + 4, 12, 12, BUTTON_BLUE);
            DrawTextCustom("Envio Expresso", filterX + 40, filterItemY + 2, 16, TEXT_GRAY);
            if(CheckCollisionPointRec(GetMousePosition(), {filterX, filterItemY, filterWidth, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                filterExpressShipping = !filterExpressShipping;
            }
            filterItemY += 35;
        }
        
        EndScissorMode();
        
        // Filter sidebar scrolling
        float filterContentHeight = filterItemY - (filterY - filterScrollOffset);
        float filterMaxScroll = filterContentHeight - filterScrollArea.height;
        if(filterMaxScroll < 0) filterMaxScroll = 0;
        
        // Mouse wheel scrolling for filters (disable while dragging)
        if(CheckCollisionPointRec(GetMousePosition(), filterScrollArea) && !isDraggingFilterScrollbar) {
            float wheel = GetMouseWheelMove();
            if(wheel != 0) {
                filterScrollOffset -= wheel * 30.0f;
                if(filterScrollOffset < 0) filterScrollOffset = 0;
                if(filterScrollOffset > filterMaxScroll) filterScrollOffset = filterMaxScroll;
            }
        }
        
        // Draw scrollbar for filters if needed
        if(filterMaxScroll > 0) {
            float scrollbarX = filterScrollArea.width - 8;
            float scrollbarY = filterScrollArea.y;
            float scrollbarHeight = filterScrollArea.height;
            float scrollbarWidth = 6;
            
            DrawRectangleRounded({scrollbarX, scrollbarY, scrollbarWidth, scrollbarHeight}, 0.5f, 4, METAL_ACCENT);
            
            float thumbHeight = (filterScrollArea.height / (filterContentHeight)) * scrollbarHeight;
            if(thumbHeight < 30) thumbHeight = 30;
            float thumbY = scrollbarY + (filterScrollOffset / filterMaxScroll) * (scrollbarHeight - thumbHeight);
            Rectangle filterScrollThumb = {scrollbarX, thumbY, scrollbarWidth, thumbHeight};
            
            // Check if mouse is over filter scrollbar thumb
            bool hoverFilterThumb = CheckCollisionPointRec(GetMousePosition(), filterScrollThumb);
            Color filterThumbColor = hoverFilterThumb ? Fade(BUTTON_BLUE, 1.0f) : BUTTON_BLUE;
            
            // Handle filter scrollbar dragging
            if(hoverFilterThumb && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                isDraggingFilterScrollbar = true;
                dragFilterScrollStartY = GetMousePosition().y;
                dragFilterScrollStartOffset = filterScrollOffset;
            }
            
            if(isDraggingFilterScrollbar) {
                if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    float mouseY = GetMousePosition().y;
                    float deltaY = mouseY - dragFilterScrollStartY;
                    float scrollRange = scrollbarHeight - thumbHeight;
                    float scrollDelta = (deltaY / scrollRange) * filterMaxScroll;
                    
                    filterScrollOffset = dragFilterScrollStartOffset + scrollDelta;
                    if(filterScrollOffset < 0) filterScrollOffset = 0;
                    if(filterScrollOffset > filterMaxScroll) filterScrollOffset = filterMaxScroll;
                    
                    filterThumbColor = Fade(BUTTON_BLUE_HOVER, 1.0f);
                } else {
                    isDraggingFilterScrollbar = false;
                }
            }
            
            DrawRectangleRounded(filterScrollThumb, 0.5f, 4, filterThumbColor);
        }

        // Modern search bar with icon - moved to right side to avoid filter sidebar
        Rectangle searchBar = {450, 110, 380, 40};
        DrawRectangle(searchBar.x + 2, searchBar.y + 2, searchBar.width, searchBar.height, Fade(SHADOW_COLOR, 0.2f));
        DrawRectangleRounded(searchBar, 0.2f, 8, METAL_PANEL);
        Color searchBorderColor = isSearchActive ? BUTTON_BLUE : METAL_ACCENT;
        DrawRectangleLinesEx(searchBar, 2.0f, searchBorderColor);
        
        const char* searchLabel = "[?] Pesquisar produtos...";
        float labelX = searchBar.x + 16;
        float textStartX = labelX + 20;
        Rectangle textArea = {textStartX, searchBar.y, searchBar.width - 40, searchBar.height};

        if (!isSearchActive && search.empty()) {
            DrawTextCustom(searchLabel, labelX, searchBar.y + 10, 18, TEXT_GRAY);  // Maior e mais visível
        }
        if (CheckCollisionPointRec(GetMousePosition(), searchBar) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            isSearchActive = true;
        } else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !CheckCollisionPointRec(GetMousePosition(), searchBar)) {
            isSearchActive = false;
        }
        if (isSearchActive) {
            DrawTextCustom("[?]", labelX, searchBar.y + 10, 18, BUTTON_BLUE);
            float textWidth = MeasureTextCustom(search.c_str(), 18);
            BeginScissorMode((int)textArea.x, (int)textArea.y, (int)textArea.width, (int)textArea.height);
            DrawTextCustom(search.c_str(), (int)textStartX + 20, searchBar.y + 10, 18, TEXT_WHITE);
            if (((GetTime() * 2) - (int)(GetTime() * 2) < 0.5f)) {
                DrawRectangle((int)textStartX + 20 + (int)textWidth + 2, searchBar.y + 10, 2, 20, BUTTON_BLUE);
            }
            EndScissorMode();
        }
        if (isSearchActive) {
            if (IsKeyPressed(KEY_BACKSPACE) && !search.empty()) {
                search.pop_back();
            }
            int key = GetCharPressed();
            while (key > 0) {
                if (key >= 32 && key <= 126) {
                    std::string testStr = search + (char)key;
                    if (MeasureTextCustom(testStr.c_str(), 16) < (textArea.width - 10)) {
                        search += (char)key;
                    }
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_ESCAPE)) isSearchActive = false;
        }

        // Produtos filtrados por categoria e pesquisa
        std::vector<int> visible;
        for(size_t i=0;i<products.size();++i){
            bool shown=true;
            
            // Filter by category using enum
            if(selectedCategory != "All") {
                std::string prodCat = CategoryToString(products[i].category);
                if(prodCat != selectedCategory) shown = false;
            }
            
            // Filter by search
            std::string low=products[i].name+products[i].desc;
            std::transform(low.begin(),low.end(),low.begin(),::tolower);
            std::string q=search; std::transform(q.begin(),q.end(),q.begin(),::tolower);
            if(!q.empty()&&low.find(q)==std::string::npos) shown=false;
            
            // Filter by manufacturer (check product name for brand keywords)
            if(filterAMD || filterIntel || filterNvidia) {
                std::string nameLower = products[i].name;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                bool matchesManu = false;
                
                if(filterAMD && (nameLower.find("amd") != std::string::npos || nameLower.find("ryzen") != std::string::npos || nameLower.find("radeon") != std::string::npos)) {
                    matchesManu = true;
                }
                if(filterIntel && (nameLower.find("intel") != std::string::npos || nameLower.find("core i") != std::string::npos)) {
                    matchesManu = true;
                }
                if(filterNvidia && (nameLower.find("nvidia") != std::string::npos || nameLower.find("geforce") != std::string::npos || nameLower.find("rtx") != std::string::npos || nameLower.find("gtx") != std::string::npos)) {
                    matchesManu = true;
                }
                
                if(!matchesManu) shown = false;
            }
            
            // Filter by price range
            if(products[i].price < filterMinPrice || products[i].price > filterMaxPrice) {
                shown = false;
            }
            
            // Filter by stock status
            if(filterInStock && !products[i].inStock) {
                shown = false;
            }
            if(filterOutOfStock && products[i].inStock) {
                shown = false;
            }
            
            // Filter by product state
            if(filterPromotion && !products[i].isOnDiscount) {
                shown = false;
            }
            // Note: filterNew and filterHighlight would need additional fields in Product struct
            
            // Filter by brand (more specific than manufacturer)
            if(filterAsus || filterMSI || filterGigabyte || filterCorsair) {
                std::string nameLower = products[i].name;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                bool matchesBrand = false;
                
                if(filterAsus && nameLower.find("asus") != std::string::npos) {
                    matchesBrand = true;
                }
                if(filterMSI && nameLower.find("msi") != std::string::npos) {
                    matchesBrand = true;
                }
                if(filterGigabyte && nameLower.find("gigabyte") != std::string::npos) {
                    matchesBrand = true;
                }
                if(filterCorsair && nameLower.find("corsair") != std::string::npos) {
                    matchesBrand = true;
                }
                
                if(!matchesBrand) shown = false;
            }
            
            // Filter by rating
            if(filter5Stars || filter4Plus || filter3Plus) {
                bool matchesRating = false;
                
                if(filter5Stars && products[i].rating >= 4.8f) {
                    matchesRating = true;
                }
                if(filter4Plus && products[i].rating >= 4.0f) {
                    matchesRating = true;
                }
                if(filter3Plus && products[i].rating >= 3.0f) {
                    matchesRating = true;
                }
                
                if(!matchesRating) shown = false;
            }
            
            // Filter by shipping (placeholder logic - would need shipping field in Product)
            // For now, we'll use category and price as proxy:
            // - Free shipping for items over 100 EUR or CPUs/GPUs
            // - Express shipping available for smaller items under 500 EUR
            if(filterFreeShipping || filterExpressShipping) {
                bool matchesShipping = false;
                ProductCategory cat = products[i].category;
                
                if(filterFreeShipping) {
                    // Free shipping for expensive items or CPUs/GPUs
                    if(products[i].price > 100 || cat == ProductCategory::CPU || 
                       cat == ProductCategory::GPU) {
                        matchesShipping = true;
                    }
                }
                if(filterExpressShipping) {
                    // Express shipping for smaller/lighter items
                    if(cat == ProductCategory::RAM || cat == ProductCategory::STORAGE ||
                       cat == ProductCategory::COOLING || products[i].price < 500) {
                        matchesShipping = true;
                    }
                }
                
                if(!matchesShipping) shown = false;
            }
            
            if(shown) visible.push_back((int)i);
        }
        
        // Apply sorting
        if(sortMode == SORT_PRICE_ASC) {
            std::sort(visible.begin(), visible.end(), [&](int a, int b) {
                return products[a].price < products[b].price;
            });
        } else if(sortMode == SORT_PRICE_DESC) {
            std::sort(visible.begin(), visible.end(), [&](int a, int b) {
                return products[a].price > products[b].price;
            });
        } else if(sortMode == SORT_NAME) {
            std::sort(visible.begin(), visible.end(), [&](int a, int b) {
                return products[a].name < products[b].name;
            });
        }
        
        // Calculate total pages
        int totalProducts = (int)visible.size();
        int totalPages = (totalProducts + productsPerPage - 1) / productsPerPage;
        if(totalPages < 1) totalPages = 1;
        
        // Clamp current page
        if(currentPage >= totalPages) currentPage = totalPages - 1;
        if(currentPage < 0) currentPage = 0;
        
        // Get products for current page
        int startIdx = currentPage * productsPerPage;
        int endIdx = std::min(startIdx + productsPerPage, totalProducts);
        std::vector<int> pageVisible;
        for(int i = startIdx; i < endIdx; i++) {
            pageVisible.push_back(visible[i]);
        }

        // Sort buttons (moved to right side)
        Rectangle btnSortPrice = {1600, 110, 90, 40};
        Rectangle btnSortName = {1700, 110, 90, 40};
        Rectangle btnSortReset = {1800, 110, 90, 40};
        
        if(DrawButton("$ Preco", btnSortPrice, sortMode == SORT_PRICE_ASC || sortMode == SORT_PRICE_DESC ? SUCCESS_GREEN : METAL_ACCENT)) {
            sortMode = (sortMode == SORT_PRICE_ASC) ? SORT_PRICE_DESC : SORT_PRICE_ASC;
        }
        if(DrawButton("A-Z Nome", btnSortName, sortMode == SORT_NAME ? SUCCESS_GREEN : METAL_ACCENT)) {
            sortMode = SORT_NAME;
        }
        if(DrawButton("Resetar", btnSortReset, METAL_ACCENT)) {
            sortMode = SORT_NONE;
        }

        // Product counter with pagination info
        char counterText[128];
        snprintf(counterText, sizeof(counterText), "Mostrando %d-%d de %d produtos | Pagina %d/%d", 
                 startIdx + 1, endIdx, totalProducts, currentPage + 1, totalPages);
        DrawTextCustom(counterText, 1200, 120, 14, METAL_BRONZE);
        
        // Calculate max scroll based on content
        int totalRows = (pageVisible.size() + cols - 1) / cols;
        float contentHeight = totalRows * (cardH + gutter);
        float paginationHeight = totalPages > 1 ? 70 : 0; // Reserve space for pagination if needed
        float viewportHeight = screenHeight - 220 - paginationHeight; // Adjusted for UI and pagination
        maxScroll = contentHeight > viewportHeight ? contentHeight - viewportHeight : 0;
        
        // Handle scroll wheel input (disable while dragging scrollbar)
        if(!showCart && !showAdmin && !isSearchActive && !showCategoryFilter && !isDraggingScrollbar && !showProductDetails) {
            float wheelMove = GetMouseWheelMove();
            if(wheelMove != 0) {
                scrollOffset -= wheelMove * scrollSpeed;
                // Clamp scroll
                if(scrollOffset < 0) scrollOffset = 0;
                if(scrollOffset > maxScroll) scrollOffset = maxScroll;
            }
        }

        // Ambient lighting effect - radial gradient from top center
        Vector2 lightCenter = {(float)screenWidth / 2, -200};
        for(int r = 0; r < 5; r++) {
            float radius = 300 + r * 150;
            float alpha = (0.12f - r * 0.02f);
            DrawCircleGradient(lightCenter.x, lightCenter.y, radius, 
                             Fade(BUTTON_BLUE, alpha), Fade(BUTTON_BLUE, 0.0f));
        }
        
        // Scissor mode for scrollable area
        Rectangle scrollArea = {0, 190, (float)screenWidth, viewportHeight};
        BeginScissorMode(scrollArea.x, scrollArea.y, scrollArea.width, scrollArea.height);

        // Check if any blocking modal is open (excludes showProductDetails to allow switching between products)
        bool blockingModalOpen = showCart || showLoginPrompt || showUserProfile || showWishlist || showCheckout || showOrderHistory || showAdmin;
        
        // MODERN PRODUCT GRID with hover effects and scroll
        float productStartX = 350; // Start products after filter sidebar
        for(size_t k=0;k<pageVisible.size();++k){
            int i=pageVisible[k];
            int row=k/cols, col=k%cols;
            float x=productStartX+col*(cardW+gutter);
            float y=190+row*(cardH+gutter) - scrollOffset;
            Rectangle card{x,y,cardW,cardH};
            
            // Only render if visible in viewport
            if(y + cardH < 190 || y > 190 + viewportHeight) continue;
            
            // Disable hover and interaction when blocking modal is open
            bool isHovered = !blockingModalOpen && CheckCollisionPointRec(GetMousePosition(), card);
            
            // Smooth scale animation on hover
            static std::map<int, float> cardScales;
            if(cardScales.find(i) == cardScales.end()) cardScales[i] = 1.0f;
            float targetScale = isHovered ? 1.02f : 1.0f;
            cardScales[i] += (targetScale - cardScales[i]) * 0.2f; // Smooth lerp
            
            // Apply scale transform
            float scaleOffset = (cardW * cardScales[i] - cardW) / 2;
            Rectangle scaledCard = {card.x - scaleOffset, card.y - scaleOffset, cardW * cardScales[i], cardH * cardScales[i]};
            
            // Click on card to view details (but not on buttons area)
            Rectangle cardClickArea = {scaledCard.x, scaledCard.y, scaledCard.width, scaledCard.height - 50};
            if(!blockingModalOpen && CheckCollisionPointRec(GetMousePosition(), cardClickArea) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                selectedProductForDetails = i;
                showProductDetails = true;
            }
            
            // Enhanced multi-layer shadow with blur effect
            DrawRectangleRounded({scaledCard.x + 3, scaledCard.y + 8, scaledCard.width, scaledCard.height}, 0.06f, 8, Fade(SHADOW_COLOR, 0.6f));
            DrawRectangleRounded({scaledCard.x + 2, scaledCard.y + 5, scaledCard.width, scaledCard.height}, 0.06f, 8, Fade(SHADOW_COLOR, 0.4f));
            DrawRectangleRounded({scaledCard.x + 1, scaledCard.y + 2, scaledCard.width, scaledCard.height}, 0.06f, 8, Fade(SHADOW_COLOR, 0.2f));
            
            // Card background with gradient
            Color cardBg = isHovered ? CARD_HOVER : METAL_PANEL;
            DrawRectangleRounded(scaledCard, 0.06f, 8, cardBg);
            
            // Animated inner glow when hovered
            if(isHovered) {
                static float glowPulse = 0.0f;
                glowPulse += 0.05f;
                float glowIntensity = 0.08f + sin(glowPulse) * 0.03f;
                DrawRectangleRounded(scaledCard, 0.06f, 8, Fade(BUTTON_BLUE, glowIntensity));
                
                // Animated border pulse
                DrawRectangleLinesEx(scaledCard, 3.0f, Fade(BUTTON_BLUE_HOVER, 0.6f + sin(glowPulse) * 0.2f));
            }
            
            // Premium border
            Color cardBorder = isHovered ? BUTTON_BLUE : METAL_ACCENT;
            DrawRectangleLinesEx(scaledCard, isHovered ? 2.5f : 1.5f, cardBorder);
            
            // Gradient shine effect
            Rectangle topShine = {scaledCard.x, scaledCard.y, scaledCard.width, 80};
            DrawRectangleGradientV(topShine.x, topShine.y, topShine.width, topShine.height, 
                                   Fade(METAL_HIGHLIGHT, 0.05f), Fade(METAL_HIGHLIGHT, 0.0f));
            
            // Use scaled card for remaining rendering
            card = scaledCard;
            
            // Color indicator strip
            Rectangle colorStrip = {card.x, card.y, 5, card.height};
            DrawRectangleRounded(colorStrip, 0.5f, 4, products[i].color);
            
            // Product image thumbnail (left side)
            Texture2D productImg = GetProductImage(products[i].imagePath);
            Rectangle imgRect = {x + 15, y + 10, 90, 90};
            DrawTexturePro(productImg, 
                          {0, 0, (float)productImg.width, (float)productImg.height},
                          imgRect,
                          {0, 0}, 0, WHITE);
            DrawRectangleLinesEx(imgRect, 1, METAL_ACCENT);
            
            // Wishlist heart icon (only if logged in)
            if(isLoggedIn && currentUser != nullptr) {
                bool isInWishlist = std::find(currentUser->wishlist.begin(), currentUser->wishlist.end(), products[i].id) != currentUser->wishlist.end();
                Rectangle heartBtn = {x + cardW - 45, y + 75, 35, 35};
                bool heartHover = !blockingModalOpen && CheckCollisionPointRec(GetMousePosition(), heartBtn);
                
                Color heartColor = isInWishlist ? BUTTON_RED : METAL_ACCENT;
                if(heartHover) heartColor = isInWishlist ? ColorBrightness(BUTTON_RED, 0.2f) : METAL_HIGHLIGHT;
                
                DrawRectangleRounded(heartBtn, 0.2f, 8, Fade(heartColor, 0.3f));
                DrawRectangleLinesEx(heartBtn, 2, heartColor);
                
                // Heart icon (♥)
                const char* heartIcon = isInWishlist ? "@" : "o";
                int heartW = MeasureTextCustom(heartIcon, 24);
                DrawTextCustom(heartIcon, heartBtn.x + (heartBtn.width - heartW) / 2, heartBtn.y + 6, 24, heartColor);
                
                if(!blockingModalOpen && heartHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if(isInWishlist) {
                        // Remove from wishlist
                        currentUser->wishlist.erase(
                            std::remove(currentUser->wishlist.begin(), currentUser->wishlist.end(), products[i].id),
                            currentUser->wishlist.end()
                        );
                        SaveUsers(users);
                        toastMessage = "Removido da lista de desejos!";
                        toastTimer = 2.0f;
                    } else {
                        // Add to wishlist
                        currentUser->wishlist.push_back(products[i].id);
                        SaveUsers(users);
                        toastMessage = "Adicionado a lista de desejos!";
                        toastTimer = 2.0f;
                    }
                }
            }
            
            // Product name (shifted right to make room for image) - FONTE MAIOR
            DrawTextWithShadow(products[i].name.c_str(), x+115, y+16, 24, TEXT_WHITE);
            
            // Description (shifted right) - FONTE MAIOR
            DrawTextCustom(products[i].desc.c_str(), x+115, y+50, 16, TEXT_GRAY);
            
            // Premium discount badge (if on discount)
            if(products[i].isOnDiscount && products[i].discountPercent > 0) {
                Rectangle discountBadge = {x + 10, y + 10, 70, 32};
                // Shadow for badge
                DrawRectangleRounded({discountBadge.x + 1, discountBadge.y + 2, discountBadge.width, discountBadge.height}, 0.4f, 6, Fade(SHADOW_COLOR, 0.6f));
                // Gradient background effect
                DrawRectangleRounded(discountBadge, 0.4f, 6, BUTTON_RED);
                DrawRectangleRounded({discountBadge.x, discountBadge.y, discountBadge.width, discountBadge.height / 2}, 0.4f, 6, Fade(WHITE, 0.15f));
                // Border for sharpness
                DrawRectangleLinesEx(discountBadge, 1.5f, Fade(WHITE, 0.3f));
                char discountBuf[16];
                snprintf(discountBuf, sizeof(discountBuf), "-%0.f%%", products[i].discountPercent);
                int discW = MeasureTextCustom(discountBuf, 17);
                DrawTextWithShadow(discountBuf, discountBadge.x + (discountBadge.width - discW)/2, discountBadge.y + 7, 17, TEXT_WHITE, 1);
            }
            
            // Ultra-premium price badge with animated shimmer
            char priceBuf[32];
            float displayPrice = products[i].price;
            bool hasDiscount = products[i].isOnDiscount && products[i].discountPercent > 0;
            if(hasDiscount) {
                displayPrice = products[i].price * (1.0f - products[i].discountPercent / 100.0f);
                snprintf(priceBuf, sizeof(priceBuf), "EUR %.2f", displayPrice);
            } else {
                snprintf(priceBuf, sizeof(priceBuf), "EUR %.2f", products[i].price);
            }
            int priceW = MeasureTextCustom(priceBuf, 22);
            Rectangle priceBadge = {x + cardW - priceW - 30, y + 14, (float)priceW + 20, 30};
            
            // Animated shimmer effect on price
            static float shimmerTime = 0.0f;
            shimmerTime += GetFrameTime() * 2.0f;
            float shimmerX = priceBadge.x + fmod(shimmerTime * 50, priceBadge.width + 100) - 50;
            
            // Premium gradient background
            DrawRectangleGradientV(priceBadge.x, priceBadge.y, priceBadge.width, priceBadge.height,
                                   hasDiscount ? PREMIUM_GOLD : SUCCESS_GREEN,
                                   hasDiscount ? ColorBrightness(PREMIUM_GOLD, -0.2f) : ColorBrightness(SUCCESS_GREEN, -0.2f));
            
            // Shimmer sweep
            Rectangle shimmer = {shimmerX, priceBadge.y, 30, priceBadge.height};
            DrawRectangleGradientH(shimmer.x, shimmer.y, shimmer.width, shimmer.height,
                                   Fade(WHITE, 0.0f), Fade(WHITE, 0.4f));
            DrawRectangleGradientH(shimmer.x + 15, shimmer.y, shimmer.width, shimmer.height,
                                   Fade(WHITE, 0.4f), Fade(WHITE, 0.0f));
            
            // Outer glow
            DrawRectangleLinesEx({priceBadge.x - 1, priceBadge.y - 1, priceBadge.width + 2, priceBadge.height + 2}, 
                                2.0f, Fade(hasDiscount ? PREMIUM_GOLD : SUCCESS_GREEN, 0.5f));
            // Premium text with glow
            DrawTextWithShadow(priceBuf, priceBadge.x + 10, priceBadge.y + 5, 22, TEXT_WHITE);
            
            // Show original price if discounted
            if(products[i].isOnDiscount && products[i].discountPercent > 0) {
                char originalPriceBuf[32];
                snprintf(originalPriceBuf, sizeof(originalPriceBuf), "EUR %.2f", products[i].price);
                int origPriceW = MeasureTextCustom(originalPriceBuf, 14);
                DrawTextCustom(originalPriceBuf, x + cardW - origPriceW - 20, y + 48, 14, TEXT_GRAY);
                DrawLine(x + cardW - origPriceW - 20, y + 56, x + cardW - 20, y + 56, TEXT_GRAY);
            }
            
            // Out of stock overlay
            if(!products[i].inStock) {
                DrawRectangleRounded(card, 0.05f, 8, Fade(BLACK, 0.7f));
                const char* outOfStockText = "ESGOTADO";
                int textW = MeasureTextCustom(outOfStockText, 32);
                DrawTextWithShadow(outOfStockText, x + (cardW - textW)/2, y + cardH/2 - 16, 32, BUTTON_RED);
            }

            // Quantity selector with +/- buttons
            // Initialize quantity if not set
            if(productQuantitySelector.find(products[i].id) == productQuantitySelector.end()) {
                productQuantitySelector[products[i].id] = 1;
            }
            int& selectedQty = productQuantitySelector[products[i].id];
            
            // Quantity control buttons (disabled if out of stock)
            Rectangle btnQtyMinus = {x + 115, y + cardH - 40, 35, 32};
            Rectangle btnQtyDisplay = {x + 155, y + cardH - 40, 50, 32};
            Rectangle btnQtyPlus = {x + 210, y + cardH - 40, 35, 32};
            
            if(products[i].inStock) {
                // Minus button
                if(!blockingModalOpen && DrawButton("-", btnQtyMinus, BUTTON_RED)) {
                    if(selectedQty > 1) selectedQty--;
                }
                
                // Quantity display
                char qtyBuf[16];
                snprintf(qtyBuf, sizeof(qtyBuf), "%d", selectedQty);
                DrawRectangleRounded(btnQtyDisplay, 0.1f, 4, METAL_ACCENT);
                DrawRectangleLinesEx(btnQtyDisplay, 1.0f, METAL_HIGHLIGHT);
                int qtyTextW = MeasureTextCustom(qtyBuf, 18);
                DrawTextCustom(qtyBuf, btnQtyDisplay.x + (btnQtyDisplay.width - qtyTextW) / 2, btnQtyDisplay.y + 7, 18, TEXT_WHITE);
                
                // Plus button
                if(!blockingModalOpen && DrawButton("+", btnQtyPlus, SUCCESS_GREEN)) {
                    selectedQty++;
                }
                
                // Add to cart button (uses selected quantity)
                Rectangle btn = {x + cardW - 130, y + cardH - 40, 115, 32};
                if(!blockingModalOpen && DrawButton("Carrinho", btn, BUTTON_BLUE)){
                    if(!isLoggedIn) showLoginPrompt=true;
                    else{
                        auto it=std::find_if(cart.begin(),cart.end(),
                            [&](const CartItem&c){return c.product.id==products[i].id;});
                        char toastBuf[128];
                        if(it!=cart.end()) {
                            it->qty += selectedQty;
                            snprintf(toastBuf, sizeof(toastBuf), "Adicionado %d unidade(s)!", selectedQty);
                        } else {
                            cart.push_back({products[i], selectedQty});
                            snprintf(toastBuf, sizeof(toastBuf), "%d produto(s) adicionado(s)!", selectedQty);
                        }
                        toastMessage = std::string(toastBuf);
                        toastTimer = 2.0f;
                        selectedQty = 1; // Reset after adding
                    }
                }
            }
        }
        
        EndScissorMode();
        
        // Draw scrollbar if needed
        if(maxScroll > 0) {
            float scrollbarX = screenWidth - 12;
            float scrollbarY = 195;
            float scrollbarHeight = viewportHeight - 10;
            float scrollbarWidth = 8;
            
            // Scrollbar track
            DrawRectangleRounded({scrollbarX, scrollbarY, scrollbarWidth, scrollbarHeight}, 0.5f, 4, METAL_ACCENT);
            
            // Scrollbar thumb
            float thumbHeight = (viewportHeight / (contentHeight + viewportHeight)) * scrollbarHeight;
            if(thumbHeight < 30) thumbHeight = 30;
            float thumbY = scrollbarY + (scrollOffset / maxScroll) * (scrollbarHeight - thumbHeight);
            Rectangle scrollThumb = {scrollbarX, thumbY, scrollbarWidth, thumbHeight};
            
            // Check if mouse is over scrollbar thumb
            bool hoverThumb = CheckCollisionPointRec(GetMousePosition(), scrollThumb);
            Color thumbColor = hoverThumb ? Fade(BUTTON_BLUE, 1.0f) : BUTTON_BLUE;
            
            // Handle scrollbar dragging
            if(hoverThumb && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                isDraggingScrollbar = true;
                dragScrollStartY = GetMousePosition().y;
                dragScrollStartOffset = scrollOffset;
            }
            
            if(isDraggingScrollbar) {
                if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    float mouseY = GetMousePosition().y;
                    float deltaY = mouseY - dragScrollStartY;
                    float scrollRange = scrollbarHeight - thumbHeight;
                    float scrollDelta = (deltaY / scrollRange) * maxScroll;
                    
                    scrollOffset = dragScrollStartOffset + scrollDelta;
                    if(scrollOffset < 0) scrollOffset = 0;
                    if(scrollOffset > maxScroll) scrollOffset = maxScroll;
                    
                    thumbColor = Fade(BUTTON_BLUE_HOVER, 1.0f);
                } else {
                    isDraggingScrollbar = false;
                }
            }
            
            DrawRectangleRounded(scrollThumb, 0.5f, 4, thumbColor);
        }
        
        // Pagination controls at bottom
        if(totalPages > 1) {
            float paginationY = screenHeight - 60;
            float paginationX = screenWidth / 2 - 250;
            
            // Previous button
            Rectangle btnPrev = {paginationX, paginationY, 100, 40};
            if(DrawButton("<< Anterior", btnPrev, currentPage > 0 ? BUTTON_BLUE : METAL_ACCENT)) {
                if(currentPage > 0) {
                    currentPage--;
                    scrollOffset = 0; // Reset scroll when changing pages
                }
            }
            
            // Page info
            char pageInfo[64];
            snprintf(pageInfo, sizeof(pageInfo), "Pagina %d / %d", currentPage + 1, totalPages);
            DrawTextCustom(pageInfo, paginationX + 120, paginationY + 12, 18, TEXT_WHITE);
            
            // Next button
            Rectangle btnNext = {paginationX + 250, paginationY, 100, 40};
            if(DrawButton("Proximo >>", btnNext, currentPage < totalPages - 1 ? BUTTON_BLUE : METAL_ACCENT)) {
                if(currentPage < totalPages - 1) {
                    currentPage++;
                    scrollOffset = 0; // Reset scroll when changing pages
                }
            }
            
            // Quick page jump buttons (first 5 pages)
            float quickJumpX = paginationX + 370;
            for(int p = 0; p < std::min(5, totalPages); p++) {
                Rectangle btnPage = {quickJumpX + p * 45, paginationY, 40, 40};
                char pageNum[8];
                snprintf(pageNum, sizeof(pageNum), "%d", p + 1);
                Color pageColor = (p == currentPage) ? SUCCESS_GREEN : METAL_PANEL;
                if(DrawButton(pageNum, btnPage, pageColor)) {
                    currentPage = p;
                    scrollOffset = 0;
                }
            }
        }

        // Category filter button with dropdown (DRAWN AFTER products to appear on top) - moved to right
        Rectangle btnCategoryFilter = {850, 110, 180, 40};
        bool hoverCatBtn = CheckCollisionPointRec(GetMousePosition(), btnCategoryFilter);
        
        DrawRectangle(btnCategoryFilter.x + 2, btnCategoryFilter.y + 2, btnCategoryFilter.width, btnCategoryFilter.height, Fade(SHADOW_COLOR, 0.2f));
        Color catBtnBg = showCategoryFilter ? BUTTON_BLUE : (hoverCatBtn ? METAL_ACCENT : METAL_PANEL);
        DrawRectangleRounded(btnCategoryFilter, 0.2f, 8, catBtnBg);
        DrawRectangleLinesEx(btnCategoryFilter, 2.0f, hoverCatBtn || showCategoryFilter ? BUTTON_BLUE : METAL_ACCENT);
        
        char catLabel[64];
        snprintf(catLabel, sizeof(catLabel), "[#] %s", selectedCategory.c_str());
        int catLabelW = MeasureTextCustom(catLabel, 16);
        DrawTextCustom(catLabel, btnCategoryFilter.x + (btnCategoryFilter.width - catLabelW)/2 - 10, btnCategoryFilter.y + 12, 16, showCategoryFilter ? TEXT_WHITE : METAL_HIGHLIGHT);
        
        // Arrow indicator
        const char* arrow = showCategoryFilter ? "^" : "v";
        DrawTextCustom(arrow, btnCategoryFilter.x + btnCategoryFilter.width - 20, btnCategoryFilter.y + 12, 16, showCategoryFilter ? TEXT_WHITE : METAL_HIGHLIGHT);
        
        if(CheckCollisionPointRec(GetMousePosition(), btnCategoryFilter) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            showCategoryFilter = !showCategoryFilter;
        }
        
        // Category dropdown panel
        if(showCategoryFilter) {
            Rectangle dropdownPanel = {btnCategoryFilter.x, btnCategoryFilter.y + 45, 500, 130};
            DrawRectangle(dropdownPanel.x + 3, dropdownPanel.y + 3, dropdownPanel.width, dropdownPanel.height, Fade(SHADOW_COLOR, 0.4f));
            DrawRectangleRounded(dropdownPanel, 0.15f, 8, METAL_PANEL);
            DrawRectangleLinesEx(dropdownPanel, 2.0f, BUTTON_BLUE);
            
            // Category chips inside dropdown
            float chipX = dropdownPanel.x + 10;
            float chipY = dropdownPanel.y + 10;
            for (size_t ci=0; ci<categories.size(); ++ci) {
                Rectangle chip{chipX, chipY, 95, 34};
                bool active = (categories[ci] == selectedCategory);
                bool hovered = CheckCollisionPointRec(GetMousePosition(), chip);
                
                Color chipBg = active ? BUTTON_BLUE : (hovered ? METAL_ACCENT : METAL_BG);
                DrawRectangleRounded(chip, 0.3f, 8, chipBg);
                
                int txtW = MeasureTextCustom(categories[ci].c_str(), 15);
                DrawTextCustom(categories[ci].c_str(), chip.x + (chip.width - txtW)/2, chip.y + 9, 15, active ? TEXT_WHITE : METAL_HIGHLIGHT);
                
                if(CheckCollisionPointRec(GetMousePosition(),chip) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    selectedCategory = categories[ci];
                    showCategoryFilter = false;
                    scrollOffset = 0; // Reset scroll when changing category
                    currentPage = 0;  // Reset to first page
                }
                
                chipX += 98;
                if(ci == 4) {
                    chipX = dropdownPanel.x + 10;
                    chipY += 38;
                }
            }
        }
        
        // Ultra-premium Toast notification with bounce animation
        if(toastTimer > 0) {
            float alpha = toastTimer > 1.5f ? 1.0f : (toastTimer / 1.5f);
            // Elastic bounce animation
            float slideIn = 0;
            if(toastTimer > 2.5f) {
                float t = (3.0f - toastTimer) / 0.5f; // 0 to 1
                slideIn = (1 - pow(1 - t, 3)) * 60; // Ease-out cubic for smooth entry
            }
            Rectangle toastBox = {(float)screenWidth/2 - 220, 100 - slideIn, 440, 60};
            
            // Animated glow around toast
            static float toastGlow = 0.0f;
            toastGlow += 0.1f;
            float glowSize = 4.0f + sin(toastGlow) * 2.0f;
            DrawRectangleRounded({toastBox.x - glowSize, toastBox.y - glowSize, 
                                 toastBox.width + glowSize*2, toastBox.height + glowSize*2}, 
                                0.3f, 8, Fade(SUCCESS_GREEN, (0.2f + sin(toastGlow) * 0.1f) * alpha));
            
            // Triple-layer shadow for depth
            DrawRectangleRounded({toastBox.x + 3, toastBox.y + 6, toastBox.width, toastBox.height}, 0.3f, 8, Fade(SHADOW_COLOR, 0.6f * alpha));
            DrawRectangleRounded({toastBox.x + 2, toastBox.y + 4, toastBox.width, toastBox.height}, 0.3f, 8, Fade(SHADOW_COLOR, 0.4f * alpha));
            
            // Main toast background with gradient
            DrawRectangleRounded(toastBox, 0.3f, 8, Fade(SUCCESS_GREEN, 0.95f * alpha));
            Rectangle topShine = {toastBox.x, toastBox.y, toastBox.width, toastBox.height / 2};
            DrawRectangleRounded(topShine, 0.3f, 8, Fade(WHITE, 0.15f * alpha));
            
            // Glowing border
            DrawRectangleLinesEx(toastBox, 2.5f, Fade(SUCCESS_GREEN_HOVER, alpha));
            
            // Icon (checkmark)
            DrawTextCustom("v", toastBox.x + 20, toastBox.y + 16, 24, Fade(TEXT_WHITE, alpha));
            
            // Message text (centered with icon offset)
            DrawTextWithShadow(toastMessage.c_str(), toastBox.x + 60, toastBox.y + 19, 19, Fade(TEXT_WHITE, alpha), 1);
        }

        if(showLoginPrompt){
            if(LoginFunc(screenWidth,screenHeight))isLoggedIn=true;
            showLoginPrompt=false;
        }

        // MODAL DO CARRINHO
        if(showCart){
            ShowCartModal(screenWidth, screenHeight, cart, showCart, cartMessage, showCheckout, isLoggedIn, showLoginPrompt);
        }
        
        // LOGIN/REGISTER MODAL
        if(showLoginPrompt) {
            // Dark overlay
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.75f));
            
            enum AuthMode { LOGIN, REGISTER };
            static AuthMode authMode = LOGIN;
            static std::string authUsername = "";
            static std::string authPassword = "";
            static std::string authEmail = "";
            static bool typingUsername = false;
            static bool typingPassword = false;
            static bool typingEmail = false;
            static std::string authMessage = "";
            static Color authMessageColor = BUTTON_RED;
            
            // Modal window
            float modalWidth = authMode == REGISTER ? 500 : 450;
            float modalHeight = authMode == REGISTER ? 420 : 350;
            Rectangle modal = {(float)screenWidth/2 - modalWidth/2, (float)screenHeight/2 - modalHeight/2, modalWidth, modalHeight};
            
            DrawRectangle(modal.x + 4, modal.y + 4, modal.width, modal.height, Fade(SHADOW_COLOR, 0.5f));
            DrawRectangleRounded(modal, 0.03f, 8, METAL_PANEL);
            DrawRectangleLinesEx(modal, 3, BUTTON_BLUE);
            
            // Close button
            Rectangle btnClose = {modal.x + modal.width - 45, modal.y + 10, 35, 35};
            if(DrawButton("X", btnClose, BUTTON_RED)) {
                showLoginPrompt = false;
                authUsername = "";
                authPassword = "";
                authEmail = "";
                authMessage = "";
                typingUsername = false;
                typingPassword = false;
                typingEmail = false;
            }
            
            // Title
            const char* titleText = authMode == LOGIN ? "Login" : "Criar Conta";
            DrawTextWithShadow(titleText, modal.x + 30, modal.y + 25, 32, TEXT_WHITE);
            
            float fieldX = modal.x + 30;
            float fieldY = modal.y + 85;
            float fieldWidth = modalWidth - 60;
            float fieldHeight = 45;
            float fieldSpacing = 65;
            
            // Username field
            DrawTextCustom("Utilizador:", fieldX, fieldY - 25, 18, TEXT_WHITE);
            Rectangle usernameField = {fieldX, fieldY, fieldWidth, fieldHeight};
            DrawRectangleRounded(usernameField, 0.1f, 8, typingUsername ? METAL_ACCENT : METAL_BG);
            DrawRectangleLinesEx(usernameField, 2, typingUsername ? BUTTON_BLUE : METAL_ACCENT);
            DrawTextCustom(authUsername.c_str(), fieldX + 15, fieldY + 13, 18, TEXT_WHITE);
            
            if(CheckCollisionPointRec(GetMousePosition(), usernameField) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                typingUsername = true;
                typingPassword = false;
                typingEmail = false;
            }
            
            if(typingUsername) {
                int key = GetCharPressed();
                while(key > 0) {
                    if(key >= 32 && key <= 125 && authUsername.length() < 20) {
                        authUsername += (char)key;
                    }
                    key = GetCharPressed();
                }
                if(IsKeyPressed(KEY_BACKSPACE) && authUsername.length() > 0) {
                    authUsername.pop_back();
                }
                if(IsKeyPressed(KEY_TAB)) {
                    typingUsername = false;
                    typingPassword = true;
                }
            }
            
            fieldY += fieldSpacing;
            
            // Password field
            DrawTextCustom("Password:", fieldX, fieldY - 25, 18, TEXT_WHITE);
            Rectangle passwordField = {fieldX, fieldY, fieldWidth, fieldHeight};
            DrawRectangleRounded(passwordField, 0.1f, 8, typingPassword ? METAL_ACCENT : METAL_BG);
            DrawRectangleLinesEx(passwordField, 2, typingPassword ? BUTTON_BLUE : METAL_ACCENT);
            
            // Show password as asterisks
            std::string passwordDisplay(authPassword.length(), '*');
            DrawTextCustom(passwordDisplay.c_str(), fieldX + 15, fieldY + 13, 18, TEXT_WHITE);
            
            if(CheckCollisionPointRec(GetMousePosition(), passwordField) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                typingUsername = false;
                typingPassword = true;
                typingEmail = false;
            }
            
            if(typingPassword) {
                int key = GetCharPressed();
                while(key > 0) {
                    if(key >= 32 && key <= 125 && authPassword.length() < 30) {
                        authPassword += (char)key;
                    }
                    key = GetCharPressed();
                }
                if(IsKeyPressed(KEY_BACKSPACE) && authPassword.length() > 0) {
                    authPassword.pop_back();
                }
                if(IsKeyPressed(KEY_TAB) && authMode == REGISTER) {
                    typingPassword = false;
                    typingEmail = true;
                } else if(IsKeyPressed(KEY_ENTER)) {
                    typingPassword = false;
                }
            }
            
            fieldY += fieldSpacing;
            
            // Email field (only for registration)
            if(authMode == REGISTER) {
                DrawTextCustom("Email:", fieldX, fieldY - 25, 18, TEXT_WHITE);
                Rectangle emailField = {fieldX, fieldY, fieldWidth, fieldHeight};
                DrawRectangleRounded(emailField, 0.1f, 8, typingEmail ? METAL_ACCENT : METAL_BG);
                DrawRectangleLinesEx(emailField, 2, typingEmail ? BUTTON_BLUE : METAL_ACCENT);
                DrawTextCustom(authEmail.c_str(), fieldX + 15, fieldY + 13, 18, TEXT_WHITE);
                
                if(CheckCollisionPointRec(GetMousePosition(), emailField) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    typingUsername = false;
                    typingPassword = false;
                    typingEmail = true;
                }
                
                if(typingEmail) {
                    int key = GetCharPressed();
                    while(key > 0) {
                        if(key >= 32 && key <= 125 && authEmail.length() < 50) {
                            authEmail += (char)key;
                        }
                        key = GetCharPressed();
                    }
                    if(IsKeyPressed(KEY_BACKSPACE) && authEmail.length() > 0) {
                        authEmail.pop_back();
                    }
                    if(IsKeyPressed(KEY_ENTER)) {
                        typingEmail = false;
                    }
                }
                
                fieldY += fieldSpacing;
            }
            
            // Error/success message
            if(!authMessage.empty()) {
                DrawTextCustom(authMessage.c_str(), fieldX, fieldY, 16, authMessageColor);
                fieldY += 30;
            }
            
            // Submit button
            Rectangle btnSubmit = {modal.x + modal.width - 170, modal.y + modalHeight - 60, 140, 45};
            const char* submitText = authMode == LOGIN ? "Entrar" : "Registar";
            if(DrawButton(submitText, btnSubmit, SUCCESS_GREEN)) {
                if(authMode == LOGIN) {
                    // Login logic
                    if(authUsername.empty() || authPassword.empty()) {
                        authMessage = "Preencha todos os campos!";
                        authMessageColor = BUTTON_RED;
                    } else {
                        User* user = FindUser(users, authUsername);
                        std::string passHash = SimpleHash(authPassword);
                        
                        if(user && user->passwordHash == passHash) {
                            currentUser = user;
                            currentUsername = user->username;
                            isLoggedIn = true;
                            showLoginPrompt = false;
                            authUsername = "";
                            authPassword = "";
                            authMessage = "";
                            toastMessage = "Login bem-sucedido!";
                            toastTimer = 2.0f;
                        } else {
                            authMessage = "Utilizador ou password incorretos!";
                            authMessageColor = BUTTON_RED;
                        }
                    }
                } else {
                    // Register logic
                    if(authUsername.empty() || authPassword.empty() || authEmail.empty()) {
                        authMessage = "Preencha todos os campos!";
                        authMessageColor = BUTTON_RED;
                    } else if(FindUser(users, authUsername) != nullptr) {
                        authMessage = "Utilizador ja existe!";
                        authMessageColor = BUTTON_RED;
                    } else {
                        // Create new user
                        User newUser;
                        newUser.id = users.empty() ? 1 : users.back().id + 1;
                        newUser.username = authUsername;
                        newUser.passwordHash = SimpleHash(authPassword);
                        newUser.email = authEmail;
                        users.push_back(newUser);
                        SaveUsers(users);
                        
                        authMessage = "Conta criada! Faca login.";
                        authMessageColor = SUCCESS_GREEN;
                        authMode = LOGIN;
                        authPassword = "";
                        authEmail = "";
                    }
                }
            }
            
            // Toggle mode button
            Rectangle btnToggleMode = {modal.x + 30, modal.y + modalHeight - 60, 200, 45};
            const char* toggleText = authMode == LOGIN ? "Criar nova conta" : "Ja tenho conta";
            if(DrawButton(toggleText, btnToggleMode, METAL_ACCENT)) {
                authMode = (authMode == LOGIN) ? REGISTER : LOGIN;
                authMessage = "";
                authPassword = "";
                authEmail = "";
                typingUsername = false;
                typingPassword = false;
                typingEmail = false;
            }
        }
        
        // USER PROFILE DROPDOWN
        if(showUserProfile && isLoggedIn && currentUser != nullptr) {
            // Position dropdown below user button
            float dropdownX = screenWidth - 330;
            float dropdownY = 70;
            float dropdownWidth = 280;
            float dropdownHeight = 240;
            
            Rectangle dropdown = {dropdownX, dropdownY, dropdownWidth, dropdownHeight};
            
            // Shadow
            DrawRectangle(dropdown.x + 4, dropdown.y + 4, dropdown.width, dropdown.height, Fade(SHADOW_COLOR, 0.5f));
            
            // Background
            DrawRectangleRounded(dropdown, 0.05f, 8, METAL_PANEL);
            DrawRectangleLinesEx(dropdown, 2, BUTTON_BLUE);
            
            float itemY = dropdown.y + 15;
            
            // Username header
            DrawTextWithShadow(currentUsername.c_str(), dropdown.x + 15, itemY, 22, TEXT_WHITE);
            itemY += 35;
            
            // Separator
            DrawRectangle(dropdown.x + 10, itemY, dropdown.width - 20, 2, METAL_ACCENT);
            itemY += 15;
            
            // Wishlist count
            char wishlistText[64];
            snprintf(wishlistText, sizeof(wishlistText), "Lista de Desejos (%d)", (int)currentUser->wishlist.size());
            Rectangle wishlistBtn = {dropdown.x + 10, itemY, dropdown.width - 20, 40};
            bool wishlistHover = CheckCollisionPointRec(GetMousePosition(), wishlistBtn);
            DrawRectangleRounded(wishlistBtn, 0.1f, 6, wishlistHover ? METAL_ACCENT : METAL_BG);
            DrawTextCustom(wishlistText, dropdown.x + 20, itemY + 11, 18, TEXT_WHITE);
            
            if(wishlistHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                showWishlist = true;
                showUserProfile = false;
            }
            itemY += 50;
            
            // Order history
            char ordersText[64];
            snprintf(ordersText, sizeof(ordersText), "Historico de Pedidos (%d)", (int)currentUser->orderHistory.size());
            Rectangle ordersBtn = {dropdown.x + 10, itemY, dropdown.width - 20, 40};
            bool ordersHover = CheckCollisionPointRec(GetMousePosition(), ordersBtn);
            DrawRectangleRounded(ordersBtn, 0.1f, 6, ordersHover ? METAL_ACCENT : METAL_BG);
            DrawTextCustom(ordersText, dropdown.x + 20, itemY + 11, 18, TEXT_WHITE);
            
            if(ordersHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                showOrderHistory = true;
                showUserProfile = false;
            }
            itemY += 50;
            
            // Separator
            DrawRectangle(dropdown.x + 10, itemY, dropdown.width - 20, 2, METAL_ACCENT);
            itemY += 15;
            
            // Logout button
            Rectangle logoutBtn = {dropdown.x + 10, itemY, dropdown.width - 20, 40};
            bool logoutHover = CheckCollisionPointRec(GetMousePosition(), logoutBtn);
            DrawRectangleRounded(logoutBtn, 0.1f, 6, logoutHover ? BUTTON_RED : METAL_BG);
            DrawTextCustom("Terminar Sessao", dropdown.x + 20, itemY + 11, 18, logoutHover ? TEXT_WHITE : BUTTON_RED);
            
            if(logoutHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentUser = nullptr;
                currentUsername = "";
                isLoggedIn = false;
                showUserProfile = false;
                toastMessage = "Sessao terminada!";
                toastTimer = 2.0f;
            }
            
            // Close dropdown when clicking outside
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !CheckCollisionPointRec(GetMousePosition(), dropdown)) {
                showUserProfile = false;
            }
        }
        
        // WISHLIST MODAL
        if(showWishlist && isLoggedIn && currentUser != nullptr) {
            // Dark overlay
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.75f));
            
            // Consume all clicks to prevent click-through
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // Click consumed - prevents any background interaction
            }
            
            // Modal window
            float modalWidth = 900;
            float modalHeight = 650;
            Rectangle modal = {(float)screenWidth/2 - modalWidth/2, (float)screenHeight/2 - modalHeight/2, modalWidth, modalHeight};
            
            DrawRectangle(modal.x + 4, modal.y + 4, modal.width, modal.height, Fade(SHADOW_COLOR, 0.5f));
            DrawRectangleRounded(modal, 0.03f, 8, METAL_PANEL);
            DrawRectangleLinesEx(modal, 3, BUTTON_BLUE);
            
            // Close button
            Rectangle btnClose = {modal.x + modal.width - 45, modal.y + 10, 35, 35};
            if(DrawButton("X", btnClose, BUTTON_RED)) {
                showWishlist = false;
            }
            
            // Title
            char titleBuf[64];
            snprintf(titleBuf, sizeof(titleBuf), "Lista de Desejos (%d itens)", (int)currentUser->wishlist.size());
            DrawTextWithShadow(titleBuf, modal.x + 30, modal.y + 25, 32, TEXT_WHITE);
            
            // Wishlist items
            float itemY = modal.y + 85;
            float itemHeight = 120;
            int displayedItems = 0;
            
            if(currentUser->wishlist.empty()) {
                DrawTextCustom("A sua lista de desejos esta vazia!", modal.x + modalWidth/2 - 150, modal.y + modalHeight/2 - 20, 20, TEXT_GRAY);
                DrawTextCustom("Adicione produtos clicando no coracao @ nas fichas dos produtos.", modal.x + 150, modal.y + modalHeight/2 + 10, 16, TEXT_GRAY);
            } else {
                for(size_t wi = 0; wi < currentUser->wishlist.size() && displayedItems < 4; wi++) {
                    int productId = currentUser->wishlist[wi];
                    
                    // Find product
                    auto it = std::find_if(products.begin(), products.end(), [productId](const Product& p) {
                        return p.id == productId;
                    });
                    
                    if(it == products.end()) continue; // Product not found
                    
                    const Product& p = *it;
                    displayedItems++;
                    
                    // Item card
                    Rectangle itemCard = {modal.x + 30, itemY, modalWidth - 60, itemHeight};
                    bool itemHover = CheckCollisionPointRec(GetMousePosition(), itemCard);
                    DrawRectangleRounded(itemCard, 0.05f, 8, itemHover ? CARD_HOVER : METAL_BG);
                    DrawRectangleLinesEx(itemCard, 2, itemHover ? BUTTON_BLUE : METAL_ACCENT);
                    
                    // Product image
                    Texture2D img = GetProductImage(p.imagePath);
                    Rectangle imgRect = {itemCard.x + 15, itemCard.y + 10, 100, 100};
                    DrawTexturePro(img, 
                                  {0, 0, (float)img.width, (float)img.height},
                                  imgRect,
                                  {0, 0}, 0, WHITE);
                    DrawRectangleLinesEx(imgRect, 1, METAL_ACCENT);
                    
                    // Product info
                    float infoX = itemCard.x + 130;
                    float infoY = itemCard.y + 15;
                    
                    DrawTextWithShadow(p.name.c_str(), infoX, infoY, 22, TEXT_WHITE);
                    infoY += 30;
                    
                    DrawTextCustom(p.desc.c_str(), infoX, infoY, 16, TEXT_GRAY);
                    infoY += 25;
                    
                    // Price
                    char priceBuf[32];
                    float displayPrice = p.price;
                    if(p.isOnDiscount && p.discountPercent > 0) {
                        displayPrice = p.price * (1.0f - p.discountPercent / 100.0f);
                    }
                    snprintf(priceBuf, sizeof(priceBuf), "EUR %.2f", displayPrice);
                    DrawTextWithShadow(priceBuf, infoX, infoY, 24, GOLD);
                    
                    // Stock status
                    const char* stockText = p.inStock ? "Em Stock" : "ESGOTADO";
                    Color stockColor = p.inStock ? SUCCESS_GREEN : BUTTON_RED;
                    DrawTextCustom(stockText, infoX + 150, infoY + 3, 18, stockColor);
                    
                    // Remove from wishlist button
                    Rectangle btnRemove = {itemCard.x + itemCard.width - 170, itemCard.y + 15, 150, 40};
                    if(DrawButton("Remover", btnRemove, BUTTON_RED)) {
                        currentUser->wishlist.erase(
                            std::remove(currentUser->wishlist.begin(), currentUser->wishlist.end(), productId),
                            currentUser->wishlist.end()
                        );
                        SaveUsers(users);
                        toastMessage = "Removido da lista de desejos!";
                        toastTimer = 2.0f;
                    }
                    
                    // Add to cart button (if in stock)
                    if(p.inStock) {
                        Rectangle btnAddCart = {itemCard.x + itemCard.width - 170, itemCard.y + 65, 150, 40};
                        if(DrawButton("Adicionar ao Carrinho", btnAddCart, SUCCESS_GREEN)) {
                            auto cartIt = std::find_if(cart.begin(), cart.end(), [&p](const CartItem& item) {
                                return item.product.id == p.id;
                            });
                            if(cartIt != cart.end()) {
                                cartIt->qty += 1;
                                toastMessage = "Quantidade atualizada!";
                            } else {
                                cart.push_back({p, 1});
                                toastMessage = "Adicionado ao carrinho!";
                            }
                            toastTimer = 2.0f;
                        }
                    }
                    
                    itemY += itemHeight + 10;
                }
                
                // Show message if there are more items
                if(currentUser->wishlist.size() > 4) {
                    char moreBuf[64];
                    snprintf(moreBuf, sizeof(moreBuf), "+ %d itens adicionais", (int)(currentUser->wishlist.size() - 4));
                    DrawTextCustom(moreBuf, modal.x + modalWidth/2 - 80, itemY, 18, TEXT_GRAY);
                }
            }
        }
        
        // CHECKOUT MODAL
        if(showCheckout) {
            // Dark overlay
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.75f));
            
            // Consume all clicks to prevent click-through
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // Click consumed - prevents any background interaction
            }
            
            static std::string shippingName = "";
            static std::string shippingAddress = "";
            static std::string shippingCity = "";
            static std::string shippingPostal = "";
            static std::string shippingPhone = "";
            static int selectedPayment = 0; // 0=MB Way, 1=Card, 2=PayPal
            static int activeField = 0; // Which field is being typed in
            static std::string checkoutMessage = "";
            static Color checkoutMessageColor = BUTTON_RED;
            
            // Modal window
            float modalWidth = 700;
            float modalHeight = 650;
            Rectangle modal = {(float)screenWidth/2 - modalWidth/2, (float)screenHeight/2 - modalHeight/2, modalWidth, modalHeight};
            
            DrawRectangle(modal.x + 4, modal.y + 4, modal.width, modal.height, Fade(SHADOW_COLOR, 0.5f));
            DrawRectangleRounded(modal, 0.03f, 8, METAL_PANEL);
            DrawRectangleLinesEx(modal, 3, BUTTON_BLUE);
            
            // Close button
            Rectangle btnClose = {modal.x + modal.width - 45, modal.y + 10, 35, 35};
            if(DrawButton("X", btnClose, BUTTON_RED)) {
                showCheckout = false;
                checkoutMessage = "";
            }
            
            // Title
            DrawTextWithShadow("Finalizar Compra", modal.x + 30, modal.y + 25, 32, TEXT_WHITE);
            
            // Calculate total
            float total = 0.0f;
            for(const auto& item : cart) {
                float price = item.product.price;
                if(item.product.isOnDiscount && item.product.discountPercent > 0) {
                    price *= (1.0f - item.product.discountPercent / 100.0f);
                }
                total += price * item.qty;
            }
            
            char totalBuf[64];
            snprintf(totalBuf, sizeof(totalBuf), "Total: EUR %.2f", total);
            DrawTextWithShadow(totalBuf, modal.x + modalWidth - 220, modal.y + 30, 24, GOLD);
            
            float fieldY = modal.y + 85;
            float fieldHeight = 45;
            float fieldSpacing = 65;
            
            // Shipping form
            DrawTextCustom("Dados de Envio:", modal.x + 30, fieldY, 20, TEXT_WHITE);
            fieldY += 35;
            
            // Name field
            Rectangle nameField = {modal.x + 30, fieldY, modalWidth - 60, fieldHeight};
            DrawInputField(nameField, "Nome Completo", shippingName, activeField == 0, 50);
            if(CheckCollisionPointRec(GetMousePosition(), nameField) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                activeField = 0;
            }
            if(activeField == 0) {
                int key = GetCharPressed();
                while(key > 0) {
                    if((key >= 32 && key <= 125) || (key >= 128 && key <= 255)) {
                        if(shippingName.length() < 50) shippingName += (char)key;
                    }
                    key = GetCharPressed();
                }
                if(IsKeyPressed(KEY_BACKSPACE) && shippingName.length() > 0) {
                    shippingName.pop_back();
                }
                if(IsKeyPressed(KEY_TAB)) activeField = 1;
            }
            fieldY += fieldSpacing;
            
            // Address field
            Rectangle addressField = {modal.x + 30, fieldY, modalWidth - 60, fieldHeight};
            DrawInputField(addressField, "Morada", shippingAddress, activeField == 1, 80);
            if(CheckCollisionPointRec(GetMousePosition(), addressField) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                activeField = 1;
            }
            if(activeField == 1) {
                int key = GetCharPressed();
                while(key > 0) {
                    if((key >= 32 && key <= 125) || (key >= 128 && key <= 255)) {
                        if(shippingAddress.length() < 80) shippingAddress += (char)key;
                    }
                    key = GetCharPressed();
                }
                if(IsKeyPressed(KEY_BACKSPACE) && shippingAddress.length() > 0) {
                    shippingAddress.pop_back();
                }
                if(IsKeyPressed(KEY_TAB)) activeField = 2;
            }
            fieldY += fieldSpacing;
            
            // City and Postal in same row
            Rectangle cityField = {modal.x + 30, fieldY, (modalWidth - 80) / 2, fieldHeight};
            DrawInputField(cityField, "Cidade", shippingCity, activeField == 2, 30);
            if(CheckCollisionPointRec(GetMousePosition(), cityField) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                activeField = 2;
            }
            if(activeField == 2) {
                int key = GetCharPressed();
                while(key > 0) {
                    if((key >= 32 && key <= 125) || (key >= 128 && key <= 255)) {
                        if(shippingCity.length() < 30) shippingCity += (char)key;
                    }
                    key = GetCharPressed();
                }
                if(IsKeyPressed(KEY_BACKSPACE) && shippingCity.length() > 0) {
                    shippingCity.pop_back();
                }
                if(IsKeyPressed(KEY_TAB)) activeField = 3;
            }
            
            Rectangle postalField = {modal.x + 30 + (modalWidth - 80) / 2 + 20, fieldY, (modalWidth - 80) / 2, fieldHeight};
            DrawInputField(postalField, "Codigo Postal", shippingPostal, activeField == 3, 15);
            if(CheckCollisionPointRec(GetMousePosition(), postalField) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                activeField = 3;
            }
            if(activeField == 3) {
                int key = GetCharPressed();
                while(key > 0) {
                    if(key >= '0' && key <= '9' && shippingPostal.length() < 15) {
                        shippingPostal += (char)key;
                    } else if(key == '-' && shippingPostal.length() < 15) {
                        shippingPostal += (char)key;
                    }
                    key = GetCharPressed();
                }
                if(IsKeyPressed(KEY_BACKSPACE) && shippingPostal.length() > 0) {
                    shippingPostal.pop_back();
                }
                if(IsKeyPressed(KEY_TAB)) activeField = 4;
            }
            fieldY += fieldSpacing;
            
            // Phone field
            Rectangle phoneField = {modal.x + 30, fieldY, modalWidth - 60, fieldHeight};
            DrawInputField(phoneField, "Telefone", shippingPhone, activeField == 4, 20);
            if(CheckCollisionPointRec(GetMousePosition(), phoneField) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                activeField = 4;
            }
            if(activeField == 4) {
                int key = GetCharPressed();
                while(key > 0) {
                    if((key >= '0' && key <= '9') || key == '+' || key == ' ') {
                        if(shippingPhone.length() < 20) shippingPhone += (char)key;
                    }
                    key = GetCharPressed();
                }
                if(IsKeyPressed(KEY_BACKSPACE) && shippingPhone.length() > 0) {
                    shippingPhone.pop_back();
                }
            }
            fieldY += fieldSpacing + 10;
            
            // Payment method selection
            DrawTextCustom("Metodo de Pagamento:", modal.x + 30, fieldY, 20, TEXT_WHITE);
            fieldY += 35;
            
            const char* paymentMethods[] = {"MB Way", "Cartao de Credito", "PayPal"};
            for(int pm = 0; pm < 3; pm++) {
                Rectangle pmBtn = {modal.x + 30 + pm * 210.0f, fieldY, 200, 40};
                Color pmColor = (selectedPayment == pm) ? SUCCESS_GREEN : METAL_ACCENT;
                bool pmHover = CheckCollisionPointRec(GetMousePosition(), pmBtn);
                if(pmHover) pmColor = ColorBrightness(pmColor, 0.2f);
                
                DrawRectangleRounded(pmBtn, 0.1f, 6, pmColor);
                int textW = MeasureTextCustom(paymentMethods[pm], 18);
                DrawTextCustom(paymentMethods[pm], pmBtn.x + (pmBtn.width - textW) / 2, pmBtn.y + 11, 18, TEXT_WHITE);
                
                if(pmHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    selectedPayment = pm;
                }
            }
            fieldY += 60;
            
            // Error/success message
            if(!checkoutMessage.empty()) {
                DrawTextCustom(checkoutMessage.c_str(), modal.x + 30, fieldY, 16, checkoutMessageColor);
                fieldY += 30;
            }
            
            // Confirm order button
            Rectangle btnConfirm = {modal.x + modalWidth - 220, modal.y + modalHeight - 60, 190, 45};
            if(DrawButton("Confirmar Pedido", btnConfirm, SUCCESS_GREEN)) {
                // Validate fields
                if(shippingName.empty() || shippingAddress.empty() || shippingCity.empty() || 
                   shippingPostal.empty() || shippingPhone.empty()) {
                    checkoutMessage = "Preencha todos os campos!";
                    checkoutMessageColor = BUTTON_RED;
                } else {
                    // Create order
                    Order newOrder;
                    newOrder.id = orders.empty() ? 1 : orders.back().id + 1;
                    newOrder.username = currentUsername;
                    newOrder.total = total;
                    newOrder.status = OrderStatus::PENDING;
                    
                    // Get current date/time
                    time_t now = time(0);
                    tm* ltm = localtime(&now);
                    std::stringstream dateStream;
                    dateStream << std::setfill('0') 
                              << std::setw(4) << (1900 + ltm->tm_year) << "-"
                              << std::setw(2) << (1 + ltm->tm_mon) << "-"
                              << std::setw(2) << ltm->tm_mday;
                    newOrder.date = dateStream.str();
                    
                    newOrder.shippingAddress = shippingName + ", " + shippingAddress + ", " + 
                                               shippingCity + ", " + shippingPostal + ", " + shippingPhone;
                    newOrder.paymentMethod = paymentMethods[selectedPayment];
                    
                    // Add order items (simplified - in production would store items properly)
                    for(const auto& item : cart) {
                        OrderItem oi;
                        oi.product = item.product;
                        oi.qty = item.qty;
                        oi.priceAtPurchase = item.product.price;
                        if(item.product.isOnDiscount && item.product.discountPercent > 0) {
                            oi.priceAtPurchase *= (1.0f - item.product.discountPercent / 100.0f);
                        }
                        newOrder.items.push_back(oi);
                    }
                    
                    orders.push_back(newOrder);
                    currentUser->orderHistory.push_back(newOrder.id);
                    
                    SaveOrders(orders);
                    SaveUsers(users);
                    
                    // Clear cart and close modals
                    cart.clear();
                    showCheckout = false;
                    toastMessage = "Pedido realizado com sucesso!";
                    toastTimer = 3.0f;
                    
                    // Reset form
                    shippingName = "";
                    shippingAddress = "";
                    shippingCity = "";
                    shippingPostal = "";
                    shippingPhone = "";
                    selectedPayment = 0;
                    activeField = 0;
                    checkoutMessage = "";
                }
            }
            
            // Cancel button
            Rectangle btnCancel = {modal.x + 30, modal.y + modalHeight - 60, 140, 45};
            if(DrawButton("Cancelar", btnCancel, BUTTON_RED)) {
                showCheckout = false;
                checkoutMessage = "";
            }
        }
        
        // ORDER HISTORY MODAL
        if(showOrderHistory && isLoggedIn && currentUser != nullptr) {
            // Dark overlay
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.75f));
            
            // Consume all clicks to prevent click-through
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // Click consumed - prevents any background interaction
            }
            
            // Modal window
            float modalWidth = 950;
            float modalHeight = 700;
            Rectangle modal = {(float)screenWidth/2 - modalWidth/2, (float)screenHeight/2 - modalHeight/2, modalWidth, modalHeight};
            
            DrawRectangle(modal.x + 4, modal.y + 4, modal.width, modal.height, Fade(SHADOW_COLOR, 0.5f));
            DrawRectangleRounded(modal, 0.03f, 8, METAL_PANEL);
            DrawRectangleLinesEx(modal, 3, BUTTON_BLUE);
            
            // Close button
            Rectangle btnClose = {modal.x + modal.width - 45, modal.y + 10, 35, 35};
            if(DrawButton("X", btnClose, BUTTON_RED)) {
                showOrderHistory = false;
            }
            
            // Title
            char titleBuf[64];
            snprintf(titleBuf, sizeof(titleBuf), "Historico de Pedidos (%d)", (int)currentUser->orderHistory.size());
            DrawTextWithShadow(titleBuf, modal.x + 30, modal.y + 25, 32, TEXT_WHITE);
            
            // Order items
            float itemY = modal.y + 85;
            float itemHeight = 160;
            int displayedOrders = 0;
            
            if(currentUser->orderHistory.empty()) {
                DrawTextCustom("Nao tem pedidos no historico.", modal.x + modalWidth/2 - 150, modal.y + modalHeight/2 - 20, 20, TEXT_GRAY);
                DrawTextCustom("Faca compras para ver os seus pedidos aqui.", modal.x + modalWidth/2 - 200, modal.y + modalHeight/2 + 10, 16, TEXT_GRAY);
            } else {
                // Show orders in reverse (newest first)
                for(int oi = currentUser->orderHistory.size() - 1; oi >= 0 && displayedOrders < 3; oi--) {
                    int orderId = currentUser->orderHistory[oi];
                    
                    // Find order
                    auto it = std::find_if(orders.begin(), orders.end(), [orderId](const Order& o) {
                        return o.id == orderId;
                    });
                    
                    if(it == orders.end()) continue; // Order not found
                    
                    const Order& order = *it;
                    displayedOrders++;
                    
                    // Order card
                    Rectangle orderCard = {modal.x + 30, itemY, modalWidth - 60, itemHeight};
                    bool orderHover = CheckCollisionPointRec(GetMousePosition(), orderCard);
                    DrawRectangleRounded(orderCard, 0.05f, 8, orderHover ? CARD_HOVER : METAL_BG);
                    DrawRectangleLinesEx(orderCard, 2, orderHover ? BUTTON_BLUE : METAL_ACCENT);
                    
                    // Order info
                    float infoX = orderCard.x + 20;
                    float infoY = orderCard.y + 15;
                    
                    // Order number and date
                    char orderNumBuf[64];
                    snprintf(orderNumBuf, sizeof(orderNumBuf), "Pedido #%d", order.id);
                    DrawTextWithShadow(orderNumBuf, infoX, infoY, 24, TEXT_WHITE);
                    
                    DrawTextCustom(order.date.c_str(), infoX + 150, infoY + 5, 18, TEXT_GRAY);
                    infoY += 35;
                    
                    // Status badge
                    std::string statusText = OrderStatusToString(order.status);
                    Color statusColor;
                    switch(order.status) {
                        case OrderStatus::PENDING: statusColor = GOLD; break;
                        case OrderStatus::PROCESSING: statusColor = BUTTON_BLUE; break;
                        case OrderStatus::SHIPPED: statusColor = PURPLE; break;
                        case OrderStatus::DELIVERED: statusColor = SUCCESS_GREEN; break;
                        case OrderStatus::CANCELLED: statusColor = BUTTON_RED; break;
                        default: statusColor = METAL_ACCENT;
                    }
                    
                    int statusW = MeasureTextCustom(statusText.c_str(), 16);
                    Rectangle statusBadge = {infoX, infoY, (float)statusW + 20, 28};
                    DrawRectangleRounded(statusBadge, 0.3f, 6, Fade(statusColor, 0.3f));
                    DrawTextCustom(statusText.c_str(), statusBadge.x + 10, statusBadge.y + 6, 16, statusColor);
                    infoY += 40;
                    
                    // Items summary
                    DrawTextCustom("Itens:", infoX, infoY, 18, TEXT_WHITE);
                    infoY += 25;
                    
                    int itemsShown = 0;
                    for(const auto& item : order.items) {
                        if(itemsShown >= 2) {
                            char moreBuf[32];
                            snprintf(moreBuf, sizeof(moreBuf), "+ %d itens adicionais", (int)(order.items.size() - 2));
                            DrawTextCustom(moreBuf, infoX + 20, infoY, 14, TEXT_GRAY);
                            break;
                        }
                        
                        char itemBuf[128];
                        snprintf(itemBuf, sizeof(itemBuf), "  • %s x%d", item.product.name.c_str(), item.qty);
                        DrawTextCustom(itemBuf, infoX + 20, infoY, 14, TEXT_GRAY);
                        infoY += 20;
                        itemsShown++;
                    }
                    infoY = orderCard.y + 15;
                    
                    // Total (right side)
                    char totalBuf[32];
                    snprintf(totalBuf, sizeof(totalBuf), "EUR %.2f", order.total);
                    int totalW = MeasureTextCustom(totalBuf, 28);
                    DrawTextWithShadow(totalBuf, orderCard.x + orderCard.width - totalW - 20, infoY, 28, GOLD);
                    
                    // Payment method
                    DrawTextCustom(order.paymentMethod.c_str(), orderCard.x + orderCard.width - 200, infoY + 45, 16, TEXT_GRAY);
                    
                    // Shipping address
                    DrawTextCustom("Envio:", orderCard.x + orderCard.width - 350, infoY + 75, 14, TEXT_GRAY);
                    
                    // Word wrap shipping address
                    std::string addr = order.shippingAddress;
                    if(addr.length() > 50) {
                        addr = addr.substr(0, 47) + "...";
                    }
                    DrawTextCustom(addr.c_str(), orderCard.x + orderCard.width - 350, infoY + 95, 13, TEXT_GRAY);
                    
                    itemY += itemHeight + 15;
                }
                
                // Show message if there are more orders
                if(currentUser->orderHistory.size() > 3) {
                    char moreBuf[64];
                    snprintf(moreBuf, sizeof(moreBuf), "+ %d pedidos adicionais", (int)(currentUser->orderHistory.size() - 3));
                    DrawTextCustom(moreBuf, modal.x + modalWidth/2 - 100, itemY, 18, TEXT_GRAY);
                }
            }
        }
        
        // Product Details Modal
        if(showProductDetails && selectedProductForDetails >= 0 && selectedProductForDetails < (int)products.size()){
            // Dark overlay
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.7f));
            
            // Consume all clicks to prevent click-through
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // Click consumed - prevents any background interaction
            }
            
            // Modal window
            Rectangle modal = {(float)screenWidth/2 - 400, (float)screenHeight/2 - 300, 800, 600};
            DrawRectangle(modal.x, modal.y, modal.width, modal.height, METAL_BG);
            DrawRectangleRounded(modal, 0.02f, 8, METAL_PANEL);
            DrawRectangleLinesEx(modal, 3, BUTTON_BLUE);
            
            const Product& p = products[selectedProductForDetails];
            
            // Close button
            Rectangle btnClose = {modal.x + modal.width - 45, modal.y + 10, 35, 35};
            if(DrawButton("X", btnClose, BUTTON_RED)) {
                showProductDetails = false;
            }
            
            // Title
            DrawTextWithShadow("Detalhes do Produto", modal.x + 20, modal.y + 20, 28, TEXT_WHITE);
            
            // Product image (larger)
            Texture2D detailImg = GetProductImage(p.imagePath);
            Rectangle detailImgRect = {modal.x + 30, modal.y + 70, 300, 300};
            DrawTexturePro(detailImg, 
                          {0, 0, (float)detailImg.width, (float)detailImg.height},
                          detailImgRect,
                          {0, 0}, 0, WHITE);
            DrawRectangleLinesEx(detailImgRect, 2, METAL_ACCENT);
            
            // Product info (right side)
            float infoX = modal.x + 350;
            float infoY = modal.y + 70;
            
            // Product name
            DrawTextWithShadow(p.name.c_str(), infoX, infoY, 24, TEXT_WHITE);
            infoY += 35;
            
            // Category
            char catBuf[64];
            snprintf(catBuf, sizeof(catBuf), "Categoria: %s", CategoryToString(p.category).c_str());
            DrawTextCustom(catBuf, infoX, infoY, 18, METAL_BRONZE);
            infoY += 30;
            
            // Stock status
            const char* stockText = p.inStock ? "Em Stock" : "ESGOTADO";
            Color stockColor = p.inStock ? SUCCESS_GREEN : BUTTON_RED;
            DrawTextCustom(stockText, infoX, infoY, 18, stockColor);
            infoY += 30;
            
            // Rating with stars
            DrawTextCustom("Avaliacao:", infoX, infoY, 18, TEXT_WHITE);
            infoY += 25;
            for(int s = 0; s < 5; s++) {
                Color starColor = (s < (int)p.rating) ? GOLD : METAL_ACCENT;
                if(s < (int)p.rating && (p.rating - s) >= 0.5f) {
                    starColor = GOLD;
                } else if(s >= (int)p.rating) {
                    starColor = METAL_ACCENT;
                }
                DrawTextCustom("*", infoX + s * 25, infoY, 24, starColor);
            }
            char ratingBuf[16];
            snprintf(ratingBuf, sizeof(ratingBuf), "%.1f/5.0", p.rating);
            DrawTextCustom(ratingBuf, infoX + 140, infoY + 3, 18, TEXT_GRAY);
            infoY += 40;
            
            // Price
            float displayPrice = p.price;
            if(p.isOnDiscount && p.discountPercent > 0) {
                displayPrice = p.price * (1.0f - p.discountPercent / 100.0f);
                
                // Discount badge
                char discountBuf[32];
                snprintf(discountBuf, sizeof(discountBuf), "DESCONTO -%0.f%%", p.discountPercent);
                DrawTextCustom(discountBuf, infoX, infoY, 20, BUTTON_RED);
                infoY += 30;
                
                // Original price (crossed out)
                char originalPriceBuf[32];
                snprintf(originalPriceBuf, sizeof(originalPriceBuf), "EUR %.2f", p.price);
                DrawTextCustom(originalPriceBuf, infoX, infoY, 18, TEXT_GRAY);
                int origW = MeasureTextCustom(originalPriceBuf, 18);
                DrawLine(infoX, infoY + 10, infoX + origW, infoY + 10, TEXT_GRAY);
                infoY += 25;
            }
            
            char priceBuf[32];
            snprintf(priceBuf, sizeof(priceBuf), "EUR %.2f", displayPrice);
            DrawTextWithShadow(priceBuf, infoX, infoY, 32, GOLD);
            infoY += 50;
            
            // Description
            DrawTextCustom("Descricao:", infoX, infoY, 18, TEXT_WHITE);
            infoY += 25;
            
            // Word wrap description
            std::string desc = p.desc;
            int maxWidth = 400;
            int lineHeight = 20;
            size_t pos = 0;
            while(pos < desc.length() && infoY < modal.y + modal.height - 80) {
                size_t endPos = pos;
                int currentWidth = 0;
                while(endPos < desc.length()) {
                    char c = desc[endPos];
                    currentWidth += 8; // Approximate character width
                    if(currentWidth > maxWidth) break;
                    if(c == '\n') {
                        endPos++;
                        break;
                    }
                    endPos++;
                }
                
                // Back up to last space if we broke mid-word
                if(endPos < desc.length() && desc[endPos] != ' ' && desc[endPos] != '\n') {
                    size_t lastSpace = desc.rfind(' ', endPos);
                    if(lastSpace > pos) endPos = lastSpace + 1;
                }
                
                std::string line = desc.substr(pos, endPos - pos);
                DrawTextCustom(line.c_str(), infoX, infoY, 16, TEXT_GRAY);
                infoY += lineHeight;
                pos = endPos;
            }
            
            // Reviews section (at bottom of modal, above add to cart button)
            float reviewsY = modal.y + modal.height - 250;
            DrawLine(modal.x + 20, reviewsY - 10, modal.x + modal.width - 20, reviewsY - 10, METAL_ACCENT);
            
            DrawTextWithShadow("Avaliacoes", modal.x + 30, reviewsY, 22, TEXT_WHITE);
            reviewsY += 35;
            
            // Get reviews for this product
            std::vector<Review> productReviews;
            for(const auto& r : reviews) {
                if(r.productId == p.id) {
                    productReviews.push_back(r);
                }
            }
            
            if(productReviews.empty()) {
                DrawTextCustom("Sem avaliacoes ainda. Seja o primeiro!", modal.x + 40, reviewsY, 16, TEXT_GRAY);
            } else {
                // Show latest review only (to save space)
                const Review& latestReview = productReviews.back();
                
                Rectangle reviewCard = {modal.x + 30, reviewsY, modal.width - 360, 80};
                DrawRectangleRounded(reviewCard, 0.05f, 6, METAL_BG);
                DrawRectangleLinesEx(reviewCard, 1, METAL_ACCENT);
                
                // Username and rating
                DrawTextCustom(latestReview.username.c_str(), reviewCard.x + 15, reviewCard.y + 10, 16, TEXT_WHITE);
                
                // Stars
                for(int s = 0; s < 5; s++) {
                    Color starColor = (s < (int)latestReview.rating) ? GOLD : METAL_ACCENT;
                    DrawTextCustom("*", reviewCard.x + 15 + s * 18, reviewCard.y + 30, 20, starColor);
                }
                
                DrawTextCustom(latestReview.date.c_str(), reviewCard.x + 15, reviewCard.y + 55, 13, TEXT_GRAY);
                
                // Comment (truncated if too long)
                std::string comment = latestReview.comment;
                if(comment.length() > 40) {
                    comment = comment.substr(0, 37) + "...";
                }
                DrawTextCustom(comment.c_str(), reviewCard.x + 150, reviewCard.y + 35, 15, TEXT_GRAY);
                
                // Show count if more reviews
                if(productReviews.size() > 1) {
                    char moreBuf[32];
                    snprintf(moreBuf, sizeof(moreBuf), "+ %d mais", (int)(productReviews.size() - 1));
                    DrawTextCustom(moreBuf, reviewCard.x + reviewCard.width - 80, reviewCard.y + 10, 14, BUTTON_BLUE);
                }
            }
            
            // Add review form (only if logged in)
            if(isLoggedIn && currentUser != nullptr) {
                static int newReviewRating = 5;
                static std::string newReviewComment = "";
                static bool typingReview = false;
                
                float formX = modal.x + modal.width - 300;
                float formY = reviewsY;
                
                DrawTextCustom("Sua Avaliacao:", formX, formY, 16, TEXT_WHITE);
                formY += 25;
                
                // Star rating selector
                for(int s = 0; s < 5; s++) {
                    Rectangle starBtn = {formX + s * 35.0f, formY, 30, 30};
                    bool starHover = CheckCollisionPointRec(GetMousePosition(), starBtn);
                    Color starColor = (s < newReviewRating) ? GOLD : METAL_ACCENT;
                    if(starHover) starColor = ColorBrightness(starColor, 0.3f);
                    
                    DrawTextCustom("*", starBtn.x, starBtn.y, 28, starColor);
                    
                    if(starHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        newReviewRating = s + 1;
                    }
                }
                formY += 40;
                
                // Comment box
                Rectangle commentBox = {formX, formY, 260, 60};
                DrawRectangleRounded(commentBox, 0.1f, 6, typingReview ? METAL_ACCENT : METAL_BG);
                DrawRectangleLinesEx(commentBox, 2, typingReview ? BUTTON_BLUE : METAL_ACCENT);
                
                BeginScissorMode(commentBox.x + 5, commentBox.y + 5, commentBox.width - 10, commentBox.height - 10);
                DrawTextCustom(newReviewComment.c_str(), commentBox.x + 10, commentBox.y + 10, 14, TEXT_WHITE);
                EndScissorMode();
                
                if(CheckCollisionPointRec(GetMousePosition(), commentBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    typingReview = true;
                }
                
                if(typingReview) {
                    int key = GetCharPressed();
                    while(key > 0) {
                        if((key >= 32 && key <= 125) || (key >= 128 && key <= 255)) {
                            if(newReviewComment.length() < 200) newReviewComment += (char)key;
                        }
                        key = GetCharPressed();
                    }
                    if(IsKeyPressed(KEY_BACKSPACE) && newReviewComment.length() > 0) {
                        newReviewComment.pop_back();
                    }
                }
                formY += 70;
                
                // Submit review button
                Rectangle btnSubmitReview = {formX, formY, 130, 35};
                if(DrawButton("Enviar", btnSubmitReview, SUCCESS_GREEN)) {
                    if(newReviewComment.empty()) {
                        toastMessage = "Escreva um comentario!";
                        toastTimer = 2.0f;
                    } else {
                        // Create new review
                        Review newReview;
                        newReview.id = reviews.empty() ? 1 : reviews.back().id + 1;
                        newReview.productId = p.id;
                        newReview.username = currentUsername;
                        newReview.rating = newReviewRating;
                        newReview.comment = newReviewComment;
                        
                        // Get current date
                        time_t now = time(0);
                        tm* ltm = localtime(&now);
                        std::stringstream dateStream;
                        dateStream << std::setfill('0') 
                                  << std::setw(4) << (1900 + ltm->tm_year) << "-"
                                  << std::setw(2) << (1 + ltm->tm_mon) << "-"
                                  << std::setw(2) << ltm->tm_mday;
                        newReview.date = dateStream.str();
                        
                        reviews.push_back(newReview);
                        SaveReviews(reviews);
                        
                        // Update product average rating
                        float totalRating = 0.0f;
                        int count = 0;
                        for(const auto& r : reviews) {
                            if(r.productId == p.id) {
                                totalRating += r.rating;
                                count++;
                            }
                        }
                        products[selectedProductForDetails].rating = count > 0 ? totalRating / count : 0.0f;
                        SaveProducts(products);
                        
                        toastMessage = "Avaliacao enviada!";
                        toastTimer = 2.0f;
                        newReviewComment = "";
                        newReviewRating = 5;
                        typingReview = false;
                    }
                }
            }
            
            // Add to cart button (if in stock)
            if(p.inStock) {
                Rectangle btnAddToCart = {modal.x + modal.width - 220, modal.y + modal.height - 60, 180, 45};
                if(DrawButton("Adicionar ao Carrinho", btnAddToCart, SUCCESS_GREEN)) {
                    // Add to cart logic
                    auto it = std::find_if(cart.begin(), cart.end(), [&](const CartItem& item) {
                        return item.product.id == p.id;
                    });
                    if(it != cart.end()) {
                        it->qty += 1;
                        toastMessage = "Quantidade atualizada!";
                    } else {
                        cart.push_back({p, 1});
                        toastMessage = "Adicionado ao carrinho!";
                    }
                    toastTimer = 2.0f;
                    showProductDetails = false;
                }
            }
        }
        
        // PAINEL ADMIN
        
        if(showAdmin){
            ShowAdminPanel(screenWidth, screenHeight, products, showAdmin);
        }

        EndDrawing();
    }
}

// --- Helper: Get timestamp string ---
std::string GetTimestamp() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    std::stringstream ss;
    ss << std::setfill('0') 
       << std::setw(4) << (1900 + ltm->tm_year) << "-"
       << std::setw(2) << (1 + ltm->tm_mon) << "-"
       << std::setw(2) << ltm->tm_mday << "_"
       << std::setw(2) << ltm->tm_hour << "-"
       << std::setw(2) << ltm->tm_min << "-"
       << std::setw(2) << ltm->tm_sec;
    return ss.str();
}

// --- Helper: Ensure directory exists ---
void EnsureDirectory(const std::string& path) {
    _mkdir(path.c_str()); // Create directory (ignores error if already exists)
}

// --- Export to CSV ---
void ExportToCSV(const std::vector<Product>& products) {
    EnsureDirectory("exports");
    
    std::string filename = "exports/products_" + GetTimestamp() + ".csv";
    std::ofstream file(filename);
    
    if(!file.is_open()) {
        LogAction("EXPORT_CSV", "ERRO: Nao foi possivel criar " + filename);
        return;
    }
    
    // CSV Header
    file << "ID,Name,Description,Price,Color_R,Color_G,Color_B,Category\n";
    
    // CSV Data
    for(const auto& p : products) {
        file << p.id << ","
             << "\"" << p.name << "\","
             << "\"" << p.desc << "\","
             << p.price << ","
             << (int)p.color.r << ","
             << (int)p.color.g << ","
             << (int)p.color.b << ","
             << CategoryToString(p.category) << "\n";
    }
    
    file.close();
    LogAction("EXPORT_CSV", "[OK] Exportado: " + filename + " (" + std::to_string(products.size()) + " produtos)");
}

// --- Generate Report ---
void GenerateReport(const std::vector<Product>& products) {
    EnsureDirectory("reports");
    
    std::string filename = "reports/report_" + GetTimestamp() + ".txt";
    std::ofstream file(filename);
    
    if(!file.is_open()) {
        LogAction("REPORT", "ERRO: Nao foi possivel criar " + filename);
        return;
    }
    
    // Report Header
    file << "========================================\n";
    file << "       RELATORIO DE PRODUTOS\n";
    file << "========================================\n";
    file << "Data: " << GetTimestamp() << "\n\n";
    
    // Statistics
    file << "ESTATISTICAS GERAIS:\n";
    file << "--------------------\n";
    file << "Total de Produtos: " << products.size() << "\n\n";
    
    // Count by category
    std::map<ProductCategory, int> categoryCount;
    std::map<ProductCategory, float> categoryTotal;
    float totalValue = 0.0f;
    float minPrice = 99999.0f;
    float maxPrice = 0.0f;
    
    for(const auto& p : products) {
        categoryCount[p.category]++;
        categoryTotal[p.category] += p.price;
        totalValue += p.price;
        if(p.price < minPrice) minPrice = p.price;
        if(p.price > maxPrice) maxPrice = p.price;
    }
    
    file << "PRODUTOS POR CATEGORIA:\n";
    file << "----------------------\n";
    for(const auto& pair : categoryCount) {
        file << CategoryToString(pair.first) << ": " << pair.second 
             << " produtos (Total: EUR " << std::fixed << std::setprecision(2) 
             << categoryTotal[pair.first] << ")\n";
    }
    
    file << "\nPRECOS:\n";
    file << "-------\n";
    file << "Valor Total Inventario: EUR " << std::fixed << std::setprecision(2) << totalValue << "\n";
    file << "Preco Medio: EUR " << (products.empty() ? 0.0f : totalValue / products.size()) << "\n";
    file << "Preco Minimo: EUR " << minPrice << "\n";
    file << "Preco Maximo: EUR " << maxPrice << "\n\n";
    
    // Top 10 most expensive
    std::vector<Product> sorted = products;
    std::sort(sorted.begin(), sorted.end(), [](const Product& a, const Product& b) {
        return a.price > b.price;
    });
    
    file << "TOP 10 MAIS CAROS:\n";
    file << "------------------\n";
    int count = 0;
    for(const auto& p : sorted) {
        if(count++ >= 10) break;
        file << count << ". " << p.name << " - EUR " << std::fixed << std::setprecision(2) 
             << p.price << " (" << CategoryToString(p.category) << ")\n";
    }
    
    file << "\n========================================\n";
    file << "           FIM DO RELATORIO\n";
    file << "========================================\n";
    
    file.close();
    LogAction("REPORT", "[OK] Relatorio gerado: " + filename);
}

// --- Create Backup ---
void CreateBackup(const std::vector<Product>& products) {
    EnsureDirectory("backups");
    
    std::string timestamp = GetTimestamp();
    
    // Backup products
    std::string productsBackup = "backups/products_backup_" + timestamp + ".txt";
    std::ifstream sourceProducts("products.txt");
    std::ofstream destProducts(productsBackup);
    
    if(sourceProducts.is_open() && destProducts.is_open()) {
        destProducts << sourceProducts.rdbuf();
        sourceProducts.close();
        destProducts.close();
        LogAction("BACKUP", "[OK] Backup de produtos: " + productsBackup);
    } else {
        LogAction("BACKUP", "ERRO: Falha ao criar backup de produtos");
    }
    
    // Backup users
    std::string usersBackup = "backups/users_backup_" + timestamp + ".txt";
    std::ifstream sourceUsers("users.txt");
    std::ofstream destUsers(usersBackup);
    
    if(sourceUsers.is_open() && destUsers.is_open()) {
        destUsers << sourceUsers.rdbuf();
        sourceUsers.close();
        destUsers.close();
        LogAction("BACKUP", "[OK] Backup de usuarios: " + usersBackup);
    } else {
        LogAction("BACKUP", "ERRO: Falha ao criar backup de usuarios");
    }
}

// --- Log Action ---
void LogAction(const std::string& action, const std::string& details) {
    EnsureDirectory("logs");
    
    std::ofstream logFile("logs/actions.log", std::ios::app);
    
    if(!logFile.is_open()) return;
    
    time_t now = time(0);
    tm* ltm = localtime(&now);
    
    logFile << "["
            << std::setfill('0') << std::setw(4) << (1900 + ltm->tm_year) << "-"
            << std::setw(2) << (1 + ltm->tm_mon) << "-"
            << std::setw(2) << ltm->tm_mday << " "
            << std::setw(2) << ltm->tm_hour << ":"
            << std::setw(2) << ltm->tm_min << ":"
            << std::setw(2) << ltm->tm_sec
            << "] " << action << ": " << details << "\n";
    
    logFile.close();
}

} // end namespace techcore

// --- Modern Login Modal ---
bool RunLoginUI(int screenWidth, int screenHeight) {
    enum Mode { LOGIN, REGISTER }; Mode mode = LOGIN;
    std::string username, password, message;
    bool typingUser = true, typingPass = false;
    bool finished = false, logged = false;
    while (!finished && !WindowShouldClose()) {
        BeginDrawing();
        DrawRectangle(0,0,screenWidth,screenHeight, Fade(BLACK,0.78f));
        
        Rectangle modal{(float)(screenWidth/2-200), (float)(screenHeight/2-140), 400, 280};
        
        // Shadow
        DrawRectangle(modal.x + 4, modal.y + 4, modal.width, modal.height, Fade(SHADOW_COLOR, 0.5f));
        
        // Modal background
        DrawRectangleRounded(modal, 0.03f, 8, METAL_PANEL);
        DrawRectangleLinesEx(modal, 2.0f, METAL_ACCENT);
        
        // Header
        DrawRectangle(modal.x, modal.y, modal.width, 56, METAL_ACCENT);
        const char* title = mode == LOGIN ? "[#] Login" : "[+] Registar";
        DrawTextCustom(title, modal.x+24, modal.y+18, 24, TEXT_WHITE);

        // Input fields with modern design
        Rectangle userBox={modal.x+40,modal.y+90,320,38};
        DrawRectangleRounded(userBox, 0.15f, 6, typingUser ? METAL_ACCENT : METAL_BG);
        Color userBorder = typingUser ? BUTTON_BLUE : METAL_ACCENT;
        DrawRectangleLinesEx(userBox, 2.0f, userBorder);
        DrawTextCustom("Nome de usuário", userBox.x, userBox.y - 24, 14, METAL_BRONZE);
        DrawTextCustom(username.c_str(), userBox.x+14, userBox.y+11, 16, TEXT_WHITE);

        Rectangle passBox={modal.x+40,modal.y+160,320,38};
        DrawRectangleRounded(passBox, 0.15f, 6, typingPass ? METAL_ACCENT : METAL_BG);
        Color passBorder = typingPass ? BUTTON_BLUE : METAL_ACCENT;
        DrawRectangleLinesEx(passBox, 2.0f, passBorder);
        DrawTextCustom("Senha", passBox.x, passBox.y - 24, 14, METAL_BRONZE);
        std::string maskedPassword(password.size(), '*');
        DrawTextCustom(maskedPassword.c_str(), passBox.x+14, passBox.y+11, 16, TEXT_WHITE);

        // Buttons
        Rectangle actBtn={modal.x+24,modal.y+228,120,36};
        Rectangle switchBtn={modal.x+154,modal.y+228,120,36};
        Rectangle cancelBtn={modal.x+284,modal.y+228,92,36};
        
        if(DrawButton(mode==LOGIN?"Entrar":"Registar",actBtn,BUTTON_BLUE)){
            if(username.empty()||password.empty()) message="[ERRO] Preencha todos os campos!";
            else if(mode==LOGIN){
                std::ifstream fi("users.txt"); std::string u,p; bool ok=false;
                while(fi>>u>>p) if(u==username&&p==password) ok=true;
                if(ok){logged=true;finished=true;}
                else message="[ERRO] Usuario ou senha invalidos!";
            } else {
                std::ifstream fi("users.txt"); std::string u,p; bool exists=false;
                while(fi>>u>>p) if(u==username) exists=true;
                if(!exists){
                    std::ofstream fo("users.txt",std::ios::app); fo<<username<<" "<<password<<"\n";
                    message="[OK] Registrado! Faca login."; mode=LOGIN; username.clear(); password.clear();
                } else message="[ERRO] Usuario ja existe!";
            }
        }
        if(DrawButton(mode==LOGIN?"Registar":"Login",switchBtn,METAL_ACCENT)) {
            mode=(mode==LOGIN?REGISTER:LOGIN); message.clear();username.clear();password.clear();
        }
        if(DrawButton("Cancelar",cancelBtn,BUTTON_RED)) {finished=true;logged=false;}
        
        // Message feedback
        if(!message.empty()){
            bool isSuccess = message.find("[OK]") != std::string::npos;
            Color msgColor = isSuccess ? SUCCESS_GREEN : BUTTON_RED;
            Rectangle msgBox = {modal.x + 24, modal.y + modal.height - 56, modal.width - 48, 32};
            DrawRectangleRounded(msgBox, 0.2f, 6, Fade(msgColor, 0.15f));
            DrawTextCustom(message.c_str(), msgBox.x + 12, msgBox.y + 8, 15, msgColor);
        }

        Vector2 mp=GetMousePosition();
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            if(CheckCollisionPointRec(mp,userBox)) typingUser=true,typingPass=false;
            else if(CheckCollisionPointRec(mp,passBox)) typingUser=false,typingPass=true;
        }
        int key=GetCharPressed();
        while(key>0){
            if(typingUser&&key>=32&&key<=126) username.push_back((char)key);
            if(typingPass&&key>=32&&key<=126) password.push_back((char)key);
            key=GetCharPressed();
        }
        if(IsKeyPressed(KEY_BACKSPACE)){
            if(typingUser&&!username.empty()) username.pop_back();
            if(typingPass&&!password.empty()) password.pop_back();
        }
        if(IsKeyPressed(KEY_TAB)) typingUser=!typingUser,typingPass=!typingPass;
        EndDrawing();
    }
    return logged;
}

// === USER MANAGEMENT ===
namespace techcore {

std::string SimpleHash(const std::string& password) {
    // Simple hash for demo purposes (NOT secure for production!)
    unsigned long hash = 5381;
    for (char c : password) {
        hash = ((hash << 5) + hash) + c;
    }
    return std::to_string(hash);
}

std::vector<techcore::User> LoadUsers() {
    std::vector<techcore::User> users;
    std::ifstream file("users.txt");
    if(file.is_open()) {
        std::string line;
        while(std::getline(file, line)) {
            if(line.empty()) continue; // Skip empty lines
            
            try {
                techcore::User u;
                size_t pos = 0;
                
                // Parse ID
                size_t nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue; // Invalid line
                std::string idStr = line.substr(pos, nextPos - pos);
                if(idStr.empty()) continue;
                u.id = std::stoi(idStr);
                pos = nextPos + 1;
                
                // Parse username
                nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue;
                u.username = line.substr(pos, nextPos - pos);
                pos = nextPos + 1;
                
                // Parse password hash
                nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue;
                u.passwordHash = line.substr(pos, nextPos - pos);
                pos = nextPos + 1;
                
                // Parse email
                nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue;
                u.email = line.substr(pos, nextPos - pos);
                pos = nextPos + 1;
                
                // Parse wishlist (comma-separated IDs)
                nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue;
                std::string wishlistStr = line.substr(pos, nextPos - pos);
                if(!wishlistStr.empty() && wishlistStr != "none") {
                    size_t wPos = 0;
                    while(wPos < wishlistStr.length()) {
                        size_t comma = wishlistStr.find(',', wPos);
                        if(comma == std::string::npos) comma = wishlistStr.length();
                        std::string numStr = wishlistStr.substr(wPos, comma - wPos);
                        if(!numStr.empty()) {
                            u.wishlist.push_back(std::stoi(numStr));
                        }
                        wPos = comma + 1;
                    }
                }
                pos = nextPos + 1;
                
                // Parse order history (comma-separated IDs)
                if(pos < line.length()) {
                    std::string ordersStr = line.substr(pos);
                    if(!ordersStr.empty() && ordersStr != "none") {
                        size_t oPos = 0;
                        while(oPos < ordersStr.length()) {
                            size_t comma = ordersStr.find(',', oPos);
                            if(comma == std::string::npos) comma = ordersStr.length();
                            std::string numStr = ordersStr.substr(oPos, comma - oPos);
                            if(!numStr.empty()) {
                                u.orderHistory.push_back(std::stoi(numStr));
                            }
                            oPos = comma + 1;
                        }
                    }
                }
                
                users.push_back(u);
            } catch(const std::exception& e) {
                // Skip malformed lines
                continue;
            }
        }
        file.close();
    }
    return users;
}

void SaveUsers(const std::vector<techcore::User>& users) {
    std::ofstream file("users.txt");
    if(file.is_open()) {
        for(const auto& u : users) {
            file << u.id << "|" << u.username << "|" << u.passwordHash << "|" << u.email << "|";
            
            // Wishlist
            if(u.wishlist.empty()) {
                file << "none";
            } else {
                for(size_t i = 0; i < u.wishlist.size(); i++) {
                    file << u.wishlist[i];
                    if(i < u.wishlist.size() - 1) file << ",";
                }
            }
            file << "|";
            
            // Order history
            if(u.orderHistory.empty()) {
                file << "none";
            } else {
                for(size_t i = 0; i < u.orderHistory.size(); i++) {
                    file << u.orderHistory[i];
                    if(i < u.orderHistory.size() - 1) file << ",";
                }
            }
            file << "\n";
        }
        file.close();
    }
}

techcore::User* FindUser(std::vector<techcore::User>& users, const std::string& username) {
    for(auto& u : users) {
        if(u.username == username) return &u;
    }
    return nullptr;
}

// === ORDER MANAGEMENT ===

std::string OrderStatusToString(techcore::OrderStatus status) {
    switch(status) {
        case techcore::OrderStatus::PENDING: return "Pendente";
        case techcore::OrderStatus::PROCESSING: return "Processando";
        case techcore::OrderStatus::SHIPPED: return "Enviado";
        case techcore::OrderStatus::DELIVERED: return "Entregue";
        case techcore::OrderStatus::CANCELLED: return "Cancelado";
        default: return "Desconhecido";
    }
}

techcore::OrderStatus StringToOrderStatus(const std::string& str) {
    if(str == "Pendente") return techcore::OrderStatus::PENDING;
    if(str == "Processando") return techcore::OrderStatus::PROCESSING;
    if(str == "Enviado") return techcore::OrderStatus::SHIPPED;
    if(str == "Entregue") return techcore::OrderStatus::DELIVERED;
    if(str == "Cancelado") return techcore::OrderStatus::CANCELLED;
    return techcore::OrderStatus::PENDING;
}

std::vector<techcore::Order> LoadOrders() {
    std::vector<techcore::Order> orders;
    std::ifstream file("orders.txt");
    if(file.is_open()) {
        std::string line;
        while(std::getline(file, line)) {
            if(line.empty()) continue; // Skip empty lines
            
            try {
                techcore::Order o;
                size_t pos = 0;
                
                // Parse ID
                size_t nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue;
                std::string idStr = line.substr(pos, nextPos - pos);
                if(idStr.empty()) continue;
                o.id = std::stoi(idStr);
                pos = nextPos + 1;
                
                // Parse username
                nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue;
                o.username = line.substr(pos, nextPos - pos);
                pos = nextPos + 1;
                
                // Parse total
                nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue;
                std::string totalStr = line.substr(pos, nextPos - pos);
                if(totalStr.empty()) continue;
                o.total = std::stof(totalStr);
                pos = nextPos + 1;
                
                // Parse status
                nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue;
                o.status = StringToOrderStatus(line.substr(pos, nextPos - pos));
                pos = nextPos + 1;
                
                // Parse date
                nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue;
                o.date = line.substr(pos, nextPos - pos);
                pos = nextPos + 1;
                
                // Parse shipping address
                nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue;
                o.shippingAddress = line.substr(pos, nextPos - pos);
                pos = nextPos + 1;
                
                // Parse payment method
                nextPos = line.find('|', pos);
                o.paymentMethod = line.substr(pos, nextPos - pos);
                
                // Note: OrderItems are complex, storing count for now
                // In production, would store item details separately
                
                orders.push_back(o);
            } catch(const std::exception& e) {
                // Skip malformed lines
                continue;
            }
        }
        file.close();
    }
    return orders;
}

void SaveOrders(const std::vector<techcore::Order>& orders) {
    std::ofstream file("orders.txt");
    if(file.is_open()) {
        for(const auto& o : orders) {
            file << o.id << "|" << o.username << "|" << o.total << "|" 
                 << OrderStatusToString(o.status) << "|" << o.date << "|" 
                 << o.shippingAddress << "|" << o.paymentMethod << "\n";
        }
        file.close();
    }
}

// === REVIEW MANAGEMENT ===

std::vector<techcore::Review> LoadReviews() {
    std::vector<techcore::Review> reviews;
    std::ifstream file("reviews.txt");
    if(file.is_open()) {
        std::string line;
        while(std::getline(file, line)) {
            if(line.empty()) continue; // Skip empty lines
            
            try {
                techcore::Review r;
                size_t pos = 0;
                
                // Parse ID
                size_t nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue;
                std::string idStr = line.substr(pos, nextPos - pos);
                if(idStr.empty()) continue;
                r.id = std::stoi(idStr);
                pos = nextPos + 1;
                
                // Parse product ID
                nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue;
                std::string pidStr = line.substr(pos, nextPos - pos);
                if(pidStr.empty()) continue;
                r.productId = std::stoi(pidStr);
                pos = nextPos + 1;
                
                // Parse username
                nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue;
                r.username = line.substr(pos, nextPos - pos);
                pos = nextPos + 1;
                
                // Parse rating
                nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue;
                std::string ratingStr = line.substr(pos, nextPos - pos);
                if(ratingStr.empty()) continue;
                r.rating = std::stof(ratingStr);
                pos = nextPos + 1;
                
                // Parse comment
                nextPos = line.find('|', pos);
                if(nextPos == std::string::npos) continue;
                r.comment = line.substr(pos, nextPos - pos);
                pos = nextPos + 1;
                
                // Parse date
                r.date = line.substr(pos);
                
                reviews.push_back(r);
            } catch(const std::exception& e) {
                // Skip malformed lines
                continue;
            }
        }
        file.close();
    }
    return reviews;
}

void SaveReviews(const std::vector<techcore::Review>& reviews) {
    std::ofstream file("reviews.txt");
    if(file.is_open()) {
        for(const auto& r : reviews) {
            file << r.id << "|" << r.productId << "|" << r.username << "|" 
                 << r.rating << "|" << r.comment << "|" << r.date << "\n";
        }
        file.close();
    }
}

} // end namespace techcore for user/order/review management
