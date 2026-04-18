#include "../../include/ui/CustomerUI.h"
#include "../../include/repository/CustomerRepo.h"
#include "../../include/utils/InputHandler.h"
#include "../../include/utils/ValidationUtils.h"
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/terminal.hpp>

#include "../../include/ui/UITheme.h"

using namespace ftxui;

Element build_customer_table_element(const std::vector<Customer> &customers) {
  if (customers.empty()) {
    return text("Khong co du lieu khach hang.") | center;
  }

  std::vector<std::vector<std::string>> table_data;
  table_data.push_back({"ID", "Ten Khach Hang", "So Dien Thoai"});

  bool has_data = false;
  for (const auto &c : customers) {
    if (!c.is_deleted) {
      has_data = true;
      table_data.push_back({std::to_string(c.id), truncate_text(c.name, 35), truncate_text(c.phone, 15)});
    }
  }

  if (!has_data) {
    return text("Khong co du lieu (hoac da bi xoa).") | center;
  }

  auto table = Table(table_data);
  table.SelectAll().Border(LIGHT);
  table.SelectRow(0).Decorate(bold);
  table.SelectAll().SeparatorVertical(LIGHT);
  table.SelectRow(0).Border(DOUBLE);

  return table.Render();
}

int select_customer_ui(const std::string& title) {
  system("cls");
  std::vector<Customer> all = read_all_customers();
  if (all.empty()) {
     std::cout << "Khong co khach hang nao trong he thong!\n";
     get_string_input("Nhan Enter de tiep tuc...");
     return -1;
  }

  auto form_screen = ScreenInteractive::Fullscreen();
  std::string search_kw = "";
  std::vector<Customer> filtered_list = all;

  auto apply_filter = [&] {
     filtered_list.clear();
     std::string kw = search_kw;
     for (char &c : kw) c = std::tolower(c);
     for (const auto& c : all) {
        std::string n = c.name; for (char &ch : n) ch = std::tolower(ch);
        std::string p = c.phone;
        bool ok = kw.empty() || n.find(kw) != std::string::npos || p.find(kw) != std::string::npos || std::to_string(c.id) == kw;
        if (ok && !c.is_deleted) {
           filtered_list.push_back(c);
        }
     }
  };

  Component input_search = Input(&search_kw, "ID, SDT hoac Ten...");
  
  int selected_customer_index = 0;
  int final_chosen_id = -1;
  std::vector<std::string> dummy_entries;
  
  MenuOption customer_menu_opt;
  customer_menu_opt.on_enter = [&] {
     if (!filtered_list.empty() && selected_customer_index >= 0 && selected_customer_index < (int)filtered_list.size()) {
        final_chosen_id = filtered_list[selected_customer_index].id;
        form_screen.ExitLoopClosure()();
     }
  };
  
  customer_menu_opt.entries_option.transform = [](const EntryState& state) { return text(""); };
  Component customer_menu = Menu(&dummy_entries, &selected_customer_index, customer_menu_opt);

  Component exit_button = Button("Huy & Tro Ve (ESC)", [&] { 
      final_chosen_id = -1;
      form_screen.ExitLoopClosure()(); 
  }, ButtonOption::Animated());

  auto left_controls = Container::Vertical({
     input_search,
     exit_button
  });

  auto controls = Container::Horizontal({
     left_controls,
     customer_menu
  });
  ftxui::Box table_box;

  auto controls_event = CatchEvent(controls, [&](Event event) {
     if (event == Event::Escape) {
        final_chosen_id = -1;
        form_screen.ExitLoopClosure()();
        return true;
     }

     if (event.is_mouse() && table_box.Contain(event.mouse().x, event.mouse().y)) {
        int hovered_row = event.mouse().y - table_box.y_min - 3;
        int max_idx = (int)filtered_list.size() - 1;
        if (hovered_row >= 0 && hovered_row <= max_idx) {
            if (event.mouse().button == Mouse::Left && event.mouse().motion == Mouse::Pressed) {
                selected_customer_index = hovered_row;
                final_chosen_id = filtered_list[hovered_row].id;
                form_screen.ExitLoopClosure()();
                return true;
            } else if (event.mouse().motion == Mouse::Moved) {
                selected_customer_index = hovered_row;
            }
        }
     }
     return false;
  });

  auto ui_renderer = Renderer(controls_event, [&] {
     apply_filter();
     auto filter_panel = vbox({
        text("--- BỘ LỌC TÌM KIẾM --- ") | bold,
        separator(),
        hbox(text(" Tim kiem: "), input_search->Render()),
        separator(),
        text(" Dùng Phím -> để Chuyển sang chọn") | color(Color::Green) | bold,
        exit_button->Render() | center
     }) | border | size(WIDTH, GREATER_THAN, 30);

     dummy_entries.resize(filtered_list.size(), "");
     if (selected_customer_index >= (int)filtered_list.size()) selected_customer_index = std::max(0, (int)filtered_list.size() - 1);

     Element table_element;
     if (filtered_list.empty()) {
         table_element = text("Khong co du lieu khach hang.") | center;
     } else {
         std::vector<std::vector<std::string>> table_data;
         table_data.push_back({"ID", "Ten Khach Hang", "SDT"});
         for (const auto& c : filtered_list) {
             table_data.push_back({
                 std::to_string(c.id), truncate_text(c.name, 35),
                 c.phone
             });
         }
         
         auto table = Table(table_data);
         table.SelectAll().Border(LIGHT);
         table.SelectRow(0).Decorate(bold);
         table.SelectAll().SeparatorVertical(LIGHT);
         table.SelectRow(0).Border(DOUBLE);

         int row_index = selected_customer_index + 1;
         if (customer_menu->Focused()) {
             table.SelectRow(row_index).Decorate(inverted);
         } else {
             table.SelectRow(row_index).Decorate(bold);
         }
         table_element = table.Render() | reflect(table_box);
     }

     auto table_panel = window(
        text(" DANH SACH KHACH (" + std::to_string(filtered_list.size()) + ") - BẤM ENTER ĐỂ CHỌN ") | bold | center,
        table_element | vscroll_indicator | frame | flex
     ) | flex;

     return window(text(" " + title + " ") | bold | center, hbox({filter_panel, table_panel}));
  });

  form_screen.Loop(ui_renderer);
  system("cls");
  return final_chosen_id;
}


#include "../../include/ui/customer/CustomerViewUI.h"
#include "../../include/ui/customer/CustomerAddUI.h"
#include "../../include/ui/customer/CustomerEditUI.h"
#include "../../include/ui/customer/CustomerDeleteUI.h"

void render_customer_menu() {
  auto screen = ScreenInteractive::TerminalOutput();
  std::vector<std::string> entries = {
      " [1] Xem danh sách khách ", 
      " [2] Thêm khách hàng mới ",
      " [3] Sửa thông tin khách ", 
      " [4] Xóa khách hàng      ",
      " [5] Trở về Menu chính   "
  };
  int selected = 0;
  MenuOption option;
  option.on_enter = screen.ExitLoopClosure();
  auto menu = Menu(&entries, &selected, option);
  auto menu_with_event = CatchEvent(menu, [&](Event event) {
      if (event == Event::Escape) { selected = entries.size() - 1; screen.ExitLoopClosure()(); return true; }
      if (event.is_character()) {
          char c = event.character()[0];
          if (c >= '1' && c <= '5') {
              int index = c - '1';
              if (index < (int)entries.size()) { selected = index; screen.ExitLoopClosure()(); return true; }
          }
      }
      if (event.is_mouse() && event.mouse().button == ftxui::Mouse::Left && event.mouse().motion == ftxui::Mouse::Pressed) {
          if (menu->OnEvent(event)) { if (option.on_enter) { option.on_enter(); } return true; }
      }
      return false;
  });
  
  auto renderer = Renderer(menu_with_event, [&]() -> Element {
    auto sidebar = window(
        text(" QUẢN LÝ KHÁCH HÀNG ") | bold | center, 
        menu_with_event->Render() | vscroll_indicator | frame
    ) | size(WIDTH, EQUAL, 32);

    auto main_area = window(
        text(" CHỨC NĂNG ") | bold | center,
        vbox({
            text(" Chào mừng đến với module Quản lý Khách Hàng.") | center,
            text(" Dùng phím (1-5) để chọn chức năng nhanh.") | color(ui::theme::kTextMutedColor) | center
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
    if (selected == 0) { handle_view_customers(); } 
    else if (selected == 1) { handle_add_customer(); } 
    else if (selected == 2) { handle_edit_customer(); } 
    else if (selected == 3) { handle_delete_customer(); } 
    else if (selected == 4) { system("cls"); break; }
  }
}
