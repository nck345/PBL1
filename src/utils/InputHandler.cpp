#include "utils/InputHandler.h"
#include <iostream>
#include <limits>

void clear_input_buffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int get_int_input(const std::string& prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            clear_input_buffer();
            return value;
        } else {
            std::cout << "Loi: Vui long nhap mot so nguyen hop le.\n";
            clear_input_buffer();
        }
    }
}

double get_double_input(const std::string& prompt) {
    double value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            clear_input_buffer();
            return value;
        } else {
            std::cout << "Loi: Vui long nhap mot so thuc hop le.\n";
            clear_input_buffer();
        }
    }
}

std::string get_string_input(const std::string& prompt) {
    std::string value;
    std::cout << prompt;
    std::getline(std::cin, value);
    return value;
}
