#include "../../include/repository/CustomerRepo.h"
#include <fstream>
#include <cstring>
#include <iostream>

using namespace std;

// Đường dẫn file lưu dữ liệu khách hàng
static const char* CUSTOMERS_FILE = "data/customers.dat";

// Lấy ID tiếp theo bằng cách đọc ID cuối cùng trong file
int get_next_customer_id() {
    ifstream file(CUSTOMERS_FILE, ios::binary | ios::ate);
    if (!file.is_open() || file.tellg() == 0) {
        return 1; // File rỗng hoặc chưa tồn tại -> bắt đầu từ 1
    }

    // Nhảy đến bản ghi cuối cùng
    streamsize record_size = sizeof(Customer);
    file.seekg(-record_size, ios::end);

    Customer last;
    file.read(reinterpret_cast<char*>(&last), sizeof(Customer));
    file.close();

    return last.id + 1;
}

// Lưu khách hàng vào file (append binary)
void save_customer(const Customer& customer) {
    ofstream file(CUSTOMERS_FILE, ios::binary | ios::app);
    if (!file.is_open()) {
        cerr << "Loi: Khong mo duoc file customers.dat de ghi.\n";
        return;
    }
    file.write(reinterpret_cast<const char*>(&customer), sizeof(Customer));
    file.close();
}

// Đọc toàn bộ danh sách khách hàng
vector<Customer> read_all_customers() {
    vector<Customer> list;
    ifstream file(CUSTOMERS_FILE, ios::binary);
    if (!file.is_open()) return list;

    Customer c;
    while (file.read(reinterpret_cast<char*>(&c), sizeof(Customer))) {
        if (!c.is_deleted) {
            list.push_back(c);
        }
    }
    file.close();
    return list;
}

// Tìm khách hàng theo số điện thoại
// Trả về true nếu tìm thấy và là khách chưa bị xóa, gán vào out_customer
bool find_customer_by_phone(const char* phone, Customer& out_customer) {
    ifstream file(CUSTOMERS_FILE, ios::binary);
    if (!file.is_open()) return false;

    Customer c;
    while (file.read(reinterpret_cast<char*>(&c), sizeof(Customer))) {
        if (!c.is_deleted && strncmp(c.phone, phone, sizeof(c.phone)) == 0) {
            out_customer = c;
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

// Soft delete khách hàng theo ID (ghi đè seekp)
bool delete_customer(int id) {
    fstream file(CUSTOMERS_FILE, ios::in | ios::out | ios::binary);
    if (!file.is_open()) return false;

    Customer c;
    streampos pos;
    while (true) {
        pos = file.tellg();
        if (!file.read(reinterpret_cast<char*>(&c), sizeof(Customer))) break;
        if (c.id == id) {
            c.is_deleted = true;
            file.seekp(pos);
            file.write(reinterpret_cast<const char*>(&c), sizeof(Customer));
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

// Cập nhật thông tin khách hàng (ghi đè seekp)
bool update_customer(const Customer& updated_customer) {
    fstream file(CUSTOMERS_FILE, ios::in | ios::out | ios::binary);
    if (!file.is_open()) return false;

    Customer c;
    streampos pos;
    while (true) {
        pos = file.tellg();
        if (!file.read(reinterpret_cast<char*>(&c), sizeof(Customer))) break;
        if (c.id == updated_customer.id) {
            file.seekp(pos);
            file.write(reinterpret_cast<const char*>(&updated_customer), sizeof(Customer));
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

// Kiểm tra trùng số điện thoại (dùng khi thêm mới khách hàng)
bool is_customer_duplicate(const char* phone) {
    Customer dummy;
    return find_customer_by_phone(phone, dummy);
}
