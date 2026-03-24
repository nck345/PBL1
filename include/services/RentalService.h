#ifndef RENTALSERVICE_H
#define RENTALSERVICE_H

#include "../models/Date.h"
#include "../models/RentalSlip.h"

// Ty le phat/thue
#define PHI_PHAT_QUA_HAN_MOT_NGAY 5000.0
#define TY_LE_THUE_THEO_NGAY 0.1 // 10% gia bia

// Thư viện tự tạo hỗ trợ tính ngày
long date_to_days(Date d);

// Xử lý nghiệp vụ Mượn/Trả
void process_new_rental(int id_truyen, int id_khach_hang, Date ngay_muon,
                        Date ngay_tra_du_kien, double gia_bia);
void compute_payment_bill(RentalSlip &slip, double gia_bia);
void process_return_comic(int id_phieu, Date ngay_tra_thuc_te,
                          int trang_thai_tra, double gia_bia);

#endif
