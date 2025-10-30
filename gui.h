#pragma once
#include <raylib.h>
#include <vector>
#include <string>

namespace techcore {

// Estrutura que representa um produto da loja
struct Product {
    int id;             // ID único do produto
    std::string name;   // Nome do produto
    std::string desc;   // Descrição do produto
    float price;        // Preço do produto
    Color color;        // Cor associada ao produto para UI
};

// Estrutura que representa um item no carrinho
struct CartItem {
    Product product;  // Produto selecionado
    int qty;          // Quantidade do produto
};

// Função que inicia a interface da loja
// loginFunc é função callback para mostrar UI de login quando necessário
void RunTechcoreUI(int screenWidth, int screenHeight, bool (*loginFunc)(int, int));
}

// Utilitário para desenhar um botão com texto
// Retorna true se o botão for clicado
bool DrawButton(const char* label, Rectangle r, Color bg);
