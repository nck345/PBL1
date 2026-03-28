#ifndef CUSTOMER_REPO_H
#define CUSTOMER_REPO_H

#include "../../include/models/Customer.h"
#include <vector>

// Lấy ID khách hàng tự tăng (đọc từ cuối file customers.dat)
int get_next_customer_id();

// Lưu khách hàng mới vào file customers.dat (mode append binary)
void save_customer(const Customer& customer);

// Đọc toàn bộ danh sách khách hàng từ customers.dat
std::vector<Customer> read_all_customers();

// Tìm khách hàng theo số điện thoại (trả về true nếu tìm thấy, gán vào out_customer)
// Chỉ trả về khách chưa bị xóa (is_deleted == false)
bool find_customer_by_phone(const char* phone, Customer& out_customer);

// Soft delete khách hàng theo ID
bool delete_customer(int id);

// Cập nhật thông tin khách hàng (ghi đè seekp)
bool update_customer(const Customer& updated_customer);

// Kiểm tra trùng số điện thoại (dùng khi thêm mới)
bool is_customer_duplicate(const char* phone);

#endif
