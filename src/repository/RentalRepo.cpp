#include "../../include/repository/RentalRepo.h"
#include <iostream>
#include <fstream>
#include <ctime>

// In ra chuỗi ngày chuẩn từ time_t (dd/mm/yyyy)
void print_date(time_t t) {
    if (t == 0) {
        std::cout << "N/A";
        return;
    }
    struct tm* tm_info = std::localtime(&t);
    char buffer[20];
    std::strftime(buffer, 20, "%d/%m/%Y", tm_info);
    std::cout << buffer;
}

// Ghi 1 struct RentalSlip vào cuối file nhị phân rentals.dat
void save_rental_slip(const RentalSlip& slip) {
    std::ofstream file("data/rentals.dat", std::ios::binary | std::ios::app);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(&slip), sizeof(RentalSlip));
        file.close();
        // Console log tạm thời để kiểm tra
        std::cout << "Luu phieu thue id: " << slip.id_phieu << " thanh cong!\n";
    } else {
        std::cout << "Loi: Khong the mo file data/rentals.dat de ghi.\n";
    }
}

// Đọc toàn bộ file rentals.dat và in ra terminal để kiểm tra
void read_all_rental_slips() {
    std::ifstream file("data/rentals.dat", std::ios::binary);
    if (file.is_open()) {
        RentalSlip slip;
        std::cout << "--- DANH SACH PHIEU THUE ---\n";
        while (file.read(reinterpret_cast<char*>(&slip), sizeof(RentalSlip))) {
            std::cout << "Phieu ID: " << slip.id_phieu 
                      << " | Truyen ID: " << slip.id_truyen 
                      << " | Khach ID: " << slip.id_khach_hang 
                      << " | Ngay thue: ";
            print_date(slip.ngay_muon);
            std::cout << " | Ngay du kien tra: ";
            print_date(slip.ngay_tra);
            std::cout << " | Tong Tien: " << slip.tong_tien 
                      << " | Trang thai: " << slip.trang_thai << "\n";
        }
        std::cout << "----------------------------\n";
        file.close();
    } else {
        std::cout << "Loi: Khong the mo file data/rentals.dat de doc.\n";
    }
}
