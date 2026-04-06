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
#include <ctime>
#include "../../include/ui/ComicUI.h"
#include "../../include/ui/CustomerUI.h"
#include "../../include/repository/CustomerRepo.h"


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

std::string get_c_name(int comic_id, const std::vector<Comic>& comics) {
  for (const auto& c : comics) {
    if (c.id == comic_id) return c.comic_name;
  }
  return "Unknown Comic";
}

std::string get_cu_name(int customer_id, const std::vector<Customer>& customers) {
  for (const auto& c : customers) {
    if (c.id == customer_id) return std::string(c.name) + " (" + std::string(c.phone) + ")";
  }
  return "Unknown Customer";
}

void render_new_rental_screen() {
  system("cls");

  std::cout << "--- BUOC 1: CHON TRUYEN ---\n";
  int comic_id = select_comic_ui("CHON TRUYEN DE CHO THUE");
  if (comic_id <= 0) {
    std::cout << "Huy thao tac mượn truyện. Nhấn Enter de thoat...\n";
    get_string_input("");
    return;
  }

  system("cls");
  std::cout << "--- BUOC 2: CHON KHACH HANG ---\n";
  int customer_id = select_customer_ui("CHON KHACH HANG MƯỢN TRUYỆN");
  if (customer_id <= 0) {
    std::cout << "Huy thao tac mượn truyện. Nhấn Enter de thoat...\n";
    get_string_input("");
    return;
  }

  system("cls");
  auto screen = ScreenInteractive::TerminalOutput();

  std::string ngay_tra_d, ngay_tra_m, ngay_tra_y;
  std::string error_msg = "";

  Component input_nt_d = Input(&ngay_tra_d, "DD");
  Component input_nt_m = Input(&ngay_tra_m, "MM");
  Component input_nt_y = Input(&ngay_tra_y, "YYYY");
  bool should_submit = false;

  auto submit_button = Button("Xác nhận & Cho thuê", [&] {
      if (ngay_tra_d.empty() || ngay_tra_m.empty() || ngay_tra_y.empty()) {
          error_msg = "Lỗi: Vui lòng nhập ngày dự kiến trả!";
          return;
      }
      try {
          int td = std::stoi(ngay_tra_d); int tm = std::stoi(ngay_tra_m); int ty = std::stoi(ngay_tra_y);
          if (!is_valid_date(td, tm, ty)) throw std::invalid_argument("invalid");
      } catch (...) {
          error_msg = "Lỗi: Ngày dự kiến trả không hợp lệ!";
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
          if (input_nt_d->Focused()) { input_nt_m->TakeFocus(); return true; }
          if (input_nt_m->Focused()) { input_nt_y->TakeFocus(); return true; }
          if (input_nt_y->Focused()) {
              submit_button->TakeFocus();
              return true;
          }
      }
      return false;
  });

  auto renderer = Renderer(container_with_esc, [&] {
    return vbox({text(" THIẾT LẬP NGÀY TRẢ ") | bold | center, separator(),
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
    int td = std::stoi(ngay_tra_d); int tm = std::stoi(ngay_tra_m); int ty = std::stoi(ngay_tra_y);
    Date d_tra = {td, tm, ty};

    system("cls");
    process_new_rental(comic_id, customer_id, d_tra);
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
  
  std::vector<Comic> all_c = read_all_comics();
  std::vector<Customer> all_cu = read_all_customers();

  for (const auto& s : slips) {
    if (s.trang_thai == 0) { // Dang thue
      std::string cu_name = get_cu_name(s.customer_id, all_cu);
      std::string kh_lower = cu_name;
      for (auto& c : kh_lower) c = std::tolower(c);
      
      std::string c_name = get_c_name(s.comic_id, all_c);
      std::string ten_lower = c_name;
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
    std::string cu_name = get_cu_name(s.customer_id, all_cu);
    std::string c_name = get_c_name(s.comic_id, all_c);
    std::string item = std::to_string(s.id_phieu) + ". " + cu_name + " muon [" + c_name + "] ngay " +
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

Element build_rental_table_element(const std::vector<RentalSlip>& slips, const std::vector<Comic>& all_c, const std::vector<Customer>& all_cu) {
  if (slips.empty()) {
    return text("Khong co du lieu phiếu thuê.") | center;
  }
  std::vector<std::vector<std::string>> table_data;
  table_data.push_back({"ID", "Khach Hang", "Ten Truyen", "Ngay Muon", "Trang Thai"});
  for (const auto &s : slips) {
    std::string nm = std::to_string(s.ngay_muon.day) + "/" + std::to_string(s.ngay_muon.month) + "/" + std::to_string(s.ngay_muon.year);
    std::string tt = (s.trang_thai == 0) ? "Dang thue" : (s.trang_thai == 1) ? "Da tra" : (s.trang_thai == 2) ? "Mat/Hong" : "Qua han";
    std::string cu_name = get_cu_name(s.customer_id, all_cu);
    std::string c_name = get_c_name(s.comic_id, all_c);
    table_data.push_back({std::to_string(s.id_phieu), cu_name, c_name, nm, tt});
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
            
            std::vector<Comic> all_c = read_all_comics();
            std::vector<Customer> all_cu = read_all_customers();

            auto apply_filter = [&] {
                filtered_slips.clear();
                std::string kw = search_kw;
                for(auto&c : kw) c = std::tolower(c);
                for (const auto& s : all_slips) {
                    std::string tn = get_c_name(s.comic_id, all_c); for(auto&c : tn) c = std::tolower(c);
                    std::string tk = get_cu_name(s.customer_id, all_cu); for(auto&c : tk) c = std::tolower(c);
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
                    build_rental_table_element(filtered_slips, all_c, all_cu) | vscroll_indicator | frame
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
            std::to_string(s.id_phieu), c_name, cu_name, ngay_m, ngay_d, ngay_t,
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
            std::to_string(s.id_phieu), c_name, cu_name,
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

