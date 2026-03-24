#include "../../include/services/RentalService.h"
#include "../../include/repository/RentalRepo.h"
#include <iostream>
#include <fstream>

using namespace std;

bool is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

long date_to_days(Date d) {
    long total_days = 0;

    //Tính tổng ngày của các NĂM trọn vẹn trước đó (từ năm 1 đến year - 1)
    int y = d.year - 1; 
    // Mỗi năm 365 ngày + Cộng thêm 1 ngày cho mỗi năm nhuận đã trôi qua
    total_days = y * 365 + y / 4 - y / 100 + y / 400;

    //Tính tổng ngày của các THÁNG trọn vẹn trước đó (trong năm hiện tại)
    // Mảng lưu số ngày của 12 tháng (tháng 0 bỏ trống để index chạy từ 1 đến 12 cho dễ nhìn)
    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // Nếu năm nay là năm nhuận, tháng 2 có 29 ngày
    if (is_leap_year(d.year)) {
        days_in_month[2] = 29;
    }

    // Cộng dồn ngày của các tháng trước tháng d.month
    for (int m = 1; m < d.month; m++) {
        total_days += days_in_month[m];
    }

    //Cộng thêm số ngày của tháng hiện tại
    total_days += d.day;

    return total_days;
}

// Xử lý mượn truyện
void process_new_rental(int id_truyen, int id_khach_hang, Date ngay_muon, Date ngay_tra_du_kien, double gia_bia) {
    RentalSlip slip;
    slip.id_phieu = get_next_rental_id(); // Lấy ID tự tăng từ đĩa (Repository)
    slip.id_truyen = id_truyen;
    slip.id_khach_hang = id_khach_hang;
    slip.ngay_muon = ngay_muon;
    slip.ngay_tra_du_kien = ngay_tra_du_kien;
    slip.ngay_tra_thuc_te = {0, 0, 0}; // Gán mặt định chưa trả
    
    slip.tien_coc = gia_bia; // 100% giá bìa - Thu hồi khi bắt đầu tạo phiếu
    slip.tong_tien = 0;
    slip.trang_thai = 0;     // Đang thuê

    // Gọi Repository để lưu cấu trúc ghi liền xuống disk
    save_rental_slip(slip);
}

// Tính toán hóa đơn
void compute_payment_bill(RentalSlip& slip, double gia_bia) {
// 1. Trường hợp làm mất hoặc hư hỏng nặng
    if (slip.trang_thai == 2) {
        // Phạt đền 100% giá bìa (Đúng bằng số tiền cọc ban đầu slip.tien_coc)
        // Hệ thống chốt tổng tiền bằng giá bìa, khách không được thối lại tiền
        slip.tong_tien = gia_bia; 
        
        // Cập nhật trạng thái xuống đĩa và kết thúc hàm ngay lập tức
        update_rental_status(slip);
        return; 
    }
// 2. Trường hợp trả truyện bình thường 
    long so_ngay_thue = date_to_days(slip.ngay_tra_thuc_te) - date_to_days(slip.ngay_muon);
    if (so_ngay_thue < 1) {
        so_ngay_thue = 1; // Thuê trả ngay trong ngày tính 1
    }

    // Tiền thuê = số ngày * 10% giá bìa
    slip.tong_tien = so_ngay_thue * (TY_LE_THUE_THEO_NGAY * gia_bia);

    // Xử lý phạt mượn lố qua ngày
    long so_ngay_qua_han = date_to_days(slip.ngay_tra_thuc_te) - date_to_days(slip.ngay_tra_du_kien);
    if (so_ngay_qua_han > 0) {
        slip.tong_tien += (so_ngay_qua_han * PHI_PHAT_QUA_HAN_MOT_NGAY);
    }

    // (Thu tiền cọc hay trả lại tiền cọc sẽ do hàm/ui ở phần Thống Kê lo liệu)
    
    // Call hàm của Repo để ghi đè `seekp` nóng xuống đĩa 
    update_rental_status(slip);
}

// Xử lý trả khách hàng
void process_return_comic(int id_phieu, Date ngay_tra_thuc_te, int trang_thai_tra, double gia_bia) {
    // Để có dữ liệu phiếu, đọc file lướt qua xem tồn tại không
    ifstream inFile("data/rentals.dat", ios::binary);
    if (!inFile.is_open()) {
        cerr << "Loi: Khong mo duoc file rentals.dat.\n";
        return;
    }

    RentalSlip slip;
    bool found = false;
    while (inFile.read(reinterpret_cast<char*>(&slip), sizeof(RentalSlip))) {
        if (slip.id_phieu == id_phieu && slip.trang_thai == 0) {
            found = true;
            break; // Tim thay phieu va phieu dọ con trong that thai dang muon
        }
    }
    inFile.close();

    if (found) {
        slip.ngay_tra_thuc_te = ngay_tra_thuc_te; 
        slip.trang_thai = trang_thai_tra; // 1: Đã Trả Hoàn, 2: Làm Mất Hư Hỏng
        
        // Cầu nối giao tiếp 
        compute_payment_bill(slip, gia_bia);
    } else {
        cout << "Khong tim thay Phieu hoac sach nay khach da tra roi.\n";
    }
}
