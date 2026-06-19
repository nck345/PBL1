#ifndef RENTALSLIP_H
#define RENTALSLIP_H

#include "Date.h"

// Khai báo struct cho Phiếu Thuê
struct RentalSlip {
  int id_phieu;
  int comic_id;
  int book_copy_id; // Thêm trường book_copy_id để theo dõi bản sao cụ thể
  int customer_id;
  Date ngay_muon;
  Date ngay_tra_du_kien;
  Date ngay_tra_thuc_te;
  double tien_coc;
  double tong_tien;
  int trang_thai;// 0: Đang thuê, 1: Đã trả, 2: Trả bị rách, 3: Đặt trước / Trả mất trang, 4: Làm mất/Hỏng hoàn toàn
};

#endif
