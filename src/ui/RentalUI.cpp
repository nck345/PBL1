#include "../../include/ui/RentalUI.h"
#include "../../include/repository/ComicRepo.h"
#include "../../include/repository/CustomerRepo.h"
#include "../../include/repository/RentalRepo.h"
#include "../../include/services/RentalService.h"
#include "../../include/ui/ComicUI.h"
#include "../../include/ui/CustomerUI.h"
#include "../../include/utils/InputHandler.h"
#include "../../include/utils/SortUtils.h"
#include "../../include/utils/ValidationUtils.h"
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/terminal.hpp>

#include "../../include/ui/UITheme.h"

using namespace ftxui;

// Helper to parse date strings
Date parse_date_string(const std::string &date_str) {
  Date d = {0, 0, 0};
  if (date_str.length() >= 8) {
    sscanf(date_str.c_str(), "%d/%d/%d", &d.day, &d.month, &d.year);
  }
  return d;
}

std::string get_c_name(int comic_id, const std::vector<Comic> &comics) {
  for (const auto &c : comics) {
    if (c.id == comic_id)
      return c.comic_name;
  }
  return "Unknown Comic";
}

std::string get_c_author(int comic_id, const std::vector<Comic> &comics) {
  for (const auto &c : comics) {
    if (c.id == comic_id)
      return c.author;
  }
  return "Unknown Author";
}

std::string get_c_type(int comic_id, const std::vector<Comic> &comics) {
  for (const auto &c : comics) {
    if (c.id == comic_id)
      return c.type;
  }
  return "Unknown Type";
}

std::string get_cu_name(int customer_id,
                        const std::vector<Customer> &customers) {
  for (const auto &c : customers) {
    if (c.id == customer_id)
      return std::string(c.name) + " (" + std::string(c.phone) + ")";
  }
  return "Unknown Customer";
}

bool cmp_rui_id_asc(const RentalUIRow &a, const RentalUIRow &b) {
  return a.slip.id_phieu < b.slip.id_phieu;
}
bool cmp_rui_id_desc(const RentalUIRow &a, const RentalUIRow &b) {
  return a.slip.id_phieu > b.slip.id_phieu;
}

bool cmp_rui_cu_asc(const RentalUIRow &a, const RentalUIRow &b) {
  return a.cu_name < b.cu_name;
}
bool cmp_rui_cu_desc(const RentalUIRow &a, const RentalUIRow &b) {
  return a.cu_name > b.cu_name;
}

bool cmp_rui_c_asc(const RentalUIRow &a, const RentalUIRow &b) {
  return a.c_name < b.c_name;
}
bool cmp_rui_c_desc(const RentalUIRow &a, const RentalUIRow &b) {
  return a.c_name > b.c_name;
}

bool cmp_rui_date_asc(const RentalUIRow &a, const RentalUIRow &b) {
  if (a.slip.ngay_muon.year != b.slip.ngay_muon.year)
    return a.slip.ngay_muon.year < b.slip.ngay_muon.year;
  if (a.slip.ngay_muon.month != b.slip.ngay_muon.month)
    return a.slip.ngay_muon.month < b.slip.ngay_muon.month;
  return a.slip.ngay_muon.day < b.slip.ngay_muon.day;
}
bool cmp_rui_date_desc(const RentalUIRow &a, const RentalUIRow &b) {
  return cmp_rui_date_asc(b, a);
}

int select_rental_slip_ui(const std::string &title) {
  system("cls");
  std::vector<RentalSlip> all_slips = get_all_rental_slips();
  std::vector<Comic> all_c = read_all_comics();
  std::vector<Customer> all_cu = read_all_customers();

  std::vector<RentalUIRow> active_rows;
  for (const auto &s : all_slips) {
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
    std::string s_comic = search_comic;
    for (char &c : s_comic)
      c = std::tolower(c);
    std::string s_author = search_author;
    for (char &c : s_author)
      c = std::tolower(c);
    std::string cu_kw = search_cu;
    for (char &c : cu_kw)
      c = std::tolower(c);
    std::string ty_kw = search_type;
    for (char &c : ty_kw)
      c = std::tolower(c);

    for (const auto &r : active_rows) {
      std::string tn = r.c_name;
      for (auto &c : tn)
        c = std::tolower(c);
      std::string ta = r.c_author;
      for (auto &c : ta)
        c = std::tolower(c);
      std::string tk = r.cu_name;
      for (auto &c : tk)
        c = std::tolower(c);
      std::string tl = r.c_type;
      for (auto &c : tl)
        c = std::tolower(c);

      bool m_comic = s_comic.empty() || tn.find(s_comic) != std::string::npos ||
                     std::to_string(r.slip.id_phieu) == s_comic;
      bool m_author =
          s_author.empty() || ta.find(s_author) != std::string::npos;
      bool m_cu = cu_kw.empty() || tk.find(cu_kw) != std::string::npos;
      bool m_ty = ty_kw.empty() || tl.find(ty_kw) != std::string::npos;

      if (m_comic && m_author && m_cu && m_ty)
        filtered_rows.push_back(r);
    }

    if (last_clicked_sort == 1) {
      if (sort_id == 1)
        quick_sort(filtered_rows, cmp_rui_id_asc);
      else if (sort_id == 2)
        quick_sort(filtered_rows, cmp_rui_id_desc);
    } else if (last_clicked_sort == 2) {
      if (sort_c == 1)
        quick_sort(filtered_rows, cmp_rui_c_asc);
      else if (sort_c == 2)
        quick_sort(filtered_rows, cmp_rui_c_desc);
    } else if (last_clicked_sort == 3) {
      if (sort_cu == 1)
        quick_sort(filtered_rows, cmp_rui_cu_asc);
      else if (sort_cu == 2)
        quick_sort(filtered_rows, cmp_rui_cu_desc);
    } else if (last_clicked_sort == 4) {
      if (sort_date == 1)
        quick_sort(filtered_rows, cmp_rui_date_asc);
      else if (sort_date == 2)
        quick_sort(filtered_rows, cmp_rui_date_desc);
    }
  };

  Component input_search_comic =
      Input(&search_comic, "Tim ID hoac Ten Truyen...");
  Component input_search_author = Input(&search_author, "Tim theo Tac Gia...");
  Component input_search_cu = Input(&search_cu, "Tim Khach hang/SDT...");
  Component input_search_ty = Input(&search_type, "Go de tim The Loai...");

  auto type_menu_raw = Menu(&filtered_type_options, &selected_type_option);
  auto type_menu_c = CatchEvent(type_menu_raw, [&](Event event) {
    if (event.is_mouse() && event.mouse().motion == Mouse::Moved)
      return true;
    if (event == Event::Return) {
      if (!filtered_type_options.empty() &&
          is_valid_type_suggestion(
              filtered_type_options[selected_type_option])) {
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

  t_id |= CatchEvent([&](Event e) {
    if (e == Event::Return || e.is_mouse()) {
      last_clicked_sort = 1;
      sort_c = 0;
      sort_cu = 0;
      sort_date = 0;
    }
    return false;
  });
  t_c |= CatchEvent([&](Event e) {
    if (e == Event::Return || e.is_mouse()) {
      last_clicked_sort = 2;
      sort_id = 0;
      sort_cu = 0;
      sort_date = 0;
    }
    return false;
  });
  t_cu |= CatchEvent([&](Event e) {
    if (e == Event::Return || e.is_mouse()) {
      last_clicked_sort = 3;
      sort_id = 0;
      sort_c = 0;
      sort_date = 0;
    }
    return false;
  });
  t_d |= CatchEvent([&](Event e) {
    if (e == Event::Return || e.is_mouse()) {
      last_clicked_sort = 4;
      sort_id = 0;
      sort_c = 0;
      sort_cu = 0;
    }
    return false;
  });

  int selected_slip_index = 0;
  int final_chosen_id = -1;
  std::vector<std::string> dummy_entries;

  MenuOption slip_menu_opt;
  slip_menu_opt.on_enter = [&] {
    if (!filtered_rows.empty() && selected_slip_index >= 0 &&
        selected_slip_index < (int)filtered_rows.size()) {
      final_chosen_id = filtered_rows[selected_slip_index].slip.id_phieu;
      form_screen.ExitLoopClosure()();
    }
  };

  Component slip_menu =
      Menu(&dummy_entries, &selected_slip_index, slip_menu_opt);

  Component exit_button = Button(
      "Huy & Tro Ve (ESC)",
      [&] {
        final_chosen_id = -1;
        form_screen.ExitLoopClosure()();
      },
      ButtonOption::Animated());

  auto left_controls = Container::Vertical(
      {input_search_comic, input_search_author, input_search_cu,
       input_search_ty, type_menu_c, t_id, t_c, t_cu, t_d, exit_button});

  auto main_container = Container::Horizontal({left_controls, slip_menu});
  ftxui::Box table_box;

  auto controls_event = CatchEvent(main_container, [&](Event event) {
    if (event == Event::Escape) {
      final_chosen_id = -1;
      form_screen.ExitLoopClosure()();
      return true;
    }

    if (event.is_mouse() &&
        table_box.Contain(event.mouse().x, event.mouse().y)) {
      int hovered_row = event.mouse().y - table_box.y_min - 3;
      int max_idx = (int)filtered_rows.size() - 1;
      if (hovered_row >= 0 && hovered_row <= max_idx) {
        if (event.mouse().button == Mouse::Left &&
            event.mouse().motion == Mouse::Pressed) {
          selected_slip_index = hovered_row;
          final_chosen_id = filtered_rows[hovered_row].slip.id_phieu;
          form_screen.ExitLoopClosure()();
          return true;
        } else if (event.mouse().motion == Mouse::Moved) {
          selected_slip_index = hovered_row;
        }
      }
    }
    return false;
  });

  auto ui_renderer = Renderer(controls_event, [&] {
    refresh_type_suggestions(type_options, search_type, filtered_type_options,
                             selected_type_option);
    apply_filter();

    auto type_dropdown_box =
        vbox({input_search_ty->Render(),
              type_menu_c->Render() | vscroll_indicator | frame |
                  size(HEIGHT, LESS_THAN, 5) | borderEmpty});

    auto filter_panel =
        vbox({text("--- BỘ LỌC TÌM KIẾM --- ") | bold, separator(),
              hbox(text(" Tên Truyện: "), input_search_comic->Render()),
              hbox(text(" Tác Giả:    "), input_search_author->Render()),
              hbox(text(" Khách Hàng: "), input_search_cu->Render()),
              hbox(text(" Thể loại:   "), type_dropdown_box), separator(),
              text("--- SẮP XẾP ---") | bold,
              hbox(text(" ID Phiếu:   "), t_id->Render()),
              hbox(text(" Tên Truyện: "), t_c->Render()),
              hbox(text(" Tên Khách:  "), t_cu->Render()),
              hbox(text(" Ngày Mượn:  "), t_d->Render()), separator(),
              text(" Phím -> để Chuyển sang chọn") | color(Color::Green) | bold,
              exit_button->Render() | center}) |
        border | size(WIDTH, GREATER_THAN, 35);

    dummy_entries.resize(filtered_rows.size(), "");
    if (selected_slip_index >= (int)filtered_rows.size())
      selected_slip_index = std::max(0, (int)filtered_rows.size() - 1);
    Element table_element;
    if (filtered_rows.empty()) {
      table_element = text("Khong co du lieu phieu muon.") | center;
    } else {
      std::vector<std::vector<std::string>> table_data;
      table_data.push_back({"ID", "Khach Hang", "Ten Truyen", "Tac Gia",
                            "The Loai", "Ngay Muon"});
      int term_width = ftxui::Terminal::Size().dimx;
      int remaining = std::max(20, term_width - 80);
      int dyn_cu = remaining / 2;
      int dyn_c = remaining - dyn_cu;

      for (const auto &r : filtered_rows) {
        std::string nm = std::to_string(r.slip.ngay_muon.day) + "/" +
                         std::to_string(r.slip.ngay_muon.month) + "/" +
                         std::to_string(r.slip.ngay_muon.year);
        table_data.push_back(
            {std::to_string(r.slip.id_phieu), truncate_text(r.cu_name, dyn_cu),
             truncate_text(r.c_name, dyn_c), truncate_text(r.c_author, 10),
             truncate_text(r.c_type, 12), nm});
      }

      auto table = Table(table_data);
      table.SelectAll().Border(LIGHT);
      table.SelectRow(0).Decorate(bold);
      table.SelectAll().SeparatorVertical(LIGHT);
      table.SelectRow(0).Border(DOUBLE);

      int row_index = selected_slip_index + 1;
      if (slip_menu->Focused()) {
        table.SelectRow(row_index).Decorate(inverted);
      } else {
        table.SelectRow(row_index).Decorate(bold);
      }
      table_element = table.Render() | reflect(table_box);
    }

    auto table_panel =
        window(text(" DANH SACH PHIEU (" +
                    std::to_string(filtered_rows.size()) +
                    ") - BẤM ENTER ĐỂ CHỌN ") |
                   bold | center,
               table_element | vscroll_indicator | frame | flex) |
        flex;

    return window(text(" " + title + " ") | bold | center,
                  hbox({filter_panel, table_panel}));
  });

  form_screen.Loop(ui_renderer);
  system("cls");
  return final_chosen_id;
}

Element build_rental_table_element(const std::vector<RentalSlip> &slips,
                                   const std::vector<Comic> &all_c,
                                   const std::vector<Customer> &all_cu) {
  if (slips.empty()) {
    return text("Khong co du lieu phiếu thuê.") | center;
  }
  std::vector<std::vector<std::string>> table_data;
  table_data.push_back({"ID", "Khach Hang", "Ten Truyen", "Tac Gia", "The Loai",
                        "Ngay Muon", "Trang Thai"});
  int term_width = ftxui::Terminal::Size().dimx;
  int remaining = std::max(20, term_width - 92);
  int dyn_cu = remaining / 2;
  int dyn_c = remaining - dyn_cu;

  for (const auto &s : slips) {
    std::string nm = std::to_string(s.ngay_muon.day) + "/" +
                     std::to_string(s.ngay_muon.month) + "/" +
                     std::to_string(s.ngay_muon.year);
    std::string tt = (s.trang_thai == 0)   ? "Dang thue"
                     : (s.trang_thai == 1) ? "Da tra"
                     : (s.trang_thai == 2) ? "Mat/Hong"
                     : (s.trang_thai == 3) ? "Dat truoc"
                                           : "Khong ro";
    std::string cu_name = get_cu_name(s.customer_id, all_cu);
    std::string c_name = get_c_name(s.comic_id, all_c);
    std::string c_author = get_c_author(s.comic_id, all_c);
    std::string c_type = get_c_type(s.comic_id, all_c);
    table_data.push_back({std::to_string(s.id_phieu),
                          truncate_text(cu_name, dyn_cu), truncate_text(c_name, dyn_c),
                          truncate_text(c_author, 10),
                          truncate_text(c_type, 12), nm, tt});
  }
  auto table = Table(table_data);
  table.SelectAll().Border(LIGHT);
  table.SelectRow(0).Decorate(bold);
  table.SelectAll().SeparatorVertical(LIGHT);
  table.SelectRow(0).Border(DOUBLE);
  return table.Render();
}

#include "../../include/ui/rental/RentalAddUI.h"
#include "../../include/ui/rental/RentalReturnUI.h"
#include "../../include/ui/rental/RentalViewUI.h"


void render_rental_menu() {
  auto screen = ScreenInteractive::TerminalOutput();
  std::vector<std::string> entries = {
      " [1] Danh sách Phiếu thuê ",
      " [2] Lập thẻ mượn mới     ", 
      " [3] Trả sách & Thu tiền  ",
      " [4] Trở về Menu chính    "
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
      if (c >= '1' && c <= '4') {
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
    auto sidebar = window(
        text(" QUẢN LÝ PHIẾU THUÊ ") | bold | center, 
        menu_with_event->Render() | vscroll_indicator | frame
    ) | size(WIDTH, EQUAL, 32);

    auto main_area = window(
        text(" CHỨC NĂNG ") | bold | center,
        vbox({
            text(" Chào mừng đến với module Quản lý Phiếu Thuê.") | center,
            text(" Dùng phím (1-4) để chọn chức năng nhanh.") | color(ui::theme::kTextMutedColor) | center
        }) | center
    ) | flex;

    auto layout = hbox({ sidebar, main_area }) | ui::theme::FocusedPanel() | flex;

    auto title = text(" QUẢN LÝ THUÊ TRUYỆN TRANH (PBL1) ") | ui::theme::AppTitle() | center;
    
    return vbox({
        text("") | size(HEIGHT, EQUAL, 1),
        title,
        text("") | size(HEIGHT, EQUAL, 1),
        layout,
        text("") | size(HEIGHT, EQUAL, 1)
    }) | bgcolor(ui::theme::kBgColor) | borderEmpty | center;
  });

  while (true) {
    system("cls");
    screen.Loop(renderer);
    if (selected == 0) {
      handle_view_rentals();
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

