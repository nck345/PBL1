#ifndef VALIDATION_UTILS_H
#define VALIDATION_UTILS_H

#include <string>
#include "../models/Date.h"

// Tu StringUtils cu
std::string trim(const std::string& str);
std::string format_currency(double amount);

// Validations moi
bool is_empty_string(const std::string& str);
bool is_negative(double val);
bool is_valid_date(int day, int month, int year);
bool is_valid_date_struct(Date d);

#endif
