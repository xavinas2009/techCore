#pragma once
#include <raylib.h>
#include <vector>
#include <string>

namespace techcore {

struct Product {
    int id;
    std::string name;
    std::string desc;
    float price;
    Color color;
};

struct CartItem {
    Product product;
    int qty;
};

void RunTechcoreUI(int screenWidth, int screenHeight, bool (*loginFunc)(int, int));

}

bool DrawButton(const char* label, Rectangle r, Color bg);
