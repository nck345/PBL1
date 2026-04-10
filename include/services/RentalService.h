#ifndef RENTALSERVICE_H
#define RENTALSERVICE_H

#include <vector>
#include "../models/Date.h"
#include "../models/RentalSlip.h"

// Ty le phat/thue
#define PHI_PHAT_QUA_HAN_MOT_NGAY 5000.0
#define TY_LE_THUE_THEO_NGAY 0.1 // 10% gia bia

// Thư viện tự tạo hỗ trợ tính ngày
long date_to_days(Date d);

// Xử lý nghiệp vụ Mượn/Trả
void process_new_rental(const char* ten_truyen, const char* khach_hang, Date ngay_muon,
                        Date ngay_tra_du_kien, double gia_bia);
void compute_payment_bill(RentalSlip &slip, double gia_bia);
void process_return_comic(int id_phieu, Date ngay_tra_thuc_te,
                          int trang_thai_tra, double gia_bia);

// Thong ke & Bao cao
Date get_current_date();
bool compare_revenue_desc(const RentalSlip& a, const RentalSlip& b);
bool compare_overdue_priority_desc(const RentalSlip& a, const RentalSlip& b);

typedef struct {
  double target_daily_revenue;
  double range_monthly_revenue;
  int rented_count;
  int lost_count;
  std::vector<int> chart_data; // Used for FTXUI graph plotting
} rental_statistics;

rental_statistics compute_all_statistics(Date target_date, int start_month, int start_year,
                                         int end_month, int end_year);
void find_overdue_slips(RentalSlip overdue_list[], int &count, int max_size,
                        Date today);

#endif
