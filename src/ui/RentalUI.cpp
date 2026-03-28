#include "../../include/ui/RentalUI.h"
#include "../../include/repository/RentalRepo.h"
#include "../../include/services/RentalService.h"
#include "../../include/utils/InputHandler.h"
#include "../../include/utils/ValidationUtils.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>

using namespace ftxui;

// Helper to parse date strings
Date parse_date_string(const std::string& date_str) {
    Date d = {0, 0, 0};
    if (date_str.length() >= 8) {
        sscanf(date_str.c_str(), "%d/%d/%d", &d.day, &d.month, &d.year);
    }
    return d;
}

void render_new_rental_screen() {
    system("cls");
    auto screen = ScreenInteractive::TerminalOutput();

    std::string truyen_id_str;
    std::string khach_id_str;
    std::string ngay_muon_str;
    std::string ngay_tra_str;

    Component input_truyen = Input(&truyen_id_str, "nhập số...");
    Component input_khach = Input(&khach_id_str, "nhập số...");
    Component input_ngay_muon = Input(&ngay_muon_str, "dd/mm/yyyy");
    Component input_ngay_tra = Input(&ngay_tra_str, "dd/mm/yyyy");

    auto submit_button = Button("Xác nhận & Cho thuê", screen.ExitLoopClosure(), ButtonOption::Animated());
    auto cancel_button = Button("Hủy & Trở về", screen.ExitLoopClosure(), ButtonOption::Animated());

    auto container = Container::Vertical({
        input_truyen,
        input_khach,
        input_ngay_muon,
        input_ngay_tra,
        Container::Horizontal({submit_button, cancel_button})
    });

    auto renderer = Renderer(container, [&] {
        return vbox({
            text(" THIET LAP PHIEU THUE ") | bold | center,
            separator(),
            hbox(text(" Truyện ID:      "), input_truyen->Render()),
            hbox(text(" Khách ID:       "), input_khach->Render()),
            hbox(text(" Ngày mượn:      "), input_ngay_muon->Render()),
            hbox(text(" Hạn trả (dự kiến): "), input_ngay_tra->Render()),
            separator(),
            hbox(submit_button->Render(), text("   "), cancel_button->Render()) | center
        }) | border;
    });

    screen.Loop(renderer);

    if (!truyen_id_str.empty() && !khach_id_str.empty() && !ngay_muon_str.empty()) {
        int t_id = 0, k_id = 0;
        try { t_id = std::stoi(truyen_id_str); } catch (...) {}
        try { k_id = std::stoi(khach_id_str); } catch (...) {}
        
        Date d_muon = parse_date_string(ngay_muon_str);
        Date d_tra = parse_date_string(ngay_tra_str);

        if (t_id > 0 && k_id > 0 && d_muon.year > 0) {
            system("cls");
            process_new_rental(t_id, k_id, d_muon, d_tra, 0.0);
            get_string_input("Nhan Enter de tiep tuc...");
        }
    }
}

void render_return_comic_screen() {
    system("cls");
    auto screen = ScreenInteractive::TerminalOutput();

    std::string phieu_id_str;
    std::string ngay_tra_str;
    std::string trang_thai_str; // 1: tra binh thuong, 2: mat hong

    Component input_phieu = Input(&phieu_id_str, "nhập số...");
    Component input_ngay = Input(&ngay_tra_str, "dd/mm/yyyy");
    Component input_tt = Input(&trang_thai_str, "1: Bình thường, 2: Mất/Hỏng");

    auto submit_button = Button("Xác nhận & Thanh toán", screen.ExitLoopClosure(), ButtonOption::Animated());
    auto cancel_button = Button("Hủy & Trở về", screen.ExitLoopClosure(), ButtonOption::Animated());

    auto container = Container::Vertical({
        input_phieu,
        input_ngay,
        input_tt,
        Container::Horizontal({submit_button, cancel_button})
    });

    auto renderer = Renderer(container, [&] {
        return vbox({
            text(" TRA TRUYEN & THANH TOAN ") | bold | center,
            separator(),
            hbox(text(" Phiếu ID:   "), input_phieu->Render()),
            hbox(text(" Ngày trả:   "), input_ngay->Render()),
            hbox(text(" Tình trạng: "), input_tt->Render()),
            separator(),
            hbox(submit_button->Render(), text("   "), cancel_button->Render()) | center
        }) | border;
    });

    screen.Loop(renderer);

    if (!phieu_id_str.empty() && !ngay_tra_str.empty() && !trang_thai_str.empty()) {
        int p_id = 0, tt = 1;
        try { p_id = std::stoi(phieu_id_str); } catch(...) {}
        try { tt = std::stoi(trang_thai_str); } catch(...) {}

        Date d_tra = parse_date_string(ngay_tra_str);
        if (p_id > 0 && d_tra.year > 0) {
            system("cls");
            process_return_comic(p_id, d_tra, tt, 0.0);
            get_string_input("Nhan Enter de tiep tuc...");
        }
    }
}

void render_rental_menu() {
    auto screen = ScreenInteractive::TerminalOutput();

    std::vector<std::string> entries = {
        "1. Cho thue truyen moi",
        "2. Tra truyen & Thanh toan",
        "3. Tro ve"
    };
    int selected = 0;

    MenuOption option;
    option.on_enter = screen.ExitLoopClosure();

    auto menu = Menu(&entries, &selected, option);
    auto renderer = Renderer(menu, [&] {
        return window(text(" QUAN LY PHIEU THUE "),
                      menu->Render() | vscroll_indicator | frame) | bold;
    });

    while (true) {
        screen.Loop(renderer);

        if (selected == 0) {
            render_new_rental_screen();
        } else if (selected == 1) {
            render_return_comic_screen();
        } else if (selected == 2) {
            system("cls");
            break;
        }
    }
}

void render_statistics_screen() {
    system("cls");
    std::vector<RentalSlip> slips = get_all_rental_slips();
    
    if (slips.empty()) {
        std::cout << "Khong co du lieu thong ke.\n";
        get_string_input("Nhan Enter de tiep tuc...");
        return;
    }

    std::vector<std::vector<std::string>> table_data;
    table_data.push_back({"ID", "Truyen", "Khach", "Ngay Muon", "Du Kien", "Thuc Te", "Tien Coc", "Tong Tien", "Trang Thai"});
    
    std::vector<int> warning_rows;
    
    int row_idx = 1;
    // Can hardcode today for project presentation or make a prompt. Let's assume today is when the presentation happens (e.g 24/3/2026)
    Date today_test = {24, 3, 2026};

    for (const auto& s : slips) {
        std::string ngay_m = std::to_string(s.ngay_muon.day) + "/" + std::to_string(s.ngay_muon.month) + "/" + std::to_string(s.ngay_muon.year);
        std::string ngay_d = std::to_string(s.ngay_tra_du_kien.day) + "/" + std::to_string(s.ngay_tra_du_kien.month) + "/" + std::to_string(s.ngay_tra_du_kien.year);
        std::string ngay_t = (s.ngay_tra_thuc_te.year > 0) ? (std::to_string(s.ngay_tra_thuc_te.day) + "/" + std::to_string(s.ngay_tra_thuc_te.month) + "/" + std::to_string(s.ngay_tra_thuc_te.year)) : "N/A";
        
        std::string tt = "Dang Thue";
        if (s.trang_thai == 1) tt = "Da Tra";
        else if (s.trang_thai == 2) tt = "Mat/Hong";
        else if (s.trang_thai == 3) tt = "Qua Han";
        
        // Cảnh báo màu đỏ
        if (s.trang_thai == 2 || s.trang_thai == 3 || 
           (s.trang_thai == 0 && date_to_days(today_test) > date_to_days(s.ngay_tra_du_kien))) {
           warning_rows.push_back(row_idx);
        }

        table_data.push_back({
            std::to_string(s.id_phieu),
            std::to_string(s.id_truyen),
            std::to_string(s.id_khach_hang),
            ngay_m, ngay_d, ngay_t,
            format_currency(s.tien_coc),
            format_currency(s.tong_tien),
            tt
        });
        row_idx++;
    }

    auto table = Table(table_data);
    table.SelectAll().Border(LIGHT);
    table.SelectRow(0).Decorate(bold);
    table.SelectRow(0).SeparatorVertical(LIGHT);
    table.SelectRow(0).Border(DOUBLE);

    for (int r : warning_rows) {
        table.SelectRow(r).Decorate(color(Color::Red));
    }

    rental_statistics stats = compute_all_statistics(today_test, today_test.month, today_test.year);
    
    auto stats_panel = vbox({
        text("--- TONG QUAN (" + std::to_string(today_test.day) + "/" + std::to_string(today_test.month) + ") ---") | bold,
        text("So sach dang thue: " + std::to_string(stats.rented_count)),
        text("So sach mat/hong:  " + std::to_string(stats.lost_count)),
        text("Doanh thu hom nay: " + std::string(format_currency(stats.daily_revenue))),
        text("Doanh thu trong thang: " + std::string(format_currency(stats.monthly_revenue)))
    }) | border;

    auto document = vbox({
        text(" THONG KE PHIEU THUE & DOANH THU ") | bold | center,
        separator(),
        stats_panel,
        text(" DANH SACH PHIEU THUE ") | bold,
        table.Render()
    });

    auto screen = Screen::Create(Dimension::Fit(document), Dimension::Fit(document));
    Render(screen, document);
    screen.Print();
    
    get_string_input("\nNhan Enter de tro ve...");
}
