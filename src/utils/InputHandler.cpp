#include "../../include/utils/InputHandler.h"
#include <iostream>
#include <limits>
#include <conio.h>
#include <cctype>

void clear_input_buffer() {
    // Không dùng nữa, đổi qua handle char
}

std::string get_string_input(const std::string& prompt) {
    std::cout << prompt;
    std::string value = "";
    while (true) {
        if (_kbhit()) {
            char c = _getch();
            if (c == 27) { // ESC
                std::cout << "\n[Huy thao tac]\n";
                return "[ESC]";
            } else if (c == '\r') { // Enter
                std::cout << '\n';
                return value;
            } else if (c == '\b') { // Backspace
                if (!value.empty()) {
                    std::cout << "\b \b";
                    value.pop_back();
                }
            } else if (std::isprint(c)) {
                std::cout << c;
                value.push_back(c);
            }
        }
    }
}

int get_int_input(const std::string& prompt) {
    while (true) {
        std::string raw = get_string_input(prompt);
        if (raw == "[ESC]") return -999999;
        try {
            size_t pos;
            int value = std::stoi(raw, &pos);
            if (pos == raw.length()) return value;
        } catch (...) {}
        std::cout << "Loi: Vui long nhap mot so nguyen hop le.\n";
    }
}

double get_double_input(const std::string& prompt) {
    while (true) {
        std::string raw = get_string_input(prompt);
        if (raw == "[ESC]") return -999999.0;
        try {
            size_t pos;
            double value = std::stod(raw, &pos);
            if (pos == raw.length()) return value;
        } catch (...) {}
        std::cout << "Loi: Vui long nhap mot so thuc hop le.\n";
    }
}
