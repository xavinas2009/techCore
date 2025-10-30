#include "gui.h"
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

// Theme colors
static const Color METAL_BG = {18, 18, 20, 255};
static const Color METAL_PANEL = {36, 36, 40, 255};
static const Color METAL_ACCENT = {96, 100, 104, 255};
static const Color METAL_HIGHLIGHT = {180, 180, 184, 255};
static const Color METAL_BRONZE = {120, 116, 112, 255};
static const Color BUTTON_BLUE = {50, 115, 220, 255};
static const Color BUTTON_RED = {190, 50, 50, 255};
static const Color TEXT_WHITE = {220, 220, 220, 255};

// Button utility
bool DrawButton(const char *label, Rectangle r, Color bg) {
    DrawRectangleRec(r, bg);
    int txtW = MeasureText(label, 18);
    DrawText(label, (int)(r.x + (r.width-txtW)/2), (int)(r.y+(r.height-18)/2), 18, TEXT_WHITE);
    return (CheckCollisionPointRec(GetMousePosition(), r) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
}

namespace techcore {

std::vector<Product> defaultProducts() {
    return {
        {1, "CPU: Ryzen 5 5600X", "Processador 6 núcleos para desktop.", 199.99f, ORANGE},
        {2, "CPU: Ryzen 7 5800X", "Processador 8 núcleos para desktop.", 349.99f, ORANGE},
        {3, "CPU: Ryzen 9 5950X", "Processador 16 núcleos para desktop.", 549.99f, ORANGE},
        {4, "CPU: Intel i5-12600K", "Processador 10 núcleos para desktop.", 279.99f, BLUE},
        {5, "CPU: Intel i7-12700K", "Processador 12 núcleos para desktop.", 389.99f, BLUE},
        {6, "CPU: Intel i9-12900K", "Processador 16 núcleos para desktop.", 589.99f, BLUE},
        {7, "GPU: RTX 3060", "Placa gráfica 12GB GDDR6.", 299.99f, DARKBLUE},
        {8, "GPU: RTX 3070", "Placa gráfica 8GB GDDR6.", 499.99f, DARKBLUE},
        {9, "GPU: RTX 3080", "Placa gráfica 10GB GDDR6X.", 699.99f, DARKBLUE},
        {13, "RAM: 16GB DDR4 3200", "Kit memória 16GB (2x8GB) 3200MHz.", 79.99f, LIME},
    };
}

void DrawHeader(int screenW, int cartCount, bool highlightCartBtn) {
    DrawRectangle(0, 0, screenW, 72, METAL_BG);
    Rectangle logoBox{12.0f, 12.0f, 56.0f, 48.0f};
    DrawRectangleRec(logoBox, METAL_PANEL);
    DrawRectangleLinesEx(logoBox, 2, METAL_ACCENT);
    DrawText("Techcore", 80, 24, 28, METAL_HIGHLIGHT);
    DrawText("Componentes", 80, 54, 15, METAL_BRONZE);

    // Botão de carrinho no topo direito
    float btnW = 120, btnH = 40;
    float btnX = screenW - btnW - 32, btnY = 16;
    Rectangle cartBtn = {btnX, btnY, btnW, btnH};
    DrawRectangleRec(cartBtn, highlightCartBtn ? BUTTON_BLUE : METAL_PANEL);
    DrawRectangleLinesEx(cartBtn, 2, METAL_ACCENT);
    std::string text = "Carrinho";
    if(cartCount > 0) text += " (" + std::to_string(cartCount) + ")";
    int txW = MeasureText(text.c_str(),18);
    DrawText(text.c_str(), (int)(btnX+btnW/2-txW/2), (int)(btnY+btnH/2-9), 18, TEXT_WHITE);
}

void ShowCartModal(int screenWidth, int screenHeight, std::vector<CartItem>& cart, bool& showModal, std::string& cartMessage) {
    // Modal semitransparente
    DrawRectangle(0,0,screenWidth,screenHeight, Fade(BLACK,0.72f));
    int mw = 480, mh = 380;
    Rectangle modal{(float)(screenWidth/2-mw/2), (float)(screenHeight/2-mh/2), (float)mw, (float)mh};
    DrawRectangleRec(modal, METAL_PANEL);
    DrawRectangleLinesEx(modal, 2, METAL_ACCENT);

    DrawText("Carrinho de Compras", modal.x+20, modal.y+12, 24, METAL_HIGHLIGHT);

    int y = (int)modal.y + 50;
    float total = 0.0f;
    if(cart.empty()){
        DrawText("O carrinho está vazio.", modal.x+20, y, 18, METAL_BRONZE);
    } else {
        for(size_t i=0; i<cart.size(); ++i){
            const CartItem& item = cart[i];
            char line[256];
            snprintf(line, sizeof(line), "%dx %s  -  €%.2f", item.qty, item.product.name.c_str(), item.product.price * item.qty);
            DrawText(line, modal.x+24, y, 18, METAL_HIGHLIGHT);
            y += 28;
            total += item.product.price * item.qty;
        }
    }

    char totalBuf[64];
    snprintf(totalBuf, sizeof(totalBuf), "Total: € %.2f", total);
    DrawText(totalBuf, modal.x+24, modal.y+mh-90, 20, METAL_BRONZE);

    Rectangle btnFinalizar{modal.x+30, modal.y+mh-48, 160, 32};
    Rectangle btnContinuar{modal.x+modal.width-190, modal.y+mh-48, 160, 32};

    if(DrawButton("Finalizar compra", btnFinalizar, BUTTON_BLUE)) {
        cartMessage = "Compra registada! (Simulação)";
        showModal = false; // Fecha o modal; podes aqui abrir outro ou limpar carrinho, etc.
        cart.clear();
    }
    if(DrawButton("Continuar a comprar", btnContinuar, METAL_ACCENT)) {
        showModal = false;
    }

    // Mensagem feedback rápida, se existir
    if(!cartMessage.empty()){
        DrawText(cartMessage.c_str(), modal.x+24, modal.y+mh-110, 18, BUTTON_RED);
    }
}

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
    float cardW = (screenWidth - 80 - (cols-1)*gutter)/cols;
    float cardH = 112.f;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(METAL_BG);

        // Detecta se o cursor está sobre o botão do carrinho
        float btnW = 120, btnH = 40;
        float btnX = screenWidth - btnW - 32, btnY = 16;
        Rectangle cartBtn = {btnX, btnY, btnW, btnH};
        Vector2 mp = GetMousePosition();
        bool highlightCart = CheckCollisionPointRec(mp, cartBtn);

        DrawHeader(screenWidth, (int)cart.size(), highlightCart);

        // Abre modal do carrinho, se clicar no botão carrinho
        if(CheckCollisionPointRec(mp, cartBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            showCart = true;
            cartMessage = "";
        }

        // Barra de pesquisa, igual à última versão enviada a ti
        Rectangle searchBar = {40, 120, 320, 32};
        DrawRectangleRec(searchBar, METAL_PANEL);
        DrawRectangleLinesEx(searchBar, 1, METAL_ACCENT);
        const char* searchLabel = "Pesquisar";
        int labelWidth = MeasureText(searchLabel, 16);
        float labelX = 48;
        float textStartX = labelX + labelWidth + 6; // 6px após label
        Rectangle textArea = {textStartX, 120, searchBar.width - (textStartX - searchBar.x) - 10, 32};

        if (!isSearchActive && search.empty()) {
            DrawText(searchLabel, labelX, 126, 16, METAL_BRONZE);
        }
        if (CheckCollisionPointRec(GetMousePosition(), searchBar) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            isSearchActive = true;
        } else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !CheckCollisionPointRec(GetMousePosition(), searchBar)) {
            isSearchActive = false;
        }
        if (isSearchActive) {
            DrawText(searchLabel, labelX, 126, 16, METAL_BRONZE);
            float textWidth = MeasureText(search.c_str(), 16);
            BeginScissorMode((int)textArea.x, (int)textArea.y, (int)textArea.width, (int)textArea.height);
            DrawText(search.c_str(), (int)textStartX, 126, 16, METAL_HIGHLIGHT);
            if (((GetTime() * 2) - (int)(GetTime() * 2) < 0.5f)) {
                DrawRectangle((int)textStartX + (int)textWidth + 2, 126, 2, 16, METAL_HIGHLIGHT);
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
                    if (MeasureText(testStr.c_str(), 16) < (textArea.width - 10)) {
                        search += (char)key;
                    }
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_ESCAPE)) isSearchActive = false;
        }

        // CATEGORY FILTER UI (chips/buttons)
        float chipX = 32, chipY = 80;
        for (size_t ci=0; ci<categories.size(); ++ci) {
            Rectangle chip{chipX, chipY, 90, 28};
            bool active = (categories[ci] == selectedCategory);
            DrawRectangleRec(chip, active ? METAL_ACCENT : METAL_PANEL);
            DrawText(categories[ci].c_str(), chip.x+14, chip.y+6, 16, active?METAL_BG:METAL_HIGHLIGHT);
            if(CheckCollisionPointRec(GetMousePosition(),chip) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                selectedCategory = categories[ci];
            chipX += 100;
        }

        // Produtos filtrados por categoria e pesquisa
        std::vector<int> visible;
        for(size_t i=0;i<products.size();++i){
            bool shown=true;
            if(selectedCategory!="All"&&products[i].name.find(selectedCategory)==std::string::npos) shown=false;
            std::string low=products[i].name+products[i].desc;
            std::transform(low.begin(),low.end(),low.begin(),::tolower);
            std::string q=search; std::transform(q.begin(),q.end(),q.begin(),::tolower);
            if(!q.empty()&&low.find(q)==std::string::npos) shown=false;
            if(shown) visible.push_back((int)i);
        }

        // GRID LIST
        for(size_t k=0;k<visible.size();++k){
            int i=visible[k];
            int row=k/cols, col=k%cols;
            float x=32+col*(cardW+gutter);
            float y=180+row*(cardH+gutter);
            Rectangle card{x,y,cardW,cardH};
            DrawRectangleRec(card,METAL_PANEL);
            DrawRectangleLinesEx(card,2,METAL_ACCENT);
            DrawText(products[i].name.c_str(),x+18,y+18,18,METAL_HIGHLIGHT);
            DrawText(products[i].desc.c_str(),x+18,y+48,15,METAL_BRONZE);
            char priceBuf[32]; snprintf(priceBuf,sizeof(priceBuf),"€ %.2f",products[i].price);
            DrawText(priceBuf, x+cardW-100, y+18, 16, GOLD);

            Rectangle btn={x+cardW-140, y+cardH-36, 110, 28};
            if(DrawButton("Adicionar", btn, BUTTON_BLUE)){
                if(!isLoggedIn) showLoginPrompt=true;
                else{
                    auto it=std::find_if(cart.begin(),cart.end(),
                        [&](const CartItem&c){return c.product.id==products[i].id;});
                    if(it!=cart.end()) it->qty+=1; else cart.push_back({products[i],1});
                }
            }
        }

        if(showLoginPrompt){
            if(LoginFunc(screenWidth,screenHeight))isLoggedIn=true;
            showLoginPrompt=false;
        }

        // MODAL DO CARRINHO
        if(showCart){
            ShowCartModal(screenWidth, screenHeight, cart, showCart, cartMessage);
        }

        EndDrawing();
    }
}
}

// --- Login Modal (igual ao teu) ---
bool RunLoginUI(int screenWidth, int screenHeight) {
    enum Mode { LOGIN, REGISTER }; Mode mode = LOGIN;
    std::string username, password, message;
    bool typingUser = true, typingPass = false;
    bool finished = false, logged = false;
    while (!finished && !WindowShouldClose()) {
        BeginDrawing();
        DrawRectangle(0,0,screenWidth,screenHeight, Fade(BLACK,0.72f));
        Rectangle modal{(float)(screenWidth/2-170), (float)(screenHeight/2-100), 340, 200};
        DrawRectangleRec(modal, METAL_PANEL);
        DrawText(mode==LOGIN?"Login":"Registar",modal.x+24,modal.y+14,20,METAL_HIGHLIGHT);

        Rectangle userBox={modal.x+30,modal.y+60,280,28};
        DrawRectangleRec(userBox,typingUser?METAL_ACCENT:METAL_PANEL);
        DrawText("Nome:",userBox.x-18,userBox.y-18,16,METAL_BRONZE);
        DrawText(username.c_str(),userBox.x+10,userBox.y+8,16,TEXT_WHITE);

        Rectangle passBox={modal.x+30,modal.y+100,280,28};
        DrawRectangleRec(passBox,typingPass?METAL_ACCENT:METAL_PANEL);
        DrawText("Senha:",passBox.x-18,passBox.y-18,16,METAL_BRONZE);
        DrawText(std::string(password.size(),'*').c_str(),passBox.x+10,passBox.y+8,16,TEXT_WHITE);

        Rectangle actBtn={modal.x+14,modal.y+150,100,28};
        Rectangle switchBtn={modal.x+128,modal.y+150,90,28};
        Rectangle cancelBtn={modal.x+230,modal.y+150,80,28};
        if(DrawButton(mode==LOGIN?"Login":"Registar",actBtn,BUTTON_BLUE)){
            if(username.empty()||password.empty()) message="Preencha ambos campos!";
            else if(mode==LOGIN){
                std::ifstream fi("users.txt"); std::string u,p; bool ok=false;
                while(fi>>u>>p) if(u==username&&p==password) ok=true;
                if(ok){logged=true;finished=true;}
                else message="Usuário ou senha inválidos!";
            } else {
                std::ifstream fi("users.txt"); std::string u,p; bool exists=false;
                while(fi>>u>>p) if(u==username) exists=true;
                if(!exists){
                    std::ofstream fo("users.txt",std::ios::app); fo<<username<<" "<<password<<"\n";
                    message="Registrado! Faça login."; mode=LOGIN; username.clear(); password.clear();
                } else message="Usuário já existe!";
            }
        }
        if(DrawButton(mode==LOGIN?"Registar":"Login",switchBtn,METAL_ACCENT)) {
            mode=(mode==LOGIN?REGISTER:LOGIN); message.clear();username.clear();password.clear();
        }
        if(DrawButton("Cancelar",cancelBtn,BUTTON_RED)) {finished=true;logged=false;}
        DrawText(message.c_str(),modal.x+22,modal.y+modal.height-34,16,RED);

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
