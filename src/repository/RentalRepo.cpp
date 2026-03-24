#include "../../include/repository/RentalRepo.h"
#include <fstream>
#include <iostream>

using namespace std;

// In ra chuỗi ngày chuẩn từ struct Date
void print_date(Date d) {
  if (d.day == 0 && d.month == 0 && d.year == 0) {
    cout << "N/A";
    return;
  }
  cout << d.day << "/" << d.month << "/" << d.year;
}

// Ghi 1 struct RentalSlip vào cuối file nhị phân rentals.dat
void save_rental_slip(const RentalSlip &slip) {
  ofstream file("data/rentals.dat", ios::binary | ios::app);
  if (file.is_open()) {
    file.write(reinterpret_cast<const char *>(&slip), sizeof(RentalSlip));
    file.close();
    cout << "Luu phieu thue id: " << slip.id_phieu << " thanh cong!\n";
  } else {
    cout << "Loi: Khong the mo file data/rentals.dat de ghi.\n";
  }
}

// Đọc toàn bộ file rentals.dat và in ra terminal để kiểm tra
void read_all_rental_slips() {
  ifstream file("data/rentals.dat", ios::binary);
  if (!file.is_open()) {
    cout << "Loi: Khong the mo file data/rentals.dat de doc.\n";
    return;
  }

  RentalSlip slip;
  cout << "--- DANH SACH PHIEU THUE ---\n";
  while (file.read(reinterpret_cast<char *>(&slip), sizeof(RentalSlip))) {
    cout << "Phieu ID: " << slip.id_phieu << " | Truyen ID: " << slip.id_truyen
         << " | Khach ID: " << slip.id_khach_hang << " | Ngay thue: ";
    print_date(slip.ngay_muon);
    cout << " | Ngay du kien tra: ";
    print_date(slip.ngay_tra_du_kien);
    cout << " | Ngay tra thuc te: ";
    print_date(slip.ngay_tra_thuc_te);
    cout << " | Tien coc: " << slip.tien_coc
         << " | Tong Tien: " << slip.tong_tien
         << " | Trang thai: " << slip.trang_thai << "\n";
  }
  cout << "----------------------------\n";
  file.close();
}

void update_rental_status(const RentalSlip &updated_slip) {
  fstream file("data/rentals.dat", ios::in | ios::out | ios::binary);
  if (!file.is_open()) {
    cerr << "Loi: Khong the mo rentals.dat de cap nhat.\n";
    return;
  }

  RentalSlip slip;
  while (file.read(reinterpret_cast<char *>(&slip), sizeof(RentalSlip))) {
    if (slip.id_phieu == updated_slip.id_phieu) {
      file.seekp(static_cast<streampos>(file.tellg()) -
                 static_cast<streampos>(sizeof(RentalSlip)));
      file.write(reinterpret_cast<const char *>(&updated_slip),
                 sizeof(RentalSlip));
      break;
    }
  }
  file.close();
}

int get_next_rental_id() {
  ifstream inFile("data/rentals.dat", ios::in | ios::binary);
  if (!inFile.is_open()) {
    return 1;
  }

  inFile.seekg(0, ios::end);
  long length = inFile.tellg();
  if (length < static_cast<long>(sizeof(RentalSlip))) {
    inFile.close();
    return 1;
  }

  inFile.seekg(-static_cast<long>(sizeof(RentalSlip)), ios::end);
  RentalSlip last_slip;
  inFile.read(reinterpret_cast<char *>(&last_slip), sizeof(RentalSlip));
  inFile.close();

  return last_slip.id_phieu + 1;
}
