#pragma once
#include <raylib.h>
#include <vector>
#include <string>

namespace techcore {

enum class ProductCategory {
    CPU,
    GPU,
    RAM,
    STORAGE,
    MOTHERBOARD,
    PSU,
    COOLING,
    CASE,
    PERIPHERAL
};

struct Product {
    int id;
    std::string name;
    std::string desc;
    float price;
    Color color;
    ProductCategory category;
    std::string imagePath; // Path to product image
};

struct CartItem {
    Product product;
    int qty;
};

enum class AdminAction {
    NONE,
    CREATE,
    EDIT,
    DELETE
};

// Export and utility functions
void ExportToCSV(const std::vector<Product>& products);
void GenerateReport(const std::vector<Product>& products);
void CreateBackup(const std::vector<Product>& products);
void LogAction(const std::string& action, const std::string& details);

void RunTechcoreUI(int screenWidth, int screenHeight, bool (*loginFunc)(int, int));
void ShowAdminPanel(int screenWidth, int screenHeight, std::vector<Product>& products, bool& showAdmin);
}

// Button utility for use everywhere
bool DrawButton(const char* label, Rectangle r, Color bg);
