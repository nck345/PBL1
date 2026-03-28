#ifndef RENTALSLIP_H
#define RENTALSLIP_H

#include "Date.h"

// Khai báo struct cho Phiếu Thuê
struct RentalSlip {
  int id_phieu;
  char ten_truyen[100];
  char khach_hang[100]; // Tên hoặc Số điện thoại
  Date ngay_muon;
  Date ngay_tra_du_kien;
  Date ngay_tra_thuc_te;
  double tien_coc;
  double tong_tien;
  int trang_thai; // 0: Đang thuê, 1: Đã trả, 2: Mất/Hỏng
};

#endif
