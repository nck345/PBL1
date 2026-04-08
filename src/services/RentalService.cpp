#include "../../include/services/RentalService.h"
#include "../../include/models/Customer.h"
#include "../../include/repository/ComicRepo.h"
#include "../../include/repository/CustomerRepo.h"
#include "../../include/repository/RentalRepo.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <ctime>

using namespace std;

bool is_leap_year(int year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

long date_to_days(Date d) {
  long total_days = 0;

  // Tính tổng ngày của các NĂM trọn vẹn trước đó (từ năm 1 đến year - 1)
  int y = d.year - 1;
  // Mỗi năm 365 ngày + Cộng thêm 1 ngày cho mỗi năm nhuận đã trôi qua
  total_days = y * 365 + y / 4 - y / 100 + y / 400;

  // Tính tổng ngày của các THÁNG trọn vẹn trước đó (trong năm hiện tại)
  //  Mảng lưu số ngày của 12 tháng (tháng 0 bỏ trống để index chạy từ 1 đến 12
  //  cho dễ nhìn)
  int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  // Nếu năm nay là năm nhuận, tháng 2 có 29 ngày
  if (is_leap_year(d.year)) {
    days_in_month[2] = 29;
  }

  // Cộng dồn ngày của các tháng trước tháng d.month
  for (int m = 1; m < d.month; m++) {
    total_days += days_in_month[m];
  }

  // Cộng thêm số ngày của tháng hiện tại
  total_days += d.day;

  return total_days;
}

Date add_days(Date d, int days_to_add) {
    struct tm t = {0};
    t.tm_year = d.year - 1900;
    t.tm_mon = d.month - 1;
    t.tm_mday = d.day;
    t.tm_hour = 12; // To avoid DST issues
    t.tm_min = 0;
    t.tm_sec = 0;
    t.tm_mday += days_to_add;
    
    // Su dung mktime de tu dong can bang ngay thang
    mktime(&t);
    
    Date result;
    result.day = t.tm_mday;
    result.month = t.tm_mon + 1;
    result.year = t.tm_year + 1900;
    return result;
}

// Xử lý mượn truyện
void process_new_rental(int comic_id, int customer_id, Date ngay_tra_du_kien, double tien_coc, double tien_thue) {
  // 1. Kiem tra ton kho va lay thong tin sach theo ten
  Comic comic;
  if (!get_comic_by_id(comic_id, comic)) {
    cout << "Loi: Khong tim thay truyen!\n";
    return;
  }

  if (comic.is_deleted) {
    cout << "Loi: Truyen nay da bi xoa khoi he thong!\n";
    return;
  }
  if (comic.quantity <= 0) {
    cout << "Loi: Truyen nay da het hang trong kho (khong the cho thue)!\n";
    return;
  }

  // 2. Kiem tra khach hang co ton tai khong
  Customer customer;
  if (!get_customer_by_id(customer_id, customer)) {
    cout << "Loi: Khong tim thay khach hang!\n";
    return;
  }

  // Auto generate current date
  time_t t = time(0);
  tm* now = localtime(&t);
  Date ngay_muon = {now->tm_mday, now->tm_mon + 1, now->tm_year + 1900};

  // 3. Kiem tra logic ngay: ngay_tra_du_kien phai >= ngay_muon
  if (date_to_days(ngay_tra_du_kien) < date_to_days(ngay_muon)) {
    cout << "Loi: Ngay tra du kien (";
    cout << ngay_tra_du_kien.day << "/" << ngay_tra_du_kien.month << "/" << ngay_tra_du_kien.year;
    cout << ") khong the som hon ngay hien tai (";
    cout << ngay_muon.day << "/" << ngay_muon.month << "/" << ngay_muon.year;
    cout << ").\n";
    return;
  }

  // 4. Kiem tra trung phieu thue dang hoat dong
  if (is_rental_duplicate(comic_id, customer_id)) {
    cout << "Loi: Khach hang dang co phieu thue cuon nay chua tra!\n";
    cout << "     Vui long tra truyen cu truoc khi thue lai.\n";
    return;
  }

  // 5. Tien hanh tru so luong sach do xuat kho (chi tru sau khi qua tat ca kiem tra)
  comic.quantity -= 1;
  if (!update_comic(comic)) {
    cout << "Loi: Khong the cap nhat so luong truyen vao kho!\n";
    return;
  }

  // 6. Lap Phieu
  RentalSlip slip;
  slip.id_phieu = get_next_rental_id();
  slip.comic_id = comic_id;
  slip.customer_id = customer_id;
  slip.ngay_muon = ngay_muon;
  slip.ngay_tra_du_kien = ngay_tra_du_kien;
  slip.ngay_tra_thuc_te = {0, 0, 0};

  slip.tien_coc = tien_coc;
  slip.tong_tien = tien_thue;
  slip.trang_thai = 0;

  save_rental_slip(slip);
  cout << "Tao phieu thue thanh cong! Ma phieu: " << slip.id_phieu << "\n";
}

// Tính toán hóa đơn
void compute_payment_bill(RentalSlip &slip, double gia_bia, int trang_thai_tra) {
  double phat_hu_hong = 0.0;
  if (trang_thai_tra == 2) {
    phat_hu_hong = 0.2 * gia_bia;
  } else if (trang_thai_tra == 3) {
    phat_hu_hong = 0.5 * gia_bia;
  } else if (trang_thai_tra == 4) {
    phat_hu_hong = slip.tien_coc; // Mat 100% tien coc
  }

  double phat_tre_han = 0.0;
  long so_ngay_qua_han = date_to_days(slip.ngay_tra_thuc_te) - date_to_days(slip.ngay_tra_du_kien);
  if (so_ngay_qua_han > 0) {
    phat_tre_han = so_ngay_qua_han * (0.10 * gia_bia); // Phạt mỗi ngày trễ 10% giá bìa
  }

  // tong_tien ban đầu chứa tiền thuê dự kiến. Giờ cộng thêm phần phạt để ra tổng doanh thu thu được từ phiếu
  double phi_goc = slip.tong_tien;
  slip.tong_tien += (phat_hu_hong + phat_tre_han);

  // Khách chỉ mới đóng tiền cọc. Trừ đi tổng phí (tiền thuê + phạt) là ra số tiền còn lại hoàn trả khách.
  double tien_hoan_tra = slip.tien_coc - slip.tong_tien;
  
  // Giới hạn không để khách đóng thêm tiền quá mức tiền cọc (coi như bán truyện)
  bool mien_tru = false;
  if (tien_hoan_tra < 0) {
      tien_hoan_tra = 0;
      slip.tong_tien = slip.tien_coc; // Doanh thu thực tế chỉ bằng tiền cọc đã thu
      mien_tru = true;
  }

  cout << "\n============= HOA DON TRA TRUYEN =============\n";
  cout << " Tien coc da thu    : " << slip.tien_coc << " VND\n";
  cout << " Phi thue truyen    : -" << phi_goc << " VND\n";
  if (phat_hu_hong > 0) {
      cout << " Phat hu hong/mat   : -" << phat_hu_hong << " VND\n";
  }
  if (so_ngay_qua_han > 0) {
      cout << " Phat tre " << so_ngay_qua_han << " ngay     : -" << phat_tre_han << " VND\n";
  }
  if (mien_tru) {
      cout << " (Mien tru tien phat vuot muc coc do truot gia)\n";
  }
  cout << "----------------------------------------------\n";
  cout << " >>> BOI HOAN CHO KHACH: " << tien_hoan_tra << " VND\n";
  cout << "==============================================\n";

  // Call hàm của Repo để ghi đè `seekp` nóng xuống đĩa
  update_rental_status(slip);
}

// Xử lý trả khách hàng
void process_return_comic(int id_phieu, Date ngay_tra_thuc_te,
                          int trang_thai_tra, double gia_bia) {
  // Để có dữ liệu phiếu, đọc file lướt qua xem tồn tại không
  ifstream inFile("data/rentals.dat", ios::binary);
  if (!inFile.is_open()) {
    cerr << "Loi: Khong mo duoc file rentals.dat.\n";
    return;
  }

  RentalSlip slip;
  bool found = false;
  while (inFile.read(reinterpret_cast<char *>(&slip), sizeof(RentalSlip))) {
    if (slip.id_phieu == id_phieu && slip.trang_thai == 0) {
      found = true;
      break;
    }
  }
  inFile.close();

  if (found) {
    slip.ngay_tra_thuc_te = ngay_tra_thuc_te;
    slip.trang_thai = trang_thai_tra; // 1: Đã Trả Hoàn, 2: Làm Mất Hư Hỏng

    Comic comic;
    if (get_comic_by_id(slip.comic_id, comic)) {
      gia_bia = comic.price;

      if (trang_thai_tra == 1) { // 1: Binh thuong
        comic.quantity += 1;
        update_comic(comic);
      } else if (trang_thai_tra == 2) { // 2: Rach
        comic.quantity += 1;
        comic.price *= 0.8;
        update_comic(comic);
      } else if (trang_thai_tra == 3) { // 3: Mat trang
        comic.quantity += 1;
        comic.price *= 0.5;
        update_comic(comic);
      } else if (trang_thai_tra == 4) { // 4: Mat hong toan bo
        comic.total_quantity -= 1;
        if (comic.total_quantity < 0) comic.total_quantity = 0;
        update_comic(comic);
      }
    } else {
      // Truyen da bi xoa khoi he thong hoac khong tim thay theo ID.
      // Fallback: dung tien_coc da luu trong phieu lam gia_bia
      if (slip.tien_coc > 0) {
        gia_bia = slip.tien_coc;
        cout << "Canh bao: Khong tim thay truyen. Su dung tien coc da thu ("
             << gia_bia << " VND) lam gia bia de tinh toan.\n";
      } else {
        cout << "Loi nghiem trong: Phieu ID " << id_phieu
             << " khong the tinh toan vi tien_coc = 0. Lien he quan tri vien.\n";
        return;
      }
    }

    compute_payment_bill(slip, gia_bia, trang_thai_tra);
  } else {
    cout << "Khong tim thay Phieu hoac sach nay khach da tra roi.\n";
  }
}

// Thong ke & Bao cao
bool compare_revenue_desc(const RentalSlip& a, const RentalSlip& b) {
    return a.tong_tien > b.tong_tien;
}

bool compare_overdue_priority_desc(const RentalSlip& a, const RentalSlip& b) {
    return date_to_days(a.ngay_tra_du_kien) < date_to_days(b.ngay_tra_du_kien);
}

rental_statistics compute_all_statistics(Date today, int target_month,
                                         int target_year) {
  rental_statistics stats = {0.0, 0.0, 0, 0};

  ifstream file("data/rentals.dat", ios::binary);
  if (!file.is_open())
    return stats;

  RentalSlip slip;
  while (file.read(reinterpret_cast<char *>(&slip), sizeof(RentalSlip))) {
    // Dem so luong sach
    if (slip.trang_thai == 0) {
      stats.rented_count++;
    } else if (slip.trang_thai == 4 || slip.trang_thai == 2 /* backward compat */) {
      stats.lost_count++;
    }

    // Tinh doanh thu
    if (slip.trang_thai != 0) {
      // Doanh thu ngay
      if (slip.ngay_tra_thuc_te.day == today.day &&
          slip.ngay_tra_thuc_te.month == today.month &&
          slip.ngay_tra_thuc_te.year == today.year) {
        stats.daily_revenue += slip.tong_tien;
      }
      // Doanh thu thang
      if (slip.ngay_tra_thuc_te.month == target_month &&
          slip.ngay_tra_thuc_te.year == target_year) {
        stats.monthly_revenue += slip.tong_tien;
      }
    }
  }

  file.close();
  return stats;
}

void find_overdue_slips(RentalSlip overdue_list[], int &count, int max_size,
                        Date today) {
  ifstream file("data/rentals.dat", ios::binary);
  count = 0;
  if (!file.is_open())
    return;

  long today_days = date_to_days(today);
  RentalSlip slip;
  while (file.read(reinterpret_cast<char *>(&slip), sizeof(RentalSlip))) {
    if (slip.trang_thai == 0) {
      long due_days = date_to_days(slip.ngay_tra_du_kien);
      if (today_days > due_days) {
        if (count < max_size) {
          overdue_list[count++] = slip;
        }
      }
    }
  }
  file.close();
}
