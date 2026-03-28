#ifndef CUSTOMER_UI_H
#define CUSTOMER_UI_H

#include "../../include/models/Customer.h"
#include <vector>
#include <string>

void render_customer_table(const std::vector<Customer> &customers);
void render_customer_menu();
int select_customer_ui(const std::string& title);

#endif
