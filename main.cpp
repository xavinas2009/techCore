#include <raylib.h>
#include <string>
#include <fstream>
#include <iostream>
#include "gui.h"

// Dark metallic theme colors
static const Color METAL_BG = { 18, 18, 20, 255 };
static const Color METAL_PANEL = { 36, 36, 40, 255 };
static const Color METAL_ACCENT = { 96, 100, 104, 255 };
static const Color METAL_HIGHLIGHT = { 180, 180, 184, 255 };
static const Color METAL_BRONZE = { 120, 116, 112, 255 };
static const Color BUTTON_BLUE = { 50, 115, 220, 255 };
static const Color BUTTON_RED = { 190, 50, 50, 255 };
static const Color TEXT_WHITE = { 220, 220, 220, 255 };
static const Color TEXT_RED = { 220, 100, 100, 255 };

// Validate credentials in users.txt
bool ValidateLogin(const std::string& username, const std::string& password) {
    std::ifstream file("users.txt");
    if (!file.is_open()) return false;
    std::string u, p;
    while (file >> u >> p) {
        if (u == username && p == password) return true;
    }
    return false;
}

// Append new user if username not taken
bool RegisterUser(const std::string& username, const std::string& password) {
    std::ifstream file("users.txt");
    std::string u, p;
    while (file >> u >> p) {
        if (u == username) return false; // username taken
    }
    std::ofstream outfile("users.txt", std::ios::app);
    if (!outfile.is_open()) return false;
    outfile << username << " " << password << "\n";
    return true;
}

// Draw centered button with label, returns true if clicked
bool DrawButton(const char* label, Rectangle rec, Color bgColor) {
    DrawRectangleRec(rec, bgColor);
    int txtW = MeasureText(label, 18);
    DrawText(label, (int)(rec.x + (rec.width - txtW) / 2), (int)(rec.y + (rec.height - 18) / 2), 18, TEXT_WHITE);
    if (CheckCollisionPointRec(GetMousePosition(), rec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return true;
    return false;
}

// Raylib graphical Login/Register UI with correct character input handling
bool RunLoginUI(int screenWidth, int screenHeight) {
    enum Mode { LOGIN, REGISTER };
    Mode mode = LOGIN;

    std::string username, password, message;
    bool typingUser = true, typingPass = false;

    InitWindow(screenWidth, screenHeight, "Techcore Login");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(METAL_BG);

        DrawText(mode == LOGIN ? "Login" : "Register", screenWidth / 2 - 40, 40, 28, METAL_HIGHLIGHT);

        Rectangle userBox{ screenWidth / 2 - 150, 120, 300, 40 };
        DrawRectangleRec(userBox, typingUser ? METAL_PANEL : METAL_ACCENT);
        DrawText("Username:", userBox.x - 110, userBox.y + 10, 20, METAL_BRONZE);
        DrawText(username.c_str(), userBox.x + 10, userBox.y + 10, 20, TEXT_WHITE);

        Rectangle passBox{ screenWidth / 2 - 150, 180, 300, 40 };
        DrawRectangleRec(passBox, typingPass ? METAL_PANEL : METAL_ACCENT);
        DrawText("Password:", passBox.x - 110, passBox.y + 10, 20, METAL_BRONZE);
        DrawText(std::string(password.size(), '*').c_str(), passBox.x + 10, passBox.y + 10, 20, TEXT_WHITE);

        Rectangle actionBtn{ screenWidth / 2 - 60, 260, 120, 40 };
        DrawButton(mode == LOGIN ? "Login" : "Register", actionBtn, BUTTON_BLUE);

        Rectangle switchBtn{ screenWidth / 2 - 80, 320, 160, 30 };
        DrawButton(mode == LOGIN ? "Switch to Register" : "Switch to Login", switchBtn, METAL_ACCENT);

        DrawText(message.c_str(), screenWidth / 2 - MeasureText(message.c_str(), 20) / 2, 380, 20, TEXT_RED);

        EndDrawing();

        Vector2 mp = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mp, userBox)) {
                typingUser = true;
                typingPass = false;
            }
            else if (CheckCollisionPointRec(mp, passBox)) {
                typingUser = false;
                typingPass = true;
            }
            else if (CheckCollisionPointRec(mp, actionBtn)) {
                if (mode == LOGIN) {
                    if (ValidateLogin(username, password)) {
                        CloseWindow();
                        return true;
                    } else {
                        message = "Invalid username or password!";
                    }
                } else {
                    if (RegisterUser(username, password)) {
                        message = "Registration successful! Please login.";
                        mode = LOGIN;
                        username.clear();
                        password.clear();
                    } else {
                        message = "Username already exists!";
                    }
                }
            }
            else if (CheckCollisionPointRec(mp, switchBtn)) {
                mode = mode == LOGIN ? REGISTER : LOGIN;
                message.clear();
                username.clear();
                password.clear();
            }
        }

        // Correct input handling: loop over all chars pressed this frame
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
    }
    CloseWindow();
    return false;
}

int main() {
    const int SCREEN_WIDTH = 1280;
    const int SCREEN_HEIGHT = 720;

    if (!RunLoginUI(SCREEN_WIDTH, SCREEN_HEIGHT)) {
        std::cout << "Login cancelled or failed, exiting.\n";
        return 0;
    }

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Techcore - PC Components Store");
    techcore::RunTechcoreUI(SCREEN_WIDTH, SCREEN_HEIGHT);
    CloseWindow();
    return 0;
}
