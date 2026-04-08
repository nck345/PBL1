#include "../../../include/ui/rental/RentalStatisticsUI.h"
#include "../../../include/ui/RentalUI.h"
#include "../../../include/repository/RentalRepo.h"
#include "../../../include/repository/ComicRepo.h"
#include "../../../include/services/RentalService.h"
#include "../../../include/utils/InputHandler.h"
#include "../../../include/utils/ValidationUtils.h"
#include "../../../include/utils/SortUtils.h"
#include "../../../include/ui/ComicUI.h"
#include "../../../include/ui/CustomerUI.h"
#include "../../../include/repository/CustomerRepo.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/terminal.hpp>

using namespace ftxui;

void render_statistics_screen() {
    auto screen = ScreenInteractive::TerminalOutput();
    std::vector<RentalSlip> slips = get_all_rental_slips();

    if (slips.empty()) {
        std::cout << "Khong co du lieu thong ke.\n";
        get_string_input("Nhan Enter de tiep tuc...");
        return;
    }

    Date today_test = {24, 3, 2026};

    // --- Sắp xếp danh sách chính theo doanh thu ---
    quick_sort(slips, compare_revenue_desc);

    std::vector<Comic> all_c = read_all_comics();
    std::vector<Customer> all_cu = read_all_customers();

    std::vector<std::vector<std::string>> main_table_data;
    main_table_data.push_back({"  ID  ", "       Tên Truyện       ", "      Khách Hàng      ", " Ngày Mượn ", "  Hạn Trả  ", "  Thực Tế  ", "  Tiền Cọc  ", "  Tổng Tiền  ", " Trạng Thái "});
    for (const auto &s : slips) {
        std::string ngay_m = std::to_string(s.ngay_muon.day) + "/" + std::to_string(s.ngay_muon.month);
        std::string ngay_d = std::to_string(s.ngay_tra_du_kien.day) + "/" + std::to_string(s.ngay_tra_du_kien.month);
        std::string ngay_t = (s.ngay_tra_thuc_te.year > 1900) ? (std::to_string(s.ngay_tra_thuc_te.day) + "/" + std::to_string(s.ngay_tra_thuc_te.month)) : "---";
        std::string tt = (s.trang_thai == 1) ? "Đã Trả" : (s.trang_thai == 2) ? "Mất/Hỏng" : "Đang Thuê";

        std::string cu_name = get_cu_name(s.customer_id, all_cu);
        std::string c_name = get_c_name(s.comic_id, all_c);

        main_table_data.push_back({
            std::to_string(s.id_phieu), truncate_text(c_name, 25), truncate_text(cu_name, 25), ngay_m, ngay_d, ngay_t,
            format_currency(s.tien_coc), format_currency(s.tong_tien), tt
        });
    }

    // --- Lọc và sắp xếp danh sách quá hạn ---
    std::vector<RentalSlip> overdue_slips;
    for (const auto& s : slips) {
        if (s.trang_thai == 0 && date_to_days(today_test) > date_to_days(s.ngay_tra_du_kien)) {
            overdue_slips.push_back(s);
        }
    }
    quick_sort(overdue_slips, compare_overdue_priority_desc);

    std::vector<std::vector<std::string>> overdue_table_data;
    overdue_table_data.push_back({"  ID  ", "       Tên Truyện       ", "      Khách Hàng      ", "  Hạn Trả  ", " Số Ngày Trễ "});
    for (const auto& s : overdue_slips) {
        std::string delay_str = "N/A";
        if (s.ngay_tra_du_kien.year > 1900) {
            long delay = date_to_days(today_test) - date_to_days(s.ngay_tra_du_kien);
            delay_str = std::to_string(delay) + " ngày";
        } else {
            delay_str = "Lỗi dữ liệu";
        }
        
        std::string cu_name = get_cu_name(s.customer_id, all_cu);
        std::string c_name = get_c_name(s.comic_id, all_c);
        
        overdue_table_data.push_back({
            std::to_string(s.id_phieu), truncate_text(c_name, 25), truncate_text(cu_name, 25),
          std::to_string(s.ngay_tra_du_kien.day) + "/" + std::to_string(s.ngay_tra_du_kien.month),
            delay_str
        });
    }

    auto main_tbl = Table(main_table_data);
    main_tbl.SelectAll().SeparatorVertical();
    rental_statistics stats = compute_all_statistics(today_test, today_test.month, today_test.year);

    // Dựng bản vẽ bảng một lần duy nhất để tránh lỗi mất dữ liệu khi vẽ lại
    auto main_tbl_obj = Table(main_table_data);
    main_tbl_obj.SelectAll().SeparatorVertical();
    main_tbl_obj.SelectRow(0).Decorate(bold);
    main_tbl_obj.SelectRow(0).SeparatorHorizontal();
    main_tbl_obj.SelectAll().Border(LIGHT);
    auto main_tbl_element = main_tbl_obj.Render();

    auto ovd_tbl_obj = Table(overdue_table_data);
    ovd_tbl_obj.SelectAll().SeparatorVertical();
    ovd_tbl_obj.SelectRow(0).Decorate(bold);
    ovd_tbl_obj.SelectRow(0).SeparatorHorizontal();
    ovd_tbl_obj.SelectAll().Border(LIGHT);
    ovd_tbl_obj.SelectAll().Decorate(color(Color::Red));
    auto ovd_tbl_element = ovd_tbl_obj.Render();

    auto renderer = Renderer([&] {
        return vbox({
            text(" BÁO CÁO THỐNG KÊ DOANH THU ") | bold | center,
            separator(),
            
            text(" 1. THỐNG KÊ DOANH THU CHI TIẾT (Sắp xếp theo tiền) ") | bold | center,
            main_tbl_element | center,
            
            text(" 2. DANH SÁCH PHIẾU QUÁ HẠN (Ưu tiên đòi sách) ") | bold | color(Color::Red) | center,
            overdue_table_data.size() > 1 
                ? ovd_tbl_element | center 
                : text(" (Không có phiếu nào quá hạn) ") | center | dim,

            separator(),
            hbox({
                text(" Doanh thu ngày (24/03): " + std::string(format_currency(stats.daily_revenue))),
                filler(),
                text(" Tổng khách đang thuê: " + std::to_string(stats.rented_count))
            }) | border | color(Color::Cyan),
            
            text(" [Bấm Enter hoặc Esc để quay lại] ") | center | dim
        });
    });

    auto renderer_with_exit = CatchEvent(renderer, [&](Event event) {
        if (event == Event::Return || event == Event::Escape) {
            screen.ExitLoopClosure()();
            return true;
        }
        return false;
    });

    screen.Loop(renderer_with_exit);
}

