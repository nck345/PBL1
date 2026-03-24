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

