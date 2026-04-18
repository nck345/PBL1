#include <cstdlib>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

// Include UI headers
#include "../include/ui/ComicUI.h"
#include "../include/ui/CustomerUI.h"
#include "../include/ui/RentalUI.h"

// Include repositories to get real data
#include "../include/repository/ComicRepo.h"
#include "../include/repository/CustomerRepo.h"
#include "../include/repository/RentalRepo.h"
#include "../include/utils/ValidationUtils.h"

// Include FTXUI headers for the main menu
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "../include/ui/UITheme.h"

using namespace ftxui;

int main() {
  auto screen = ScreenInteractive::TerminalOutput();

  std::vector<std::string> entries = {
      " 1. Quản lý Truyện tranh   ", " 2. Quản lý Khách hàng     ",
      " 3. Mượn & Trả sách        ", " 4. Báo cáo & Thống kê     ",
      " 5. Thoát chương trình     "};
  int selected = 0;

  MenuOption option;
  option.on_enter = screen.ExitLoopClosure();

  auto menu = Menu(&entries, &selected, option);
  auto menu_with_event = CatchEvent(menu, [&](Event event) {
    if (event == Event::Escape) {
      selected = entries.size() - 1;
      screen.ExitLoopClosure()();
      return true;
    }
    if (event.is_character()) {
      char c = event.character()[0];
      if (c >= '1' && c <= '5') {
        int index = c - '1';
        if (index < (int)entries.size()) {
          selected = index;
          screen.ExitLoopClosure()();
          return true;
        }
      }
    }
    if (event.is_mouse() && event.mouse().button == ftxui::Mouse::Left &&
        event.mouse().motion == ftxui::Mouse::Pressed) {
      if (menu->OnEvent(event)) {
        if (option.on_enter) {
          option.on_enter();
        }
        return true;
      }
    }
    return false;
  });

  auto renderer = Renderer(menu_with_event, [&]() -> Element {
    // Tinh toan du lieu thuc te tu he thong
    std::vector<Comic> all_comics = read_all_comics();
    int active_comic_count = 0;
    int out_of_stock_count = 0;
    for (const auto &c : all_comics) {
      if (!c.is_deleted) {
        active_comic_count++;
        if (c.quantity <= 0)
          out_of_stock_count++;
      }
    }

    std::vector<RentalSlip> all_slips = get_all_rental_slips();
    int active_rental_count = 0;
    double total_revenue = 0.0;
    for (const auto &s : all_slips) {
      if (s.trang_thai == 0) {
        active_rental_count++;
      } else if (s.trang_thai == 1) {
        total_revenue +=
            s.tong_tien; // Tong doanh thu tu cac phieu da hoan thanh
      }
    }

    // --- SIDEBAR (Bên trái) ---
    auto sidebar =
        window(text(" MENU ĐIỀU HƯỚNG ") | bold | center,
               menu_with_event->Render() | vscroll_indicator | frame) |
        size(WIDTH, EQUAL, 32);

    // --- CÁC TEXT THỐNG KÊ ---
    auto stat1 = window(text(" TỔNG DOANH THU "),
                        text(" " + format_currency(total_revenue) + " ") |
                            bold | color(Color::Green) | center) |
                 flex;
    auto stat2 =
        window(text(" SỐ LƯỢNG TRUYỆN "),
               text(" " + std::to_string(active_comic_count) + " Cuốn ") |
                   bold | color(ui::theme::kPrimaryColor) | center) |
        flex;
    auto stat3 =
        window(text(" PHIẾU ĐANG THUÊ "),
               text(" " + std::to_string(active_rental_count) + " Phiếu ") |
                   bold | color(Color::Orange1) | center) |
        flex;

    // --- DASHBOARD LAYOUT (Bên phải) ---
    auto dashboard_metrics =
        hbox({stat1, stat2, stat3}) | size(HEIGHT, EQUAL, 5);

    auto system_alerts =
        window(text(" CẢNH BÁO HỆ THỐNG ") | bold |
                   color(ui::theme::kAccentColor),
               vbox({text(" [!] Có " + std::to_string(out_of_stock_count) +
                          " bộ truyện đang rơi vào tình trạng hết hàng."),
                     text(" [*] Luôn nhắc nhở nhân viên trực quầy sao lưu dữ "
                          "liệu cuối tuần.")}) |
                   color(ui::theme::kTextMutedColor)) |
        flex;

    auto quick_actions =
        window(
            text(" PHÍM TẮT NHANH ") | bold,
            hbox({text(" (1-5): Chọn Menu ") | color(ui::theme::kWarningColor),
                  text("   |   "),
                  text(" (Enter): Truy cập ") | color(ui::theme::kWarningColor),
                  text("   |   "),
                  text(" (ESC): Thoát app ") |
                      color(ui::theme::kWarningColor)}) |
                center) |
        size(HEIGHT, EQUAL, 4);

    auto main_dashboard =
        window(text(" TỔNG QUAN TÌNH HÌNH CỬA HÀNG HÔM NAY ") | bold | center,
               vbox({text("") | size(HEIGHT, EQUAL, 1), dashboard_metrics,
                     text("") | size(HEIGHT, EQUAL, 1), system_alerts,
                     quick_actions})) |
        flex;

    // --- KẾT HỢP HAI BÊN ---
    auto layout =
        hbox({sidebar, main_dashboard}) | ui::theme::FocusedPanel() | flex;

    auto title =
        text(" QUẢN LÝ THUÊ TRUYỆN TRANH ") | ui::theme::AppTitle() | center;

    return vbox({text("") | size(HEIGHT, EQUAL, 1), title,
                 text("") | size(HEIGHT, EQUAL, 1), layout,
                 text("") | size(HEIGHT, EQUAL, 1)}) |
           bgcolor(ui::theme::kBgColor) | borderEmpty | center;
  });

  while (true) {
    system("cls");
    // Run the FTXUI loop
    screen.Loop(renderer);

    // Process the user's selection
    if (selected == 0) {
      system("cls");
      // 1. Quan ly Truyen
      render_comic_menu();
    } else if (selected == 1) {
      system("cls");
      // 2. Quan ly Khach Hang
      render_customer_menu();
    } else if (selected == 2) {
      system("cls");
      // 3. Quan ly Phieu
      render_rental_menu();
    } else if (selected == 3) {
      system("cls");
      // 4. Thong ke
      render_statistics_screen();
    } else if (selected == 4) {
      system("cls");
      // 5. Thoat chuong trinh
      std::cout << "\nDA THOAT CHUONG TRINH. TAM BIET!\n";
      break;
    }
  }

  return 0;
}