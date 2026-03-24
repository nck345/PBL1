#include "../include/models/RentalSlip.h"
#include "../include/repository/RentalRepo.h"
#include <iostream>

using namespace std;

int main() {
    cout << "--- KIEM TRA QUAN LY PHIEU THUE ---\n" << endl;

    cout << "1. Kiem tra ID phieu ke tiep: " << get_next_rental_id() << endl;

    cout << "\n2. Tao va luu phieu thue..." << endl;
    RentalSlip slip1 = {get_next_rental_id(), 101, 1001, {1, 10, 2023}, {8, 10, 2023}, {0, 0, 0}, 20000.0, 0.0, 0};
    save_rental_slip(slip1);

    RentalSlip slip2 = {get_next_rental_id(), 102, 1002, {5, 10, 2023}, {12, 10, 2023}, {0, 0, 0}, 25000.0, 0.0, 0};
    save_rental_slip(slip2);

    cout << "\n3. Doc toan bo danh sach trong rentals.dat..." << endl;
    read_all_rental_slips();

    cout << "\n4. Cap nhat phieu id (" << slip1.id_phieu << ") - Khach hang da tra sach..." << endl;
    slip1.ngay_tra_thuc_te = {7, 10, 2023};
    slip1.tong_tien = 15000.0;
    slip1.trang_thai = 1; // 1: Đã trả
    update_rental_status(slip1);

    cout << "\n5. Kiem tra lai danh sach sau khi cap nhat..." << endl;
    read_all_rental_slips();

    return 0;
}
