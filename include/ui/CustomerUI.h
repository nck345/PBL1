#ifndef CUSTOMER_UI_H
#define CUSTOMER_UI_H

#include "../../include/models/Customer.h"
#include <vector>
#include <string>

void render_customer_menu();
int select_customer_ui(const std::string& title);

#include <ftxui/dom/elements.hpp>

ftxui::Element build_customer_table_element(const std::vector<Customer> &customers);
#endif
