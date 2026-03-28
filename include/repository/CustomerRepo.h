#ifndef CUSTOMER_REPO_H
#define CUSTOMER_REPO_H

#include "../../include/models/Customer.h"
#include <vector>
#include <string>

int get_next_customer_id();

std::vector<Customer> read_all_customers();

void add_customer(Customer& customer);

bool update_customer(const Customer& updated_customer);

bool delete_customer(int id);

std::vector<Customer> search_customers_by_phone(const std::string& phone);
std::vector<Customer> search_customers_by_name(const std::string& name);

bool get_customer_by_id(int id, Customer& out_customer);

bool is_customer_duplicate(const std::string& phone);

bool find_customer_by_phone(const char* phone, Customer& out_customer);

#endif
