#pragma once
#include <vector>
#include <string>
#include <raylib.h>

namespace techcore {

struct Product {
    int id;
    std::string name;
    std::string desc;
    float price;
    Color color; // placeholder thumbnail color
};

struct CartItem {
    Product product;
    int qty;
};

enum class CartMenuState {
    Hidden,
    Showing
};

// Starts the Techcore UI loop. Assumes a window was already created.
void RunTechcoreUI(int screenWidth = 1280, int screenHeight = 720);

} // namespace techcore
