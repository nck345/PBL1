#include "../../include/utils/StringUtils.h"
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
