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
#include <ftxui/screen/terminal.hpp>

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

std::string get_c_author(int comic_id, const std::vector<Comic>& comics) {
  for (const auto& c : comics) {
    if (c.id == comic_id) return c.author;
  }
  return "Unknown Author";
}

std::string get_c_type(int comic_id, const std::vector<Comic>& comics) {
  for (const auto& c : comics) {
    if (c.id == comic_id) return c.type;
  }
  return "Unknown Type";
}

std::string get_cu_name(int customer_id, const std::vector<Customer>& customers) {
  for (const auto& c : customers) {
    if (c.id == customer_id) return std::string(c.name) + " (" + std::string(c.phone) + ")";
  }
  return "Unknown Customer";
}


bool cmp_rui_id_asc(const RentalUIRow& a, const RentalUIRow& b) { return a.slip.id_phieu < b.slip.id_phieu; }
bool cmp_rui_id_desc(const RentalUIRow& a, const RentalUIRow& b) { return a.slip.id_phieu > b.slip.id_phieu; }

bool cmp_rui_cu_asc(const RentalUIRow& a, const RentalUIRow& b) { return a.cu_name < b.cu_name; }
bool cmp_rui_cu_desc(const RentalUIRow& a, const RentalUIRow& b) { return a.cu_name > b.cu_name; }

bool cmp_rui_c_asc(const RentalUIRow& a, const RentalUIRow& b) { return a.c_name < b.c_name; }
bool cmp_rui_c_desc(const RentalUIRow& a, const RentalUIRow& b) { return a.c_name > b.c_name; }

bool cmp_rui_date_asc(const RentalUIRow& a, const RentalUIRow& b) {
   if (a.slip.ngay_muon.year != b.slip.ngay_muon.year) return a.slip.ngay_muon.year < b.slip.ngay_muon.year;
   if (a.slip.ngay_muon.month != b.slip.ngay_muon.month) return a.slip.ngay_muon.month < b.slip.ngay_muon.month;
   return a.slip.ngay_muon.day < b.slip.ngay_muon.day;
}
bool cmp_rui_date_desc(const RentalUIRow& a, const RentalUIRow& b) { return cmp_rui_date_asc(b, a); }

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
  system("cls");
  std::vector<RentalSlip> all_slips = get_all_rental_slips();
  std::vector<Comic> all_c = read_all_comics();
  std::vector<Customer> all_cu = read_all_customers();

  std::vector<RentalUIRow> active_rows;
  for (const auto& s : all_slips) {
    if (s.trang_thai == 0) {
      RentalUIRow r;
      r.slip = s;
      r.cu_name = get_cu_name(s.customer_id, all_cu);
      r.c_name = get_c_name(s.comic_id, all_c);
      r.c_author = get_c_author(s.comic_id, all_c);
      r.c_type = get_c_type(s.comic_id, all_c);
      active_rows.push_back(r);
    }
  }
  
  if (active_rows.empty()) {
     std::cout << "Khong co phieu dang thue nao thuoc he thong!\n";
     get_string_input("Nhan Enter de thu lai...");
     return -1;
  }

  auto form_screen = ScreenInteractive::Fullscreen();
  std::string search_comic = "";
  std::string search_author = "";
  std::string search_cu = "";
  std::string search_type = "";
  
  std::vector<std::string> sort_options = {"-", "Tang dan", "Giam dan"};
  std::vector<std::string> sort_options_az = {"-", "A -> Z", "Z -> A"};
  int sort_id = 0, sort_c = 0, sort_cu = 0, sort_date = 0;
  int last_clicked_sort = 0;

  std::vector<std::string> type_options = build_type_options(all_c);
  std::vector<std::string> filtered_type_options;
  int selected_type_option = 0;

  std::vector<RentalUIRow> filtered_rows = active_rows;

  auto apply_filter = [&] {
     filtered_rows.clear();
     std::string s_comic = search_comic; for (char &c : s_comic) c = std::tolower(c);
     std::string s_author = search_author; for (char &c : s_author) c = std::tolower(c);
     std::string cu_kw = search_cu; for (char &c : cu_kw) c = std::tolower(c);
     std::string ty_kw = search_type; for (char &c : ty_kw) c = std::tolower(c);
     
     for (const auto& r : active_rows) {
        std::string tn = r.c_name; for(auto&c : tn) c = std::tolower(c);
        std::string ta = r.c_author; for(auto&c : ta) c = std::tolower(c);
        std::string tk = r.cu_name; for(auto&c : tk) c = std::tolower(c);
        std::string tl = r.c_type; for(auto&c : tl) c = std::tolower(c);
        
        bool m_comic = s_comic.empty() || tn.find(s_comic) != std::string::npos || std::to_string(r.slip.id_phieu) == s_comic;
        bool m_author = s_author.empty() || ta.find(s_author) != std::string::npos;
        bool m_cu = cu_kw.empty() || tk.find(cu_kw) != std::string::npos;
        bool m_ty = ty_kw.empty() || tl.find(ty_kw) != std::string::npos;
        
        if (m_comic && m_author && m_cu && m_ty) filtered_rows.push_back(r);
     }
     
     if (last_clicked_sort == 1) {
         if (sort_id == 1) quick_sort(filtered_rows, cmp_rui_id_asc);
         else if (sort_id == 2) quick_sort(filtered_rows, cmp_rui_id_desc);
     } else if (last_clicked_sort == 2) {
         if (sort_c == 1) quick_sort(filtered_rows, cmp_rui_c_asc);
         else if (sort_c == 2) quick_sort(filtered_rows, cmp_rui_c_desc);
     } else if (last_clicked_sort == 3) {
         if (sort_cu == 1) quick_sort(filtered_rows, cmp_rui_cu_asc);
         else if (sort_cu == 2) quick_sort(filtered_rows, cmp_rui_cu_desc);
     } else if (last_clicked_sort == 4) {
         if (sort_date == 1) quick_sort(filtered_rows, cmp_rui_date_asc);
         else if (sort_date == 2) quick_sort(filtered_rows, cmp_rui_date_desc);
     }
  };

  Component input_search_comic = Input(&search_comic, "Tim ID hoac Ten Truyen...");
  Component input_search_author = Input(&search_author, "Tim theo Tac Gia...");
  Component input_search_cu = Input(&search_cu, "Tim Khach hang/SDT...");
  Component input_search_ty = Input(&search_type, "Go de tim The Loai...");
  
  auto type_menu_raw = Menu(&filtered_type_options, &selected_type_option);
  auto type_menu_c = CatchEvent(type_menu_raw, [&](Event event) {
    if (event.is_mouse() && event.mouse().motion == Mouse::Moved) return true;
    if (event == Event::Return) {
       if (!filtered_type_options.empty() && is_valid_type_suggestion(filtered_type_options[selected_type_option])) {
           search_type = filtered_type_options[selected_type_option];
       }
       return true; 
    }
    return false;
  });

  auto t_id = Toggle(&sort_options, &sort_id);
  auto t_c = Toggle(&sort_options_az, &sort_c);
  auto t_cu = Toggle(&sort_options_az, &sort_cu);
  auto t_d = Toggle(&sort_options, &sort_date);
  
  t_id |= CatchEvent([&](Event e) { if(e==Event::Return || e.is_mouse()) { last_clicked_sort=1; sort_c=0; sort_cu=0; sort_date=0; } return false; });
  t_c |= CatchEvent([&](Event e) { if(e==Event::Return || e.is_mouse()) { last_clicked_sort=2; sort_id=0; sort_cu=0; sort_date=0; } return false; });
  t_cu |= CatchEvent([&](Event e) { if(e==Event::Return || e.is_mouse()) { last_clicked_sort=3; sort_id=0; sort_c=0; sort_date=0; } return false; });
  t_d |= CatchEvent([&](Event e) { if(e==Event::Return || e.is_mouse()) { last_clicked_sort=4; sort_id=0; sort_c=0; sort_cu=0; } return false; });

  int selected_slip_index = 0;
  int final_chosen_id = -1;
  std::vector<std::string> dummy_entries;
  
  MenuOption slip_menu_opt;
  slip_menu_opt.on_enter = [&] {
     if (!filtered_rows.empty() && selected_slip_index >= 0 && selected_slip_index < (int)filtered_rows.size()) {
        final_chosen_id = filtered_rows[selected_slip_index].slip.id_phieu;
        form_screen.ExitLoopClosure()();
     }
  };
  
  slip_menu_opt.entries_option.transform = [&](const EntryState& state) {
      if (state.index >= (int)filtered_rows.size()) return text("");
      auto& r = filtered_rows[state.index];
      std::string nm = std::to_string(r.slip.ngay_muon.day) + "/" + std::to_string(r.slip.ngay_muon.month) + "/" + std::to_string(r.slip.ngay_muon.year);
      int w_id = 5;
      int w_date = 12;
      int w_cu = 25;
      int w_c = 30;
      int w_author = 20;
      int w_type = 15;

      auto row = hbox({
          text(std::to_string(r.slip.id_phieu)) | size(WIDTH, EQUAL, w_id), text(" \xe2\x94\x82 "),
          text(truncate_text(r.cu_name, w_cu)) | size(WIDTH, EQUAL, w_cu), text(" \xe2\x94\x82 "),
          text(truncate_text(r.c_name, w_c)) | size(WIDTH, EQUAL, w_c), text(" \xe2\x94\x82 "),
          text(truncate_text(r.c_author, w_author)) | size(WIDTH, EQUAL, w_author), text(" \xe2\x94\x82 "),
          text(truncate_text(r.c_type, w_type)) | size(WIDTH, EQUAL, w_type), text(" \xe2\x94\x82 "),
          text(nm) | size(WIDTH, EQUAL, w_date),
          filler()
      });
      if (state.focused) { row = row | inverted; }
      if (state.active) { row = row | bold; }
      return row;
  };
  Component slip_menu = Menu(&dummy_entries, &selected_slip_index, slip_menu_opt);

  Component exit_button = Button("Huy & Tro Ve (ESC)", [&] { 
      final_chosen_id = -1;
      form_screen.ExitLoopClosure()(); 
  }, ButtonOption::Animated());

  auto left_controls = Container::Vertical({
     input_search_comic, input_search_author, input_search_cu, input_search_ty, type_menu_c,
     t_id, t_c, t_cu, t_d, exit_button
  });

  auto main_container = Container::Horizontal({
     left_controls, slip_menu
  });
  
  auto controls_event = CatchEvent(main_container, [&](Event event) {
     if (event == Event::Escape) {
        final_chosen_id = -1;
        form_screen.ExitLoopClosure()();
        return true;
     }
     return false;
  });

  auto ui_renderer = Renderer(controls_event, [&] {
     refresh_type_suggestions(type_options, search_type, filtered_type_options, selected_type_option);
     apply_filter();
     
     auto type_dropdown_box = vbox({
         input_search_ty->Render(),
         type_menu_c->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 5) | borderEmpty
     });

     auto filter_panel = vbox({
        text("--- BỘ LỌC TÌM KIẾM --- ") | bold,
        separator(),
        hbox(text(" Tên Truyện: "), input_search_comic->Render()),
        hbox(text(" Tác Giả:    "), input_search_author->Render()),
        hbox(text(" Khách Hàng: "), input_search_cu->Render()),
        hbox(text(" Thể loại:   "), type_dropdown_box),
        separator(),
        text("--- SẮP XẾP ---") | bold,
        hbox(text(" ID Phiếu:   "), t_id->Render()),
        hbox(text(" Tên Truyện: "), t_c->Render()),
        hbox(text(" Tên Khách:  "), t_cu->Render()),
        hbox(text(" Ngày Mượn:  "), t_d->Render()),
        separator(),
        text(" Phím -> để Chuyển sang chọn") | color(Color::Green) | bold,
        exit_button->Render() | center
     }) | border | size(WIDTH, GREATER_THAN, 35);

     dummy_entries.resize(filtered_rows.size(), "");
     if (selected_slip_index >= (int)filtered_rows.size()) selected_slip_index = std::max(0, (int)filtered_rows.size() - 1);
     int w_id = 5;
     int w_date = 12;
     int w_cu = 25;
     int w_c = 30;
     int w_author = 20;
     int w_type = 15;

     auto header = hbox({
        text("ID") | size(WIDTH, EQUAL, w_id), text(" \xe2\x94\x82 "),
        text(truncate_text("Khach Hang", w_cu)) | size(WIDTH, EQUAL, w_cu), text(" \xe2\x94\x82 "),
        text(truncate_text("Ten Truyen", w_c)) | size(WIDTH, EQUAL, w_c), text(" \xe2\x94\x82 "),
        text(truncate_text("Tac Gia", w_author)) | size(WIDTH, EQUAL, w_author), text(" \xe2\x94\x82 "),
        text(truncate_text("The Loai", w_type)) | size(WIDTH, EQUAL, w_type), text(" \xe2\x94\x82 "),
        text("Ngay Muon") | size(WIDTH, EQUAL, w_date),
        filler()
     }) | bold;

     auto table_panel = window(
        text(" DANH SACH PHIEU (" + std::to_string(filtered_rows.size()) + ") - BẤM ENTER ĐỂ CHỌN ") | bold | center,
        vbox({
            header,
            separatorLight(),
            slip_menu->Render() | vscroll_indicator | frame | flex
        }) | border
     ) | flex;

     return window(text(" " + title + " ") | bold | center, hbox({filter_panel, table_panel}));
  });

  form_screen.Loop(ui_renderer);
  system("cls");
  return final_chosen_id;
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
  table_data.push_back({"ID", "Khach Hang", "Ten Truyen", "Tac Gia", "The Loai", "Ngay Muon", "Trang Thai"});
  for (const auto &s : slips) {
    std::string nm = std::to_string(s.ngay_muon.day) + "/" + std::to_string(s.ngay_muon.month) + "/" + std::to_string(s.ngay_muon.year);
    std::string tt = (s.trang_thai == 0) ? "Dang thue" : (s.trang_thai == 1) ? "Da tra" : (s.trang_thai == 2) ? "Mat/Hong" : "Qua han";
    std::string cu_name = get_cu_name(s.customer_id, all_cu);
    std::string c_name = get_c_name(s.comic_id, all_c);
    std::string c_author = get_c_author(s.comic_id, all_c);
    std::string c_type = get_c_type(s.comic_id, all_c);
    table_data.push_back({std::to_string(s.id_phieu), truncate_text(cu_name, 25), truncate_text(c_name, 25), truncate_text(c_author, 15), truncate_text(c_type, 15), nm, tt});
  }
  auto table = Table(table_data);
  table.SelectAll().Border(LIGHT);
  table.SelectRow(0).Decorate(bold);
  table.SelectAll().SeparatorVertical(LIGHT);
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
            std::string search_comic = "";
            std::string search_author = "";
            std::string search_cu = "";
            std::string search_type = "";
            std::vector<std::string> status_options = {"Tat ca", "Dang thue", "Da tra/Khac"};
            int selected_status = 0;
            
            std::vector<std::string> sort_options = {"-", "Tang dan", "Giam dan"};
            std::vector<std::string> sort_options_az = {"-", "A -> Z", "Z -> A"};
            int sort_id = 0, sort_c = 0, sort_cu = 0, sort_date = 0;
            int last_clicked_sort = 0;
            
            std::vector<RentalSlip> filtered_slips;
            
            std::vector<Comic> all_c = read_all_comics();
            std::vector<Customer> all_cu = read_all_customers();

            std::vector<std::string> type_options = build_type_options(all_c);
            std::vector<std::string> filtered_type_options;
            int selected_type_option = 0;
            
            std::vector<RentalUIRow> active_rows;
            for (const auto& s : all_slips) {
                RentalUIRow r;
                r.slip = s;
                r.cu_name = get_cu_name(s.customer_id, all_cu);
                r.c_name = get_c_name(s.comic_id, all_c);
                r.c_author = get_c_author(s.comic_id, all_c);
                r.c_type = get_c_type(s.comic_id, all_c);
                active_rows.push_back(r);
            }

            auto apply_filter = [&] {
                filtered_slips.clear();
                std::vector<RentalUIRow> t_rows;
                std::string s_comic = search_comic; for(auto&c : s_comic) c = std::tolower(c);
                std::string s_author = search_author; for(auto&c : s_author) c = std::tolower(c);
                std::string cu_kw = search_cu; for(auto&c : cu_kw) c = std::tolower(c);
                std::string ty_kw = search_type; for(auto&c : ty_kw) c = std::tolower(c);
                
                for (const auto& r : active_rows) {
                    std::string tn = r.c_name; for(auto&c : tn) c = std::tolower(c);
                    std::string ta = r.c_author; for(auto&c : ta) c = std::tolower(c);
                    std::string tk = r.cu_name; for(auto&c : tk) c = std::tolower(c);
                    std::string tl = r.c_type; for(auto&c : tl) c = std::tolower(c);
                    
                    bool m_comic = s_comic.empty() || tn.find(s_comic) != std::string::npos || std::to_string(r.slip.id_phieu) == s_comic;
                    bool m_author = s_author.empty() || ta.find(s_author) != std::string::npos;
                    bool m_cu = cu_kw.empty() || tk.find(cu_kw) != std::string::npos;
                    bool m_ty = ty_kw.empty() || tl.find(ty_kw) != std::string::npos;
                    bool m_st = true;
                    if (selected_status == 1) m_st = (r.slip.trang_thai == 0);
                    if (selected_status == 2) m_st = (r.slip.trang_thai != 0);

                    if (m_comic && m_author && m_cu && m_ty && m_st) {
                        t_rows.push_back(r);
                    }
                }
                
                if (last_clicked_sort == 1) {
                    if (sort_id == 1) quick_sort(t_rows, cmp_rui_id_asc);
                    else if (sort_id == 2) quick_sort(t_rows, cmp_rui_id_desc);
                } else if (last_clicked_sort == 2) {
                    if (sort_c == 1) quick_sort(t_rows, cmp_rui_c_asc);
                    else if (sort_c == 2) quick_sort(t_rows, cmp_rui_c_desc);
                } else if (last_clicked_sort == 3) {
                    if (sort_cu == 1) quick_sort(t_rows, cmp_rui_cu_asc);
                    else if (sort_cu == 2) quick_sort(t_rows, cmp_rui_cu_desc);
                } else if (last_clicked_sort == 4) {
                    if (sort_date == 1) quick_sort(t_rows, cmp_rui_date_asc);
                    else if (sort_date == 2) quick_sort(t_rows, cmp_rui_date_desc);
                }
                
                for (auto& r : t_rows) filtered_slips.push_back(r.slip);
            };

            Component input_search_comic = Input(&search_comic, "Tim ID hoac Ten Truyen...");
            Component input_search_author = Input(&search_author, "Tim theo Tac Gia...");
            Component input_search_cu = Input(&search_cu, "Tim Khach hang/SDT...");
            Component input_search_ty = Input(&search_type, "Go de tim The Loai...");
            Component status_radiobox = Radiobox(&status_options, &selected_status);
            
            auto type_menu_raw = Menu(&filtered_type_options, &selected_type_option);
            auto type_menu_c = CatchEvent(type_menu_raw, [&](Event event) {
              if (event.is_mouse() && event.mouse().motion == Mouse::Moved) return true;
              if (event == Event::Return) {
                 if (!filtered_type_options.empty() && is_valid_type_suggestion(filtered_type_options[selected_type_option])) {
                     search_type = filtered_type_options[selected_type_option];
                 }
                 return true; 
              }
              return false;
            });

            auto t_id = Toggle(&sort_options, &sort_id);
            auto t_c = Toggle(&sort_options_az, &sort_c);
            auto t_cu = Toggle(&sort_options_az, &sort_cu);
            auto t_d = Toggle(&sort_options, &sort_date);
            
            t_id |= CatchEvent([&](Event e) { if(e==Event::Return || e.is_mouse()) { last_clicked_sort=1; sort_c=0; sort_cu=0; sort_date=0; } return false; });
            t_c |= CatchEvent([&](Event e) { if(e==Event::Return || e.is_mouse()) { last_clicked_sort=2; sort_id=0; sort_cu=0; sort_date=0; } return false; });
            t_cu |= CatchEvent([&](Event e) { if(e==Event::Return || e.is_mouse()) { last_clicked_sort=3; sort_id=0; sort_c=0; sort_date=0; } return false; });
            t_d |= CatchEvent([&](Event e) { if(e==Event::Return || e.is_mouse()) { last_clicked_sort=4; sort_id=0; sort_c=0; sort_cu=0; } return false; });

            Component exit_button = Button("Tro Ve", [&] { form_screen.ExitLoopClosure()(); }, ButtonOption::Animated());

            auto controls = Container::Vertical({
                input_search_comic, input_search_author, input_search_cu, input_search_ty, type_menu_c, status_radiobox,
                t_id, t_c, t_cu, t_d, exit_button
            });

            auto controls_with_event = CatchEvent(controls, [&](Event event) {
                if (event == Event::Escape) {
                    form_screen.ExitLoopClosure()();
                    return true;
                }
                return false;
            });

            auto ui_renderer = Renderer(controls_with_event, [&] {
                refresh_type_suggestions(type_options, search_type, filtered_type_options, selected_type_option);
                apply_filter();
                
                auto type_dropdown_box = vbox({
                    input_search_ty->Render(),
                    type_menu_c->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 5) | borderEmpty
                });

                auto filter_panel = vbox({
                    text("--- BỘ LỌC === ") | bold,
                    separator(),
                    hbox(text(" Tên Truyện: "), input_search_comic->Render()),
                    hbox(text(" Tác Giả:    "), input_search_author->Render()),
                    hbox(text(" Khách:      "), input_search_cu->Render()),
                    hbox(text(" Thể loại:   "), type_dropdown_box),
                    separator(),
                    text(" Trang thai: ") | bold,
                    status_radiobox->Render(),
                    separator(),
                    text("--- SẮP XẾP ---") | bold,
                    hbox(text(" ID Phiếu:   "), t_id->Render()),
                    hbox(text(" Tên Truyện: "), t_c->Render()),
                    hbox(text(" Tên Khách:  "), t_cu->Render()),
                    hbox(text(" Ngày Mượn:  "), t_d->Render()),
                    separator(),
                    exit_button->Render() | center
                }) | border | size(WIDTH, GREATER_THAN, 35);

                auto table_panel = window(
                    text(" DANH SACH TẤT CẢ PHIẾU (" + std::to_string(filtered_slips.size()) + ")") | bold | center,
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

