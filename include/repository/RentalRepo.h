#ifndef RENTALREPO_H
#define RENTALREPO_H

#include "../models/RentalSlip.h"
#include <vector>

// Khai báo các hàm xử lý dữ liệu cho Phiếu Thuê
void save_rental_slip(const RentalSlip &slip);
void read_all_rental_slips();
std::vector<RentalSlip> get_all_rental_slips();
void update_rental_status(const RentalSlip &updated_slip);
int get_next_rental_id();
// Trả về true nếu TỒN TẠI phiếu đang hoạt động với cùng tên truyện + khách hàng
bool is_rental_duplicate(const char* ten_truyen, const char* khach_hang);

#endif
