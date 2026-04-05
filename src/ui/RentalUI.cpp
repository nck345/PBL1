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
  std::string ngay_muon_d, ngay_muon_m, ngay_muon_y;
  std::string ngay_tra_d, ngay_tra_m, ngay_tra_y;
  std::string error_msg = "";

  Component input_truyen = Input(&ten_truyen_str, "nhập tên truyện...");
  Component input_khach = Input(&khach_info_str, "nhập tên hoặc SĐT...");
  Component input_nm_d = Input(&ngay_muon_d, "DD");
  Component input_nm_m = Input(&ngay_muon_m, "MM");
  Component input_nm_y = Input(&ngay_muon_y, "YYYY");
  Component input_nt_d = Input(&ngay_tra_d, "DD");
  Component input_nt_m = Input(&ngay_tra_m, "MM");
  Component input_nt_y = Input(&ngay_tra_y, "YYYY");
  bool should_submit = false;

  auto submit_button = Button("Xác nhận & Cho thuê", [&] {
      if (ten_truyen_str.empty() || khach_info_str.empty() || ngay_muon_d.empty() || ngay_muon_m.empty() || ngay_muon_y.empty() || ngay_tra_d.empty() || ngay_tra_m.empty() || ngay_tra_y.empty()) {
          error_msg = "Lỗi: Không được để trống thông tin!";
          return;
      }
      try {
          int md = std::stoi(ngay_muon_d); int mm = std::stoi(ngay_muon_m); int my = std::stoi(ngay_muon_y);
          int td = std::stoi(ngay_tra_d); int tm = std::stoi(ngay_tra_m); int ty = std::stoi(ngay_tra_y);
          if (!is_valid_date(md, mm, my) || !is_valid_date(td, tm, ty)) throw std::invalid_argument("invalid");
          Date d_muon = {md, mm, my};
          Date d_tra = {td, tm, ty};
          if (date_to_days(d_tra) < date_to_days(d_muon)) {
              error_msg = "Lỗi: Ngày trả dự kiến < ngày mượn!";
              return;
          }
      } catch (...) {
          error_msg = "Lỗi: Ngày tháng năm mượn/trả không hợp lệ!";
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

  auto container = Container::Vertical({
       input_truyen, input_khach,
       input_nm_d, input_nm_m, input_nm_y,
       input_nt_d, input_nt_m, input_nt_y,
       Container::Horizontal({submit_button, cancel_button})
  });

  auto container_with_esc = CatchEvent(container, [&](Event event) {
      if (event == Event::Escape) {
          should_submit = false;
          screen.ExitLoopClosure()();
          return true;
      }
      if (event == Event::Return) {
          if (input_truyen->Focused()) { input_khach->TakeFocus(); return true; }
          if (input_khach->Focused()) { input_nm_d->TakeFocus(); return true; }
          if (input_nm_d->Focused()) { input_nm_m->TakeFocus(); return true; }
          if (input_nm_m->Focused()) { input_nm_y->TakeFocus(); return true; }
          if (input_nm_y->Focused()) { input_nt_d->TakeFocus(); return true; }
          if (input_nt_d->Focused()) { input_nt_m->TakeFocus(); return true; }
          if (input_nt_m->Focused()) { input_nt_y->TakeFocus(); return true; }
          if (input_nt_y->Focused()) {
              if (ten_truyen_str.empty() || khach_info_str.empty() || ngay_muon_d.empty() || ngay_muon_m.empty() || ngay_muon_y.empty() || ngay_tra_d.empty() || ngay_tra_m.empty() || ngay_tra_y.empty()) {
                  error_msg = "Lỗi: Không được để trống thông tin!";
                  return true;
              }
              try {
                  int md = std::stoi(ngay_muon_d); int mm = std::stoi(ngay_muon_m); int my = std::stoi(ngay_muon_y);
                  int td = std::stoi(ngay_tra_d); int tm = std::stoi(ngay_tra_m); int ty = std::stoi(ngay_tra_y);
                  if (!is_valid_date(md, mm, my) || !is_valid_date(td, tm, ty)) throw std::invalid_argument("invalid");
                  Date d_muon = {md, mm, my};
                  Date d_tra = {td, tm, ty};
                  if (date_to_days(d_tra) < date_to_days(d_muon)) {
                      error_msg = "Lỗi: Ngày trả dự kiến < ngày mượn!";
                      return true;
                  }
              } catch (...) {
                  error_msg = "Lỗi: Ngày tháng năm mượn/trả không hợp lệ!";
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
                 hbox(text(" Ngày mượn:      "), input_nm_d->Render(), text("/"), input_nm_m->Render(), text("/"), input_nm_y->Render()),
                 hbox(text(" Hạn trả (dự kiến): "), input_nt_d->Render(), text("/"), input_nt_m->Render(), text("/"), input_nt_y->Render()),
                 separator(),
                 error_msg.empty() ? text("") : text(error_msg) | color(Color::Red) | center,
                 hbox(submit_button->Render(), text("   "),
                      cancel_button->Render()) |
                     center}) |
           border;
  });

  screen.Loop(renderer);

  if (should_submit) {
    int md = std::stoi(ngay_muon_d); int mm = std::stoi(ngay_muon_m); int my = std::stoi(ngay_muon_y);
    int td = std::stoi(ngay_tra_d); int tm = std::stoi(ngay_tra_m); int ty = std::stoi(ngay_tra_y);
    Date d_muon = {md, mm, my};
    Date d_tra = {td, tm, ty};

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

int select_rental_slip_ui(const std::string& title) {
  std::string keyword = get_string_input("Nhap Ten Khach/SDT hoac Bo Trong de Xem tat ca Dang Thue: ");
  if (keyword == "[ESC]") return -1;
  std::string kw_lower = keyword;
  for (auto& c : kw_lower) c = std::tolower(c);

  std::vector<RentalSlip> slips = get_all_rental_slips();
  std::vector<RentalSlip> active_slips;
  for (const auto& s : slips) {
    if (s.trang_thai == 0) { // Dang thue
      std::string kh_lower = s.khach_hang;
      for (auto& c : kh_lower) c = std::tolower(c);
      std::string ten_lower = s.ten_truyen;
      for (auto& c : ten_lower) c = std::tolower(c);
      if (kw_lower.empty() || kh_lower.find(kw_lower) != std::string::npos || ten_lower.find(kw_lower) != std::string::npos) {
        active_slips.push_back(s);
      }
    }
  }

  if (active_slips.empty()) {
    std::cout << "Khong tim thay phieu dang thue nao phu hop!\n";
    get_string_input("Nhan Enter de thu lai...");
    return -1;
  }

  auto screen = ScreenInteractive::TerminalOutput();
  std::vector<std::string> entries;
  for (size_t i = 0; i < active_slips.size(); ++i) {
    const auto& s = active_slips[i];
    std::string item = std::to_string(s.id_phieu) + ". " + s.khach_hang + " muon [" + s.ten_truyen + "] ngay " +
                       std::to_string(s.ngay_muon.day) + "/" + std::to_string(s.ngay_muon.month) + "/" + std::to_string(s.ngay_muon.year);
    entries.push_back(item);
  }
  entries.push_back("[Huy thao tac]");

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
    return window(text(" " + title + " "),
                  menu_with_event->Render() | vscroll_indicator | frame) | bold;
  });

  screen.Loop(renderer);
  system("cls");

  if (selected >= 0 && selected < (int)active_slips.size()) {
    return active_slips[selected].id_phieu;
  }
  return -1;
}

void render_return_comic_screen() {
  system("cls");
  int p_id = select_rental_slip_ui("CHON PHIEU THUE DE TRA");
  if (p_id == -1) return;

  system("cls");
  auto screen = ScreenInteractive::TerminalOutput();

  std::string p_id_str = std::to_string(p_id);
  std::string ngay_tra_d, ngay_tra_m, ngay_tra_y;
  std::string trang_thai_str; // 1: tra binh thuong, 2: mat hong
  std::string error_msg = "";
  bool should_submit = false;

  Component input_nt_d = Input(&ngay_tra_d, "DD");
  Component input_nt_m = Input(&ngay_tra_m, "MM");
  Component input_nt_y = Input(&ngay_tra_y, "YYYY");
  Component input_tt = Input(&trang_thai_str, "1: Bình thường, 2: Mất/Hỏng");

  auto submit_button = Button("Xác nhận & Thanh toán", [&] {
      if (ngay_tra_d.empty() || ngay_tra_m.empty() || ngay_tra_y.empty() || trang_thai_str.empty()) {
          error_msg = "Lỗi: Không được để trống thông tin!";
          return;
      }

      try {
          int tt = std::stoi(trang_thai_str);
          if (tt != 1 && tt != 2) throw std::invalid_argument("not 1 or 2");
      } catch (...) {
          error_msg = "Lỗi: Trạng thái chỉ được nhập 1 hoặc 2!";
          return;
      }
      try {
          int td = std::stoi(ngay_tra_d); int tm = std::stoi(ngay_tra_m); int ty = std::stoi(ngay_tra_y);
          if (!is_valid_date(td, tm, ty)) throw std::invalid_argument("invalid");
      } catch (...) {
          error_msg = "Lỗi: Định dạng ngày trả không hợp lệ!";
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
      {input_nt_d, input_nt_m, input_nt_y, input_tt,
       Container::Horizontal({submit_button, cancel_button})});

  auto container_with_esc = CatchEvent(container, [&](Event event) {
      if (event == Event::Escape) {
          should_submit = false;
          screen.ExitLoopClosure()();
          return true;
      }
      if (event == Event::Return) {
          if (input_nt_d->Focused()) { input_nt_m->TakeFocus(); return true; }
          if (input_nt_m->Focused()) { input_nt_y->TakeFocus(); return true; }
          if (input_nt_y->Focused()) { input_tt->TakeFocus(); return true; }
          if (input_tt->Focused()) {
              if (ngay_tra_d.empty() || ngay_tra_m.empty() || ngay_tra_y.empty() || trang_thai_str.empty()) {
                  error_msg = "Lỗi: Không được để trống thông tin!";
                  return true;
              }
              try {
                  int tt = std::stoi(trang_thai_str);
                  if (tt != 1 && tt != 2) throw std::invalid_argument("not 1 or 2");
              } catch (...) {
                  error_msg = "Lỗi: Trạng thái chỉ được nhập 1 hoặc 2!";
                  return true;
              }
              try {
                  int td = std::stoi(ngay_tra_d); int tm = std::stoi(ngay_tra_m); int ty = std::stoi(ngay_tra_y);
                  if (!is_valid_date(td, tm, ty)) throw std::invalid_argument("invalid");
              } catch (...) {
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
                 hbox(text(" Phiếu ID:   "), text(p_id_str) | bold),
                 hbox(text(" Ngày trả:   "), input_nt_d->Render(), text("/"), input_nt_m->Render(), text("/"), input_nt_y->Render()),
                 hbox(text(" Tình trạng: "), input_tt->Render()), separator(),
                 error_msg.empty() ? text("") : text(error_msg) | color(Color::Red) | center,
                 hbox(submit_button->Render(), text("   "),
                      cancel_button->Render()) |
                     center}) |
           border;
  });

  screen.Loop(renderer);

  if (should_submit) {
    int tt = std::stoi(trang_thai_str);
    int td = std::stoi(ngay_tra_d); int tm = std::stoi(ngay_tra_m); int ty = std::stoi(ngay_tra_y);
    Date d_tra = {td, tm, ty};

    system("cls");
    process_return_comic(p_id, d_tra, tt, 0.0);
    get_string_input("Nhan Enter de tiep tuc...");
  }
}

Element build_rental_table_element(const std::vector<RentalSlip>& slips) {
  if (slips.empty()) {
    return text("Khong co du lieu phiếu thuê.") | center;
  }
  std::vector<std::vector<std::string>> table_data;
  table_data.push_back({"ID", "Khach Hang", "Ten Truyen", "Ngay Muon", "Trang Thai"});
  for (const auto &s : slips) {
    std::string nm = std::to_string(s.ngay_muon.day) + "/" + std::to_string(s.ngay_muon.month) + "/" + std::to_string(s.ngay_muon.year);
    std::string tt = (s.trang_thai == 0) ? "Dang thue" : (s.trang_thai == 1) ? "Da tra" : (s.trang_thai == 2) ? "Mat/Hong" : "Qua han";
    table_data.push_back({std::to_string(s.id_phieu), s.khach_hang, s.ten_truyen, nm, tt});
  }
  auto table = Table(table_data);
  table.SelectAll().Border(LIGHT);
  table.SelectRow(0).Decorate(bold);
  table.SelectRow(0).SeparatorVertical(LIGHT);
  table.SelectRow(0).Border(DOUBLE);
  return table.Render();
}

void render_rental_menu() {
    auto screen = ScreenInteractive::TerminalOutput();
    std::vector<std::string> entries = {
        "1. Xem danh sach phieu thue",
        "2. Cho thue truyen moi",
        "3. Tra truyen & Thanh toan",
        "4. Tro ve"
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
            system("cls");
            std::vector<RentalSlip> all_slips = get_all_rental_slips();
            if (all_slips.empty()) {
                std::cout << "Khong co phieu thue nao trong he thong!\n";
                get_string_input("Nhan Enter de tiep tuc...");
                continue;
            }

            auto form_screen = ScreenInteractive::Fullscreen();
            std::string search_kw = "";
            std::vector<std::string> status_options = {"Tat ca", "Dang thue", "Da tra/Khac"};
            int selected_status = 0;
            std::vector<RentalSlip> filtered_slips = all_slips;

            auto apply_filter = [&] {
                filtered_slips.clear();
                std::string kw = search_kw;
                for(auto&c : kw) c = std::tolower(c);
                for (const auto& s : all_slips) {
                    std::string tn = s.ten_truyen; for(auto&c : tn) c = std::tolower(c);
                    std::string tk = s.khach_hang; for(auto&c : tk) c = std::tolower(c);
                    bool match_kw = kw.empty() || tn.find(kw) != std::string::npos || tk.find(kw) != std::string::npos || std::to_string(s.id_phieu) == kw;
                    bool match_status = true;
                    if (selected_status == 1) match_status = (s.trang_thai == 0);
                    if (selected_status == 2) match_status = (s.trang_thai != 0);

                    if (match_kw && match_status) {
                        filtered_slips.push_back(s);
                    }
                }
            };

            Component input_search = Input(&search_kw, "ID, SDT, hoac ten truyen...");
            Component status_radiobox = Radiobox(&status_options, &selected_status);
            Component exit_button = Button("Tro Ve", [&] { form_screen.ExitLoopClosure()(); }, ButtonOption::Animated());

            auto controls = Container::Vertical({
                input_search, status_radiobox, exit_button
            });

            auto controls_with_event = CatchEvent(controls, [&](Event event) {
                if (event == Event::Escape) {
                    form_screen.ExitLoopClosure()();
                    return true;
                }
                return false;
            });

            auto ui_renderer = Renderer(controls_with_event, [&] {
                apply_filter();
                auto filter_panel = vbox({
                    text("--- BỘ LỌC === ") | bold,
                    separator(),
                    hbox(text(" Tim kiem: "), input_search->Render()),
                    separator(),
                    text(" Trang thai: ") | bold,
                    status_radiobox->Render(),
                    separator(),
                    exit_button->Render() | center
                }) | border | size(WIDTH, GREATER_THAN, 30);

                auto table_panel = window(
                    text(" DANH SACH PHIEU (" + std::to_string(filtered_slips.size()) + ")") | bold | center,
                    build_rental_table_element(filtered_slips) | vscroll_indicator | frame
                ) | flex;

                return hbox({filter_panel, table_panel});
            });

            form_screen.Loop(ui_renderer);
        } else if (selected == 1) {
            render_new_rental_screen();
        } else if (selected == 2) {
            render_return_comic_screen();
        } else if (selected == 3) {
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

