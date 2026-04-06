#ifndef RENTAL_UI_H
#define RENTAL_UI_H

#include "../models/RentalSlip.h"
#include <string>

struct RentalUIRow {
    RentalSlip slip;
    std::string cu_name;
    std::string c_name;
    std::string c_author;
    std::string c_type;
};

// Giao dien Quan ly phieu thue
void render_rental_menu();

// Giao dien Thong ke & Doanh thu
void render_statistics_screen();

#endif
