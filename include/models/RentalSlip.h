#ifndef RENTALSLIP_H
#define RENTALSLIP_H

#include <ctime>

// Khai báo struct cho Phiếu Thuê (Không dùng OOP)
struct RentalSlip {
    int id_phieu;
    int id_truyen;
    int id_khach_hang;
    time_t ngay_muon;
    time_t ngay_tra;
    double tien_coc;
    double tong_tien;
    int trang_thai; // 0: Đang thuê, 1: Đã trả, 2: Mất/Đền bù
};

#endif // RENTALSLIP_H
