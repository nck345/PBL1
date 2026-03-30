#include "../../include/ui/RentalUI.h"
#include "../../include/repository/RentalRepo.h"
#include "../../include/repository/ComicRepo.h"
#include "../../include/services/RentalService.h"
#include "../../include/utils/InputHandler.h"
#include "../../include/utils/ValidationUtils.h"
#include <iostream>
#include <string>
#include <vector>


#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>

using namespace ftxui;

// Helper to parse date strings
Date parse_date_string(const std::string &date_str) {
  Date d = {0, 0, 0};
  if (date_str.length() >= 8) {
    sscanf(date_str.c_str(), "%d/%d/%d", &d.day, &d.month, &d.year);
  }
  return d;
}

// Hàm chọn truyện từ danh sách kết quả tìm kiếm.
// - Nếu tìm thấy đúng 1 kết quả: tự động chọn luôn.
// - Nếu > 1 kết quả: hiện menu để người dùng chọn đúng quyển.
// Trả về Comic được chọn. Nếu không tìm thấy hoặc thoát, id = 0.
Comic select_comic_menu(const std::string &query) {
  std::vector<Comic> results = search_comics_by_name(query);
  if (results.empty()) return {0, "", "", "", 0, 0, false};
  if (results.size() == 1) return results[0];

  auto screen = ScreenInteractive::TerminalOutput();
  std::vector<std::string> entries;
  for (const auto &c : results)
    entries.push_back(std::string(c.comic_name) + " | Gia: " +
                      std::to_string((int)c.price) +
                      "d | Kho: " + std::to_string(c.quantity));
  entries.push_back("[Quay lai]");

  int selected = 0;
  MenuOption opt;
  opt.on_enter = screen.ExitLoopClosure();
  auto menu = Menu(&entries, &selected, opt);
  auto menu_with_event = CatchEvent(menu, [&](Event event) {
      if (event == Event::Escape) {
          selected = entries.size() - 1;
          screen.ExitLoopClosure()();
          return true;
      }
      if (event.is_character()) {
          char c = event.character()[0];
          if (c >= '1' && c <= '9') {
              int index = c - '1';
              if (index < (int)entries.size()) {
                  selected = index;
                  screen.ExitLoopClosure()();
                  return true;
              }
          }
      }
      return false;
  });

  auto renderer = Renderer(menu_with_event, [&] {
    return window(text(" Tim thay " + std::to_string(results.size()) +
                       " truyen - Chon dung quyen: "),
                  menu_with_event->Render() | vscroll_indicator | frame);
  });
  screen.Loop(renderer);

  if (selected >= 0 && selected < (int)results.size())
    return results[selected];
  return {0, "", "", "", 0, 0, false}; // Nguoi dung thoat
}

// Hàm gợi ý khách hàng dựa trên lịch sử phiếu thuê đã có.
// - Nếu không thấy khách cũ nào khớp: trả thẳng string vừa nhập.
// - Nếu thấy: hiện menu để chọn khách cũ hoặc dùng thông tin mới nhập.
// Trả về chuỗi thông tin khách. Trả về "" nếu người dùng muốn quay lại.
std::string select_customer_menu(const std::string &query) {
  std::vector<RentalSlip> slips = get_all_rental_slips();
  std::vector<std::string> matches;
  for (const auto &s : slips) {
    std::string info = s.khach_hang;
    if (info.find(query) != std::string::npos) {
      bool dup = false;
      for (const auto &m : matches) if (m == info) { dup = true; break; }
      if (!dup) matches.push_back(info);
    }
  }
  if (matches.empty()) return query;

  auto screen = ScreenInteractive::TerminalOutput();
  std::vector<std::string> entries = matches;
  entries.push_back("[Dung thong tin moi]: " + query);
  entries.push_back("[Quay lai]");

  int selected = 0;
  MenuOption opt;
  opt.on_enter = screen.ExitLoopClosure();
  auto menu = Menu(&entries, &selected, opt);
  auto menu_with_event = CatchEvent(menu, [&](Event event) {
      if (event == Event::Escape) {
          selected = entries.size() - 1; // [Quay lai]
          screen.ExitLoopClosure()();
          return true;
      }
      if (event.is_character()) {
          char c = event.character()[0];
          if (c >= '1' && c <= '9') {
              int index = c - '1';
              if (index < (int)entries.size()) {
                  selected = index;
                  screen.ExitLoopClosure()();
                  return true;
              }
          }
      }
      return false;
  });

  auto renderer = Renderer(menu_with_event, [&] {
    return window(text(" Goi y khach hang cu (chon hoac nhap moi): "),
                  menu_with_event->Render() | vscroll_indicator | frame);
  });
  screen.Loop(renderer);

  if (selected >= 0 && selected < (int)matches.size()) return matches[selected];
  if (selected == (int)matches.size()) return query;
  return ""; // Quay lai
}

void render_new_rental_screen() {
  system("cls");
  auto screen = ScreenInteractive::TerminalOutput();

  std::string ten_truyen_str;
  std::string khach_info_str;
  std::string ngay_muon_str;
  std::string ngay_tra_str;

  Component input_truyen = Input(&ten_truyen_str, "nhập tên truyện...");
  Component input_khach = Input(&khach_info_str, "nhập tên hoặc SĐT...");
  Component input_ngay_muon = Input(&ngay_muon_str, "dd/mm/yyyy");
  Component input_ngay_tra = Input(&ngay_tra_str, "dd/mm/yyyy");
  bool should_submit = false;

  auto submit_button = Button("Xác nhận & Cho thuê", [&] {
                                should_submit = true;
                                screen.ExitLoopClosure()();
                              },
                              ButtonOption::Animated());
  auto cancel_button = Button("Hủy & Trở về", [&] {
                                should_submit = false;
                                screen.ExitLoopClosure()();
                              },
                              ButtonOption::Animated());

  auto container = Container::Vertical(
      {input_truyen, input_khach, input_ngay_muon, input_ngay_tra,
       Container::Horizontal({submit_button, cancel_button})});

  auto container_with_esc = CatchEvent(container, [&](Event event) {
      if (event == Event::Escape) {
          should_submit = false;
          screen.ExitLoopClosure()();
          return true;
      }
      if (event == Event::Return) {
          if (input_truyen->Focused()) {
              input_khach->TakeFocus();
              return true;
          }
          if (input_khach->Focused()) {
              input_ngay_muon->TakeFocus();
              return true;
          }
          if (input_ngay_muon->Focused()) {
              input_ngay_tra->TakeFocus();
              return true;
          }
          if (input_ngay_tra->Focused()) {
              should_submit = true;
              screen.ExitLoopClosure()();
              return true;
          }
      }
      return false;
  });

  auto renderer = Renderer(container_with_esc, [&] {
    return vbox({text(" THIET LAP PHIEU THUE ") | bold | center, separator(),
                 hbox(text(" Tên Truyện:      "), input_truyen->Render()),
                 hbox(text(" Khách hàng:      "), input_khach->Render()),
                 hbox(text(" Ngày mượn:      "), input_ngay_muon->Render()),
                 hbox(text(" Hạn trả (dự kiến): "), input_ngay_tra->Render()),
                 separator(),
                 hbox(submit_button->Render(), text("   "),
                      cancel_button->Render()) |
                     center}) |
           border;
  });

  screen.Loop(renderer);

  if (should_submit && !ten_truyen_str.empty() && !khach_info_str.empty() &&
      !ngay_muon_str.empty()) {
    Date d_muon = parse_date_string(ngay_muon_str);
    Date d_tra = parse_date_string(ngay_tra_str);

    if (d_muon.year > 0) {
      system("cls");
      // Buoc 1: Chon chinh xac quyen truyen
      Comic chosen_comic = select_comic_menu(ten_truyen_str);
      if (chosen_comic.id == 0) {
        std::cout << "Khong tim thay truyen hoac da quay lai.\n";
        get_string_input("Nhan Enter de thu lai...");
        return;
      }
      // Buoc 2: Xac nhan thong tin khach hang
      std::string final_khach = select_customer_menu(khach_info_str);
      if (final_khach.empty()) return; // Nguoi dung quay lai

      system("cls");
      process_new_rental(chosen_comic.comic_name, final_khach.c_str(), d_muon,
                         d_tra, 0.0);
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
  bool should_submit = false;

  Component input_phieu = Input(&phieu_id_str, "nhập số...");
  Component input_ngay = Input(&ngay_tra_str, "dd/mm/yyyy");
  Component input_tt = Input(&trang_thai_str, "1: Bình thường, 2: Mất/Hỏng");

  auto submit_button = Button("Xác nhận & Thanh toán", [&] {
                                should_submit = true;
                                screen.ExitLoopClosure()();
                              },
                              ButtonOption::Animated());
  auto cancel_button = Button("Hủy & Trở về", [&] {
                                should_submit = false;
                                screen.ExitLoopClosure()();
                              },
                              ButtonOption::Animated());

  auto container = Container::Vertical(
      {input_phieu, input_ngay, input_tt,
       Container::Horizontal({submit_button, cancel_button})});

  auto container_with_esc = CatchEvent(container, [&](Event event) {
      if (event == Event::Escape) {
          should_submit = false;
          screen.ExitLoopClosure()();
          return true;
      }
      if (event == Event::Return) {
          if (input_phieu->Focused()) {
              input_ngay->TakeFocus();
              return true;
          }
          if (input_ngay->Focused()) {
              input_tt->TakeFocus();
              return true;
          }
          if (input_tt->Focused()) {
              should_submit = true;
              screen.ExitLoopClosure()();
              return true;
          }
      }
      return false;
  });

  auto renderer = Renderer(container_with_esc, [&] {
    return vbox({text(" TRA TRUYEN & THANH TOAN ") | bold | center, separator(),
                 hbox(text(" Phiếu ID:   "), input_phieu->Render()),
                 hbox(text(" Ngày trả:   "), input_ngay->Render()),
                 hbox(text(" Tình trạng: "), input_tt->Render()), separator(),
                 hbox(submit_button->Render(), text("   "),
                      cancel_button->Render()) |
                     center}) |
           border;
  });

  screen.Loop(renderer);

  if (should_submit && !phieu_id_str.empty() && !ngay_tra_str.empty() &&
      !trang_thai_str.empty()) {
    int p_id = 0, tt = 1;
    try {
      p_id = std::stoi(phieu_id_str);
    } catch (...) {
    }
    try {
      tt = std::stoi(trang_thai_str);
    } catch (...) {
    }

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
    auto menu_with_event = CatchEvent(menu, [&](Event event) {
        if (event == Event::Escape) {
            selected = entries.size() - 1;
            screen.ExitLoopClosure()();
            return true;
        }
        if (event.is_character()) {
            char c = event.character()[0];
            if (c >= '1' && c <= '9') {
                int index = c - '1';
                if (index < (int)entries.size()) {
                    selected = index;
                    screen.ExitLoopClosure()();
                    return true;
                }
            }
        }
        return false;
    });

    auto renderer = Renderer(menu_with_event, [&] {
        return window(text(" QUAN LY PHIEU THUE "),
                      menu_with_event->Render() | vscroll_indicator | frame) | bold;
    });

    while (true) {
        system("cls");
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
  table_data.push_back({"ID", "Truyen", "Khach", "Ngay Muon", "Du Kien",
                        "Thuc Te", "Tien Coc", "Tong Tien", "Trang Thai"});

  std::vector<int> warning_rows;

  int row_idx = 1;
  // Can hardcode today for project presentation or make a prompt. Let's assume
  // today is when the presentation happens (e.g 24/3/2026)
  Date today_test = {24, 3, 2026};

  for (const auto &s : slips) {
    std::string ngay_m = std::to_string(s.ngay_muon.day) + "/" +
                         std::to_string(s.ngay_muon.month) + "/" +
                         std::to_string(s.ngay_muon.year);
    std::string ngay_d = std::to_string(s.ngay_tra_du_kien.day) + "/" +
                         std::to_string(s.ngay_tra_du_kien.month) + "/" +
                         std::to_string(s.ngay_tra_du_kien.year);
    std::string ngay_t = (s.ngay_tra_thuc_te.year > 0)
                             ? (std::to_string(s.ngay_tra_thuc_te.day) + "/" +
                                std::to_string(s.ngay_tra_thuc_te.month) + "/" +
                                std::to_string(s.ngay_tra_thuc_te.year))
                             : "N/A";

    std::string tt = "Dang Thue";
    if (s.trang_thai == 1)
      tt = "Da Tra";
    else if (s.trang_thai == 2)
      tt = "Mat/Hong";
    else if (s.trang_thai == 3)
      tt = "Qua Han";

    // Cảnh báo màu đỏ
    if (s.trang_thai == 2 || s.trang_thai == 3 ||
        (s.trang_thai == 0 &&
         date_to_days(today_test) > date_to_days(s.ngay_tra_du_kien))) {
      warning_rows.push_back(row_idx);
    }

    table_data.push_back({std::to_string(s.id_phieu), std::string(s.ten_truyen),
                          std::string(s.khach_hang), ngay_m, ngay_d, ngay_t,
                          format_currency(s.tien_coc),
                          format_currency(s.tong_tien), tt});
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

  rental_statistics stats =
      compute_all_statistics(today_test, today_test.month, today_test.year);

  auto stats_panel =
      vbox({text("--- TONG QUAN (" + std::to_string(today_test.day) + "/" +
                 std::to_string(today_test.month) + ") ---") |
                bold,
            text("So sach dang thue: " + std::to_string(stats.rented_count)),
            text("So sach mat/hong:  " + std::to_string(stats.lost_count)),
            text("Doanh thu hom nay: " +
                 std::string(format_currency(stats.daily_revenue))),
            text("Doanh thu trong thang: " +
                 std::string(format_currency(stats.monthly_revenue)))}) |
      border;

  auto document = vbox(
      {text(" THONG KE PHIEU THUE & DOANH THU ") | bold | center, separator(),
       stats_panel, text(" DANH SACH PHIEU THUE ") | bold, table.Render()});

  auto screen =
      Screen::Create(Dimension::Fit(document), Dimension::Fit(document));
  Render(screen, document);
  screen.Print();

  get_string_input("\nNhan Enter de tro ve...");
}
