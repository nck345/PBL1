#include "../../include/ui/RentalUI.h"
#include "../../include/repository/RentalRepo.h"
#include "../../include/repository/ComicRepo.h"
#include "../../include/services/RentalService.h"
#include "../../include/utils/InputHandler.h"
#include "../../include/utils/ValidationUtils.h"
#include "../../include/utils/SortUtils.h"
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
  std::string error_msg = "";

  Component input_truyen = Input(&ten_truyen_str, "nhập tên truyện...");
  Component input_khach = Input(&khach_info_str, "nhập tên hoặc SĐT...");
  Component input_ngay_muon = Input(&ngay_muon_str, "dd/mm/yyyy");
  Component input_ngay_tra = Input(&ngay_tra_str, "dd/mm/yyyy");
  bool should_submit = false;

  auto submit_button = Button("Xác nhận & Cho thuê", [&] {
      if (ten_truyen_str.empty() || khach_info_str.empty() || ngay_muon_str.empty() || ngay_tra_str.empty()) {
          error_msg = "Lỗi: Không được để trống thông tin!";
          return;
      }
      Date d_muon = parse_date_string(ngay_muon_str);
      Date d_tra = parse_date_string(ngay_tra_str);
      if (d_muon.year == 0 || d_tra.year == 0) {
          error_msg = "Lỗi: Định dạng ngày không hợp lệ (nhập dd/mm/yyyy)!";
          return;
      }
      if (date_to_days(d_tra) < date_to_days(d_muon)) {
          error_msg = "Lỗi: Ngày trả dự kiến < ngày mượn!";
          return;
      }
      error_msg = "";
      should_submit = true;
      screen.ExitLoopClosure()();
  }, ButtonOption::Animated());

  auto cancel_button = Button("Hủy & Trở về", [&] {
      should_submit = false;
      screen.ExitLoopClosure()();
  }, ButtonOption::Animated());

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
              if (ten_truyen_str.empty() || khach_info_str.empty() || ngay_muon_str.empty() || ngay_tra_str.empty()) {
                  error_msg = "Lỗi: Không được để trống thông tin!";
                  return true;
              }
              Date d_muon = parse_date_string(ngay_muon_str);
              Date d_tra = parse_date_string(ngay_tra_str);
              if (d_muon.year == 0 || d_tra.year == 0) {
                  error_msg = "Lỗi: Định dạng ngày không hợp lệ (nhập dd/mm/yyyy)!";
                  return true;
              }
              if (date_to_days(d_tra) < date_to_days(d_muon)) {
                  error_msg = "Lỗi: Ngày trả dự kiến < ngày mượn!";
                  return true;
              }
              should_submit = true;
              screen.ExitLoopClosure()();
              return true;
          }
      }
      return false;
  });

  auto renderer = Renderer(container_with_esc, [&] {
    return vbox({text(" THIẾT LẬP PHIẾU THUÊ ") | bold | center, separator(),
                 hbox(text(" Tên Truyện:      "), input_truyen->Render()),
                 hbox(text(" Khách hàng:      "), input_khach->Render()),
                 hbox(text(" Ngày mượn:      "), input_ngay_muon->Render()),
                 hbox(text(" Hạn trả (dự kiến): "), input_ngay_tra->Render()),
                 separator(),
                 error_msg.empty() ? text("") : text(error_msg) | color(Color::Red) | center,
                 hbox(submit_button->Render(), text("   "),
                      cancel_button->Render()) |
                     center}) |
           border;
  });

  screen.Loop(renderer);

  if (should_submit) {
    Date d_muon = parse_date_string(ngay_muon_str);
    Date d_tra = parse_date_string(ngay_tra_str);

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
    process_new_rental(chosen_comic.comic_name, final_khach.c_str(), d_muon, d_tra, 0.0);
    get_string_input("Nhan Enter de tiep tuc...");
  }
}

void render_return_comic_screen() {
  system("cls");
  auto screen = ScreenInteractive::TerminalOutput();

  std::string phieu_id_str;
  std::string ngay_tra_str;
  std::string trang_thai_str; // 1: tra binh thuong, 2: mat hong
  std::string error_msg = "";
  bool should_submit = false;

  Component input_phieu = Input(&phieu_id_str, "nhập số ID...");
  Component input_ngay = Input(&ngay_tra_str, "dd/mm/yyyy");
  Component input_tt = Input(&trang_thai_str, "1: Bình thường, 2: Mất/Hỏng");

  auto submit_button = Button("Xác nhận & Thanh toán", [&] {
      if (phieu_id_str.empty() || ngay_tra_str.empty() || trang_thai_str.empty()) {
          error_msg = "Lỗi: Không được để trống thông tin!";
          return;
      }
      try {
          int id = std::stoi(phieu_id_str);
          if (id <= 0) throw std::invalid_argument("<=0");
      } catch (...) {
          error_msg = "Lỗi: ID Phiếu phải là một số nguyên dương!";
          return;
      }
      try {
          int tt = std::stoi(trang_thai_str);
          if (tt != 1 && tt != 2) throw std::invalid_argument("not 1 or 2");
      } catch (...) {
          error_msg = "Lỗi: Trạng thái chỉ được nhập 1 hoặc 2!";
          return;
      }
      Date d_tra = parse_date_string(ngay_tra_str);
      if (d_tra.year == 0) {
          error_msg = "Lỗi: Định dạng ngày trả không hợp lệ (nhập dd/mm/yyyy)!";
          return;
      }
      error_msg = "";
      should_submit = true;
      screen.ExitLoopClosure()();
  }, ButtonOption::Animated());

  auto cancel_button = Button("Hủy & Trở về", [&] {
      should_submit = false;
      screen.ExitLoopClosure()();
  }, ButtonOption::Animated());

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
              if (phieu_id_str.empty() || ngay_tra_str.empty() || trang_thai_str.empty()) {
                  error_msg = "Lỗi: Không được để trống thông tin!";
                  return true;
              }
              try {
                  int id = std::stoi(phieu_id_str);
                  if (id <= 0) throw std::invalid_argument("<=0");
              } catch (...) {
                  error_msg = "Lỗi: ID Phiếu phải là một số nguyên dương!";
                  return true;
              }
              try {
                  int tt = std::stoi(trang_thai_str);
                  if (tt != 1 && tt != 2) throw std::invalid_argument("not 1 or 2");
              } catch (...) {
                  error_msg = "Lỗi: Trạng thái chỉ được nhập 1 hoặc 2!";
                  return true;
              }
              Date d_tra = parse_date_string(ngay_tra_str);
              if (d_tra.year == 0) {
                  error_msg = "Lỗi: Định dạng ngày trả không hợp lệ!";
                  return true;
              }
              should_submit = true;
              screen.ExitLoopClosure()();
              return true;
          }
      }
      return false;
  });

  auto renderer = Renderer(container_with_esc, [&] {
    return vbox({text(" TRẢ TRUYỆN & THANH TOÁN ") | bold | center, separator(),
                 hbox(text(" Phiếu ID:   "), input_phieu->Render()),
                 hbox(text(" Ngày trả:   "), input_ngay->Render()),
                 hbox(text(" Tình trạng: "), input_tt->Render()), separator(),
                 error_msg.empty() ? text("") : text(error_msg) | color(Color::Red) | center,
                 hbox(submit_button->Render(), text("   "),
                      cancel_button->Render()) |
                     center}) |
           border;
  });

  screen.Loop(renderer);

  if (should_submit) {
    int p_id = std::stoi(phieu_id_str);
    int tt = std::stoi(trang_thai_str);
    Date d_tra = parse_date_string(ngay_tra_str);

    system("cls");
    process_return_comic(p_id, d_tra, tt, 0.0);
    get_string_input("Nhan Enter de tiep tuc...");
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

    std::vector<std::vector<std::string>> main_table_data;
    main_table_data.push_back({"  ID  ", "       Tên Truyện       ", "      Khách Hàng      ", " Ngày Mượn ", "  Hạn Trả  ", "  Thực Tế  ", "  Tiền Cọc  ", "  Tổng Tiền  ", " Trạng Thái "});
    for (const auto &s : slips) {
        std::string ngay_m = std::to_string(s.ngay_muon.day) + "/" + std::to_string(s.ngay_muon.month);
        std::string ngay_d = std::to_string(s.ngay_tra_du_kien.day) + "/" + std::to_string(s.ngay_tra_du_kien.month);
        std::string ngay_t = (s.ngay_tra_thuc_te.year > 1900) ? (std::to_string(s.ngay_tra_thuc_te.day) + "/" + std::to_string(s.ngay_tra_thuc_te.month)) : "---";
        std::string tt = (s.trang_thai == 1) ? "Đã Trả" : (s.trang_thai == 2) ? "Mất/Hỏng" : "Đang Thuê";

        main_table_data.push_back({
            std::to_string(s.id_phieu), s.ten_truyen, s.khach_hang, ngay_m, ngay_d, ngay_t,
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
        
        overdue_table_data.push_back({
            std::to_string(s.id_phieu), s.ten_truyen, s.khach_hang,
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

