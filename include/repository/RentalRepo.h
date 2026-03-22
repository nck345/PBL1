#ifndef RENTALREPO_H
#define RENTALREPO_H

#include "../models/RentalSlip.h"

// Khai báo các hàm xử lý dữ liệu cho Phiếu Thuê (Không dùng class)
void save_rental_slip(const RentalSlip& slip);
void read_all_rental_slips();

#endif // RENTALREPO_H
