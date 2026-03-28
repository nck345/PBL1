#include "../../include/repository/CustomerRepo.h"
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

const string CUSTOMER_FILE = "data/customers.dat";
const string CUSTOMER_META = "data/customer_id.dat";

int get_next_customer_id() {
  int id = 1;
  ifstream inFile(CUSTOMER_META, ios::binary);
  if (inFile.is_open()) {
    inFile.read(reinterpret_cast<char*>(&id), sizeof(int));
    inFile.close();
  }
  int next_id = id + 1;
  ofstream outFile(CUSTOMER_META, ios::binary);
  if (outFile.is_open()) {
    outFile.write(reinterpret_cast<const char*>(&next_id), sizeof(int));
    outFile.close();
  }
  return id;
}

vector<Customer> read_all_customers() {
  vector<Customer> customers;
  ifstream inFile(CUSTOMER_FILE, ios::binary);
  if (!inFile.is_open()) {
    return customers; 
  }

  Customer c;
  while (inFile.read(reinterpret_cast<char*>(&c), sizeof(Customer))) {
    customers.push_back(c);
  }

  inFile.close();
  return customers;
}

void add_customer(Customer& customer) {
  customer.id = get_next_customer_id();
  ofstream outFile(CUSTOMER_FILE, ios::app | ios::binary);
  if (outFile.is_open()) {
    outFile.write(reinterpret_cast<const char*>(&customer), sizeof(Customer));
    outFile.close();
  } else {
    cerr << "Khong the ghi file " << CUSTOMER_FILE << "!\n";
  }
}

bool update_customer(const Customer& updated_customer) {
  fstream file(CUSTOMER_FILE, ios::in | ios::out | ios::binary);
  if (!file.is_open()) return false;

  Customer c;
  bool found = false;
  streampos pos;
  
  while (file.read(reinterpret_cast<char*>(&c), sizeof(Customer))) {
    if (c.id == updated_customer.id) {
      found = true;
      pos = file.tellg() - static_cast<streamoff>(sizeof(Customer));
      break;
    }
  }

  if (found) {
    file.seekp(pos);
    file.write(reinterpret_cast<const char*>(&updated_customer), sizeof(Customer));
    file.close();
    return true;
  }

  file.close();
  return false;
}

bool delete_customer(int id) {
  Customer c;
  if (!get_customer_by_id(id, c)) return false;
  
  c.is_deleted = true;
  return update_customer(c);
}

vector<Customer> search_customers_by_phone(const string& phone) {
  vector<Customer> res;
  vector<Customer> all = read_all_customers();
  for (const auto& c : all) {
    if (!c.is_deleted && string(c.phone).find(phone) != string::npos) {
      res.push_back(c);
    }
  }
  return res;
}

vector<Customer> search_customers_by_name(const string& name) {
  vector<Customer> res;
  vector<Customer> all = read_all_customers();
  
  string keyword = name;
  transform(keyword.begin(), keyword.end(), keyword.begin(), ::tolower);

  for (const auto& c : all) {
    if (!c.is_deleted) {
      string c_name = c.name;
      transform(c_name.begin(), c_name.end(), c_name.begin(), ::tolower);
      if (c_name.find(keyword) != string::npos) {
        res.push_back(c);
      }
    }
  }
  return res;
}

bool get_customer_by_id(int id, Customer& out_customer) {
  vector<Customer> all = read_all_customers();
  for (const auto& c : all) {
    if (c.id == id && !c.is_deleted) {
      out_customer = c;
      return true;
    }
  }
  return false;
}

bool is_customer_duplicate(const string& phone) {
  vector<Customer> all = read_all_customers();
  for (const auto& c : all) {
    if (!c.is_deleted && string(c.phone) == phone) {
      return true;
    }
  }
  return false;
}
