#ifndef RENTALSERVICE_H
#define RENTALSERVICE_H

#include "../models/Date.h"
#include "../models/RentalSlip.h"
#include <vector>

// Ty le phat/thue
#define PHI_PHAT_QUA_HAN_MOT_NGAY 5000.0

// Thư viện tự tạo hỗ trợ tính ngày
long date_to_days(Date d);
Date add_days(Date d, int days);

// Xử lý nghiệp vụ Mượn/Trả/Đặt Trước
int process_new_rental(int comic_id, int customer_id, Date ngay_tra_du_kien, double tien_coc, double tien_thue, bool is_reservation = false, Date custom_start_date = {0,0,0});
bool get_earliest_return_date(int comic_id, Date& out_date);
void compute_payment_bill(RentalSlip &slip, double gia_bia, int trang_thai_tra);
void process_return_comic(int id_phieu, Date ngay_tra_thuc_te,
                          int trang_thai_tra, double gia_bia);

// Thong ke & Bao cao
bool compare_revenue_desc(const RentalSlip& a, const RentalSlip& b);
bool compare_overdue_priority_desc(const RentalSlip& a, const RentalSlip& b);

typedef struct {
  double daily_revenue;
  double monthly_revenue;
  int rented_count;
  int lost_count;
} rental_statistics;

rental_statistics compute_all_statistics(Date today, int target_month,
                                         int target_year);
void find_overdue_slips(RentalSlip overdue_list[], int &count, int max_size,
                        Date today);

// New statistics functions
void get_current_date(int &d, int &m, int &y);
double compute_revenue_between_months(int m1, int y1, int m2, int y2);
std::vector<int> get_revenue_chart_data(int month, int year);
std::vector<int> get_monthly_chart_data(int m1, int y1, int m2, int y2);

#endif
