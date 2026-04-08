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


#include <ftxui/dom/elements.hpp>
#include "../../include/models/RentalSlip.h"
#include "../../include/models/Comic.h"
#include "../../include/models/Customer.h"

Date parse_date_string(const std::string &date_str);
std::string get_c_name(int comic_id, const std::vector<Comic>& comics);
std::string get_c_author(int comic_id, const std::vector<Comic>& comics);
std::string get_c_type(int comic_id, const std::vector<Comic>& comics);
std::string get_cu_name(int customer_id, const std::vector<Customer>& customers);

bool cmp_rui_id_asc(const RentalUIRow& a, const RentalUIRow& b);
bool cmp_rui_id_desc(const RentalUIRow& a, const RentalUIRow& b);
bool cmp_rui_cu_asc(const RentalUIRow& a, const RentalUIRow& b);
bool cmp_rui_cu_desc(const RentalUIRow& a, const RentalUIRow& b);
bool cmp_rui_c_asc(const RentalUIRow& a, const RentalUIRow& b);
bool cmp_rui_c_desc(const RentalUIRow& a, const RentalUIRow& b);
bool cmp_rui_date_asc(const RentalUIRow& a, const RentalUIRow& b);
bool cmp_rui_date_desc(const RentalUIRow& a, const RentalUIRow& b);

ftxui::Element build_rental_table_element(const std::vector<RentalSlip>& slips, const std::vector<Comic>& all_c, const std::vector<Customer>& all_cu);
int select_rental_slip_ui(const std::string& title);
#endif
