#ifndef RENTALREPO_H
#define RENTALREPO_H

#include "../models/RentalSlip.h"

// Khai báo các hàm xử lý dữ liệu cho Phiếu Thuê
void save_rental_slip(const RentalSlip &slip);
void read_all_rental_slips();
void update_rental_status(const RentalSlip &updated_slip);
int get_next_rental_id();

#endif
