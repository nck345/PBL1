#include "../../include/utils/ValidationUtils.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

std::string trim(const std::string& str) {
    if (str.empty()) return str;

    auto start = str.begin();
    while (start != str.end() && std::isspace(*start)) {
        start++;
    }

    auto end = str.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    if (start > end) return "";
    return std::string(start, end + 1);
}

std::string format_currency(double amount) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(0) << amount;
    std::string str = ss.str();
    
    int n = str.length();
    for (int i = n - 3; i > 0; i -= 3) {
        if (str[i-1] != '-') {
            str.insert(i, ".");
        }
    }
    return str + " VND";
}

bool is_empty_string(const std::string& str) {
    return trim(str).empty();
}

bool is_negative(double val) {
    return val < 0;
}

bool is_leap_year_validation(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

bool is_valid_date(int day, int month, int year) {
    if (year < 1) return false;
    if (month < 1 || month > 12) return false;
    if (day < 1) return false;

    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    if (month == 2 && is_leap_year_validation(year)) {
        days_in_month[2] = 29;
    }

    if (day > days_in_month[month]) return false;
    return true;
}

bool is_valid_date_struct(Date d) {
    return is_valid_date(d.day, d.month, d.year);
}
