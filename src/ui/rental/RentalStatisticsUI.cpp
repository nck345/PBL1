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
    std::string peak_info_str = "---";   // "Ngày X (YYY VNĐ)" hoặc "Tháng X (YYY VNĐ)"
    bool chart_multi_month = false;       // false = chế độ 1 tháng, true = nhiều tháng
    std::vector<int> chart_data = get_revenue_chart_data(current_m, current_y);
    int chart_fm = current_m, chart_fy = current_y; // lưu từ/đến cho generate_custom_chart
    int chart_tm = current_m, chart_ty = current_y;

    // Helper: số ngày thực tế trong tháng
    auto get_days_in_month = [](int m, int y) -> int {
        if (m == 2) {
            bool leap = (y % 400 == 0) || (y % 4 == 0 && y % 100 != 0);
            return leap ? 29 : 28;
        }
        if (m == 4 || m == 6 || m == 9 || m == 11) return 30;
        return 31;
    };

    // Helper: tính đỉnh từ chart_data (dùng chung cả 2 chế độ)
    auto compute_peak = [&]() {
        int peak_idx = 0;
        int peak_val = 0;
        for (int i = 0; i < (int)chart_data.size(); ++i) {
            if (chart_data[i] > peak_val) {
                peak_val = chart_data[i];
                peak_idx = i;
            }
        }
        if (peak_val == 0) { peak_info_str = "---"; return; }
        if (!chart_multi_month) {
            // Chế độ 1 tháng: peak_idx là ngày (1-based trong chart_data)
            peak_info_str = "Ngày " + std::to_string(peak_idx) + " (" + format_currency(peak_val) + " VNĐ)";
        } else {
            // Chế độ nhiều tháng: peak_idx là offset từ chart_fm
            int pm = chart_fm + peak_idx;
            int py = chart_fy + (pm - 1) / 12;
            pm = (pm - 1) % 12 + 1;
            peak_info_str = "Tháng " + std::to_string(pm) + "/" + std::to_string(py)
                          + " (" + format_currency(peak_val) + " VNĐ)";
        }
    };
    compute_peak();
    
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

        chart_fm = fm; chart_fy = fy; chart_tm = tm; chart_ty = ty;
        // Tự động chọn chế độ biểu đồ
        chart_multi_month = !((fy == ty) && (fm == tm));
        if (!chart_multi_month) {
            chart_data = get_revenue_chart_data(tm, ty); // Chế độ ngày
        } else {
            chart_data = get_monthly_chart_data(fm, fy, tm, ty); // Chế độ tháng
        }
        compute_peak();
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
        std::vector<RentalSlip> sorted_slips;
        total_rented = 0;
        for (const auto& s : slips) {
            if (s.trang_thai == 0) total_rented++;
            if (s.trang_thai == 1) { // Chỉ lấy phiếu Đã Trả
                sorted_slips.push_back(s);
            }
        }

        quick_sort(sorted_slips, compare_revenue_desc);
        std::vector<std::vector<std::string>> data;
        data.push_back({" ID ", " Tên Truyện ", " Khách Hàng ", " Ngày Mượn ", " Hạn Trả ", " Thực Tế ", " Tổng Tiền "});
        
        if (sorted_slips.empty()) {
            data.push_back({"---", "---", "---", "---", "---", "---", "---"});
        } else {
            for (const auto &s : sorted_slips) {
                std::string ngay_m = std::to_string(s.ngay_muon.day) + "/" + std::to_string(s.ngay_muon.month);
                std::string ngay_d = std::to_string(s.ngay_tra_du_kien.day) + "/" + std::to_string(s.ngay_tra_du_kien.month);
                std::string ngay_t = (s.ngay_tra_thuc_te.year > 1900) ? (std::to_string(s.ngay_tra_thuc_te.day) + "/" + std::to_string(s.ngay_tra_thuc_te.month)) : "---";
                
                data.push_back({
                    std::to_string(s.id_phieu), truncate_text(get_c_name(s.comic_id, all_c), 20), truncate_text(get_cu_name(s.customer_id, all_cu), 20),
                    ngay_m, ngay_d, ngay_t, format_currency(s.tong_tien)
                });
            }
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
    tab_menu = CatchEvent(tab_menu, [&](Event event) {
        if (event.is_character()) {
            char c = event.character()[0];
            if (c >= '1' && c <= '3') {
                tab_selected = c - '1';
                return true;
            }
        }
        return false;
    });

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

    auto generate_custom_chart = [&]() -> Element {
        if (chart_data.empty()) return text("Chưa có dữ liệu biểu đồ.") | center;

        // ── Số điểm dữ liệu ──
        int num_points;
        if (!chart_multi_month) {
            num_points = get_days_in_month(chart_tm, chart_ty);
        } else {
            num_points = (int)chart_data.size();
        }
        if (num_points < 1) num_points = 1;

        // ── Chuẩn hoá dữ liệu sang Nghìn VNĐ ──
        double max_val_k = 0;
        std::vector<double> data_k(num_points, 0.0);
        for (int i = 0; i < num_points; ++i) {
            int src = chart_multi_month ? i : (i + 1);
            if (src < (int)chart_data.size()) {
                data_k[i] = chart_data[src] / 1000.0;
                if (data_k[i] > max_val_k) max_val_k = data_k[i];
            }
        }

        // ── Tính step chẵn cho trục Y ──
        if (max_val_k == 0) max_val_k = 10.0;
        int num_steps = 10;
        double raw_step = max_val_k / num_steps;
        double multiplier = 1.0;
        while (raw_step >= 10.0) { raw_step /= 10.0; multiplier *= 10.0; }
        while (raw_step < 1.0 && raw_step > 0) { raw_step *= 10.0; multiplier /= 10.0; }
        int nice_step_base = 1;
        if (raw_step > 5.0)      nice_step_base = 10;
        else if (raw_step > 2.0) nice_step_base = 5;
        else if (raw_step > 1.0) nice_step_base = 2;
        int step = (int)(nice_step_base * multiplier);
        if (step < 1) step = 1;

        // ── Tìm index đỉnh ──
        int peak_idx = -1;
        double peak_k = 0;
        for (int i = 0; i < num_points; ++i) {
            if (data_k[i] > peak_k) { peak_k = data_k[i]; peak_idx = i; }
        }

        // ── Xây dựng cột Y (trái, chiều rộng cố định) ──
        // Mỗi phần tử: "  50 |" — 5 ký tự số + " |" = 7 ký tự
        const int Y_COL_WIDTH = 7;
        Elements y_col;
        Elements bar_rows; // phần body cột phải (chỉ chứa bars)

        for (int row = num_steps; row >= 0; --row) {
            int thr = row * step;

            // Nhãn Y căn phải 5 ký tự + " |"
            std::string y_label = std::to_string(thr);
            while ((int)y_label.length() < 5) y_label = " " + y_label;
            y_col.push_back(
                hbox({ text(y_label) | color(Color::Cyan), text(" |") })
            );

            // Hàng bars — dùng filler() giữa các điểm để trải đều theo flex
            Elements bar;
            for (int i = 0; i < num_points; ++i) {
                if (i > 0) bar.push_back(filler());
                bool filled = (thr > 0) ? (data_k[i] >= thr) : (data_k[i] > 0);
                Color col = (i == peak_idx && filled) ? Color::Yellow : Color::Green;
                bar.push_back(filled ? (text("██") | color(col)) : text("  "));
            }
            bar_rows.push_back(hbox(bar));
        }

        // ── Xây dựng nhãn trục X ──
        Elements x_labels;
        if (!chart_multi_month) {
            // Chế độ ngày: mốc 1, 5, 10, 15, 20, 25, [ngày cuối]
            std::vector<int> ms = {1, 5, 10, 15, 20, 25, num_points};
            bool first = true;
            for (int d : ms) {
                if (d < 1 || d > num_points) continue;
                if (!first) x_labels.push_back(filler());
                x_labels.push_back(
                    text(std::to_string(d)) | color(Color::Yellow) | bold
                );
                first = false;
            }
        } else {
            // Chế độ tháng: T1, T2, ... Tháng cuối — xử lý đúng qua năm
            bool first = true;
            for (int i = 0; i < num_points; ++i) {
                int pm = chart_fm + i;
                int py = chart_fy + (pm - 1) / 12;
                pm = (pm - 1) % 12 + 1;
                (void)py; // năm không hiển thị trong nhãn tháng ngắn
                if (!first) x_labels.push_back(filler());
                x_labels.push_back(
                    text("T" + std::to_string(pm)) | color(Color::Yellow) | bold
                );
                first = false;
            }
        }

        // ── Lắp ráp layout ──
        // Trái: cột Y (chiều rộng cố định Y_COL_WIDTH)
        // Phải: vbox(bars | flex, separator, x-labels) | flex
        //   → separator() bên trong vbox sẽ tự mở rộng hết chiều ngang của phần phải

        return hbox({
            // Cột Y — chiều rộng cố định
            vbox(y_col) | size(WIDTH, EQUAL, Y_COL_WIDTH),

            // Phần phải — flex, gồm 3 dòng trong vbox
            vbox({
                // Dòng 1: vùng bars, flex về chiều cao
                vbox(bar_rows) | flex,
                // Dòng 2: đường kẻ ngang — separator() trong vbox = ngang, full width!
                separator(),
                // Dòng 3: nhãn trục X rải đều
                hbox(x_labels)
            }) | flex
        });
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
            // Tiêu đề thời gian: 1 dòng nếu cùng tháng, 2 vế nếu khác
            bool same_period = (from_m_str == to_m_str && from_y_str == to_y_str);
            std::string period_label = same_period
                ? ("Thống kê Tháng " + to_m_str + "/" + to_y_str)
                : ("Từ Tháng " + from_m_str + " Năm " + from_y_str
                   + "   -   Đến Tháng " + to_m_str + " Năm " + to_y_str);
            std::string chart_title = same_period
                ? (" BIỂU ĐỒ THÁNG " + to_m_str + "/" + to_y_str + " (Nghìn VNĐ)")
                : (" BIỂU ĐỒ GIAI ĐOẠN " + from_m_str + "/" + from_y_str
                   + " → " + to_m_str + "/" + to_y_str + " (Nghìn VNĐ)");

            content = vbox({
                vbox({
                    text(" KHOẢNG THỜI GIAN THỐNG KÊ ") | bold | center,
                    separator(),
                    hbox({
                        text(" Từ Tháng: "), input_fm->Render() | size(WIDTH, EQUAL, 5),
                        text("  Năm: "), input_fy->Render() | size(WIDTH, EQUAL, 7),
                        filler(),
                        text(" Đến Tháng: "), input_tm->Render() | size(WIDTH, EQUAL, 5),
                        text("  Năm: "), input_ty->Render() | size(WIDTH, EQUAL, 7),
                        text(" ")
                    }),
                    btn_calc_monthly->Render() | center,
                    separator(),
                    text(" " + period_label + " ") | dim | center,
                    // Gom TỔNG & ĐỈNH trên 1 hàng để tiết kiệm chiều cao
                    hbox({
                        text(" TỔNG: ") | bold,
                        text(monthly_revenue_str + " VNĐ") | bold | color(Color::Green),
                        text("   │   "),
                        text("ĐỈNH: ") | bold,
                        text(peak_info_str) | bold | color(Color::RedLight)
                    }) | center,
                }),
                separator(),
                text(chart_title) | bold | center,
                generate_custom_chart() | border | flex
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

