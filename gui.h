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
    bool inStock;          // Stock availability
    bool isOnDiscount;     // Promotion status
    float discountPercent; // Discount percentage (0-100)
    float rating;          // Product rating (0.0-5.0)
};

struct CartItem {
    Product product;
    int qty;
};

struct User {
    int id;
    std::string username;
    std::string passwordHash; // Simple hash for demo
    std::string email;
    std::vector<int> wishlist; // Product IDs
    std::vector<int> orderHistory; // Order IDs
};

struct Review {
    int id;
    int productId;
    std::string username;
    float rating;
    std::string comment;
    std::string date;
};

enum class OrderStatus {
    PENDING,
    PROCESSING,
    SHIPPED,
    DELIVERED,
    CANCELLED
};

struct OrderItem {
    Product product;
    int qty;
    float priceAtPurchase;
};

struct Order {
    int id;
    std::string username;
    std::vector<OrderItem> items;
    float total;
    OrderStatus status;
    std::string date;
    std::string shippingAddress;
    std::string paymentMethod;
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

// User management
std::vector<User> LoadUsers();
void SaveUsers(const std::vector<User>& users);
User* FindUser(std::vector<User>& users, const std::string& username);
std::string SimpleHash(const std::string& password);

// Order management
std::vector<Order> LoadOrders();
void SaveOrders(const std::vector<Order>& orders);
std::string OrderStatusToString(OrderStatus status);

// Review management
std::vector<Review> LoadReviews();
void SaveReviews(const std::vector<Review>& reviews);

void RunTechcoreUI(int screenWidth, int screenHeight, bool (*loginFunc)(int, int));
void ShowAdminPanel(int screenWidth, int screenHeight, std::vector<Product>& products, bool& showAdmin);
}

// Button utility for use everywhere
bool DrawButton(const char* label, Rectangle r, Color bg);
