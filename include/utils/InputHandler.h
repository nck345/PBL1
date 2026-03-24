#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <string>

int get_int_input(const std::string& prompt);
double get_double_input(const std::string& prompt);
std::string get_string_input(const std::string& prompt);
void clear_input_buffer();

#endif
