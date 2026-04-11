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
    int current_d, current_m, current_y;
    get_current_date(current_d, current_m, current_y);

    Date today_system = {current_d, current_m, current_y};

    // --- State variables for Tab 1 (Daily) ---
    std::string day_str = "";
    std::string month_str = "";
    std::string year_str = "";
    std::string daily_revenue_str = "...";
    
    // --- State variables for Tab 2 (Monthly & Chart) ---
    std::string from_m_str = std::to_string(current_m);
    std::string from_y_str = std::to_string(current_y);
    std::string to_m_str = std::to_string(current_m);
    std::string to_y_str = std::to_string(current_y);
    std::string monthly_revenue_str = "...";
    std::vector<int> chart_data = get_revenue_chart_data(current_m, current_y);
    
    // UI Elements for Tab 1
    Component input_day = Input(&day_str, "DD");
    Component input_month = Input(&month_str, "MM");
    Component input_year = Input(&year_str, "YYYY");

    auto btn_calc_daily = Button(" TÍNH DOANH THU NGÀY ", [&] {
        int d = current_d, m = current_m, y = current_y;
        try {
            if (!day_str.empty()) d = std::stoi(day_str);
            if (!month_str.empty()) m = std::stoi(month_str);
            if (!year_str.empty()) y = std::stoi(year_str);
        } catch(...) {} // Ignore invalid format
        
        Date target_date = {d, m, y};
        rental_statistics stats = compute_all_statistics(target_date, current_m, current_y);
        daily_revenue_str = format_currency(stats.daily_revenue);
    }, ButtonOption::Ascii());

    // UI Elements for Tab 2
    auto input_fm = Input(&from_m_str, "MM");
    auto input_fy = Input(&from_y_str, "YYYY");
    auto input_tm = Input(&to_m_str, "MM");
    auto input_ty = Input(&to_y_str, "YYYY");

    auto btn_calc_monthly = Button(" TÍNH GIAI ĐOẠN & VẼ BIỂU ĐỒ ", [&] {
        int fm = current_m, fy = current_y, tm = current_m, ty = current_y;
        try {
            if (!from_m_str.empty()) fm = std::stoi(from_m_str);
            if (!from_y_str.empty()) fy = std::stoi(from_y_str);
            if (!to_m_str.empty()) tm = std::stoi(to_m_str);
            if (!to_y_str.empty()) ty = std::stoi(to_y_str);
        } catch(...) {}
        
        double total = compute_revenue_between_months(fm, fy, tm, ty);
        monthly_revenue_str = format_currency(total);
        chart_data = get_revenue_chart_data(tm, ty); // Show chart of "To" month
    }, ButtonOption::Ascii());

    // Initialization
    rental_statistics initial_stats = compute_all_statistics(today_system, current_m, current_y);
    daily_revenue_str = format_currency(initial_stats.daily_revenue);
    monthly_revenue_str = format_currency(compute_revenue_between_months(current_m, current_y, current_m, current_y));

    // Dữ liệu tĩnh Cache cho Tab 3
    std::vector<RentalSlip> slips = get_all_rental_slips();
    std::vector<Comic> all_c = read_all_comics();
    std::vector<Customer> all_cu = read_all_customers();
    int total_rented = 0;
    
    auto generate_main_table = [&]() {
        if (slips.empty()) return text("Không có dữ liệu phiếu thuê.") | center;
        std::vector<RentalSlip> sorted_slips = slips;
        quick_sort(sorted_slips, compare_revenue_desc);
        std::vector<std::vector<std::string>> data;
        data.push_back({" ID ", " Tên Truyện ", " Khách Hàng ", " Ngày Mượn ", " Hạn Trả ", " Thực Tế ", " Tiền Cọc ", " Tổng Tiền ", " Trạng Thái "});
        total_rented = 0;
        for (const auto &s : sorted_slips) {
            std::string ngay_m = std::to_string(s.ngay_muon.day) + "/" + std::to_string(s.ngay_muon.month);
            std::string ngay_d = std::to_string(s.ngay_tra_du_kien.day) + "/" + std::to_string(s.ngay_tra_du_kien.month);
            std::string ngay_t = (s.ngay_tra_thuc_te.year > 1900) ? (std::to_string(s.ngay_tra_thuc_te.day) + "/" + std::to_string(s.ngay_tra_thuc_te.month)) : "---";
            std::string tt = (s.trang_thai == 1) ? "Đã Trả" : (s.trang_thai == 2) ? "Mất/Hỏng" : "Đang Thuê";
            if (s.trang_thai == 0) total_rented++;
            data.push_back({
                std::to_string(s.id_phieu), truncate_text(get_c_name(s.comic_id, all_c), 20), truncate_text(get_cu_name(s.customer_id, all_cu), 20),
                ngay_m, ngay_d, ngay_t, format_currency(s.tien_coc), format_currency(s.tong_tien), tt
            });
        }
        auto tbl = Table(data);
        tbl.SelectAll().SeparatorVertical();
        tbl.SelectRow(0).Decorate(bold);
        tbl.SelectRow(0).SeparatorHorizontal();
        tbl.SelectAll().Border(LIGHT);
        return vbox({
             text(" Đang thuê: " + std::to_string(total_rented) + " phiếu ") | bold | color(Color::Cyan) | center,
             tbl.Render() | center
        });
    };
    
    Element table_cache = generate_main_table(); // Cache render để không giật lag

    int tab_selected = 0;
    std::vector<std::string> tab_entries = {
        "1. Doanh Thu Ngày",
        "2. Doanh Thu Giai Đoạn",
        "3. Dữ Liệu Chi Tiết"
    };
    auto tab_menu = Menu(&tab_entries, &tab_selected);

    // Containers (must be kept alive holding focus)
    auto container_tab1 = Container::Vertical({
        Container::Horizontal({ input_day, input_month, input_year }),
        btn_calc_daily
    });
    auto container_tab2 = Container::Vertical({
        Container::Horizontal({ input_fm, input_fy }),
        Container::Horizontal({ input_tm, input_ty }),
        btn_calc_monthly
    });

    auto main_container = Container::Vertical({
        tab_menu,
        Container::Tab({ container_tab1, container_tab2, Renderer([]{return text("");}) }, &tab_selected)
    });

    auto graph_func = [&](int width, int height) {
        if (chart_data.empty()) return std::vector<int>(width, 0);
        std::vector<int> out(width, 0);
        for (int i = 0; i < width; ++i) {
            int day = (i * 31) / std::max(width, 1) + 1;
            if (day >= 1 && day <= 31 && day < chart_data.size()) {
                out[i] = chart_data[day];
            }
        }
        return out;
    };

    auto renderer = Renderer(main_container, [&] {
        Element content;
        if (tab_selected == 0) {
            content = vbox({
                text(" CHỌN MỐC TÍNH DOANH THU ") | bold | center,
                separator(),
                text(" * Để trống sẽ tự động lấy ngày hệ thống ") | dim | center,
                text(" Hôm nay là: " + std::to_string(current_d) + "/" + std::to_string(current_m) + "/" + std::to_string(current_y)) | dim | center,
                hbox({ text(" | Ngày: "), input_day->Render() | size(WIDTH, EQUAL, 5), 
                       text("  | Tháng: "), input_month->Render() | size(WIDTH, EQUAL, 5), 
                       text("  | Năm: "), input_year->Render() | size(WIDTH, EQUAL, 7) }) | center | border,
                btn_calc_daily->Render() | center,
                separator(),
                vbox({
                    text(" KẾT QUẢ DOANH THU NGÀY: ") | bold | center,
                    text(daily_revenue_str + " VNĐ") | bold | color(Color::Green) | center
                }) | border
            });
        } else if (tab_selected == 1) {
            content = vbox({
                text(" KHOẢNG THỜI GIAN THỐNG KÊ ") | bold | center,
                separator(),
                hbox({ text(" Từ Tháng: "), input_fm->Render() | size(WIDTH, EQUAL, 5), text(" Năm: "), input_fy->Render() | size(WIDTH, EQUAL, 7) }) | center,
                hbox({ text(" Đến Tháng: "), input_tm->Render() | size(WIDTH, EQUAL, 5), text(" Năm: "), input_ty->Render() | size(WIDTH, EQUAL, 7) }) | center,
                btn_calc_monthly->Render() | center,
                separator(),
                vbox({
                     text(" TỔNG DOANH THU GIAI ĐOẠN: ") | bold | center,
                     text(monthly_revenue_str + " VNĐ") | bold | color(Color::Green) | center
                }),
                separator(),
                text(" BIỂU ĐỒ DOANH THU THÁNG " + to_m_str + "/" + to_y_str + " (VNĐ)") | bold | center,
                graph(std::ref(graph_func)) | flex | size(HEIGHT, GREATER_THAN, 12) | border
            });
        } else {
             // Re-render cached table inside frame for scrolling
            content = table_cache | vscroll_indicator | frame; 
        }

        return vbox({
            text(" BÁO CÁO TÀI CHÍNH ") | bold | center,
            separator(),
            hbox({
                tab_menu->Render() | border | size(WIDTH, LESS_THAN, 35),
                content | flex | border
            }) | flex,
            separator(),
            text(" [Nhấn Esc để thoát màn hình thống kê] ") | dim | center
        });
    });

    auto renderer_with_exit = CatchEvent(renderer, [&](Event event) {
        if (event == Event::Escape) {
            screen.ExitLoopClosure()();
            return true;
        }
        return false;
    });

    screen.Loop(renderer_with_exit);
}

