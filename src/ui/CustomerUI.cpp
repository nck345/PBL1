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
      table_data.push_back({std::to_string(c.id), c.name, c.phone});
    }
  }

  if (!has_data) {
    return text("Khong co du lieu (hoac da bi xoa).") | center;
  }

  auto table = Table(table_data);
  table.SelectAll().Border(LIGHT);
  table.SelectRow(0).Decorate(bold);
  table.SelectRow(0).SeparatorVertical(LIGHT);
  table.SelectRow(0).Border(DOUBLE);

  return table.Render();
}

int select_customer_ui(const std::string& title) {
  std::string keyword = get_string_input("Nhap So dien thoai can tim: ");
  if (keyword == "[ESC]") return -1;
  if (is_empty_string(keyword)) return -1;

  std::vector<Customer> found_list = search_customers_by_phone(keyword);
  
  if (found_list.empty()) {
    std::cout << "Khong tim thay khach hang voi SDT nay!\n";
    return -1;
  }

  auto screen = ScreenInteractive::TerminalOutput();
  std::vector<std::string> entries;
  for (size_t i = 0; i < found_list.size(); ++i) {
    std::string item = std::to_string(i + 1) + ". " + std::string(found_list[i].name) + " - SDT: " + found_list[i].phone;
    entries.push_back(item);
  }
  entries.push_back(std::to_string(found_list.size() + 1) + ". -> Huy thao tac");

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

  if (static_cast<size_t>(selected) == entries.size() - 1) {
    return -1;
  }
  return found_list[selected].id;
}

void render_customer_menu() {
  auto screen = ScreenInteractive::TerminalOutput();

  std::vector<std::string> entries = {
      "1. Xem danh sach khach", "2. Them khach hang",
      "3. Sua khach hang", "4. Xoa khach hang",
      "5. Tro ve"};
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
    return window(text(" QUAN LY KHACH HANG "),
                  menu_with_event->Render() | vscroll_indicator | frame) |
           bold;
  });

  while (true) {
    system("cls");
    screen.Loop(renderer);

    if (selected == 0) {
      system("cls");
      std::vector<Customer> all = read_all_customers();
      if (all.empty()) {
         std::cout << "Khong co khach hang nao trong he thong!\n";
         get_string_input("Nhan Enter de tiep tuc...");
         continue;
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
      Component exit_button = Button("Tro Ve", [&] { form_screen.ExitLoopClosure()(); }, ButtonOption::Animated());

      auto controls = Container::Vertical({
         input_search, exit_button
      });
      auto controls_event = CatchEvent(controls, [&](Event event) {
         if (event == Event::Escape) {
            form_screen.ExitLoopClosure()();
            return true;
         }
         return false;
      });

      auto ui_renderer = Renderer(controls_event, [&] {
         apply_filter();
         auto filter_panel = vbox({
            text("--- BỘ LỌC === ") | bold,
            separator(),
            hbox(text(" Tim kiem: "), input_search->Render()),
            separator(),
            exit_button->Render() | center
         }) | border | size(WIDTH, GREATER_THAN, 30);

         auto table_panel = window(
            text(" DANH SACH KHACH HANG (" + std::to_string(filtered_list.size()) + ")") | bold | center,
            build_customer_table_element(filtered_list) | vscroll_indicator | frame
         ) | flex;

         return hbox({filter_panel, table_panel});
      });

      form_screen.Loop(ui_renderer);
    } else if (selected == 1) {
      system("cls");
      auto form_screen = ScreenInteractive::TerminalOutput();
      
      std::string name_str = "";
      std::string phone_str = "";
      std::string error_msg = "";
      bool is_saved = false;

      Component input_name = Input(&name_str, "Nhap ten khach hang...");
      Component input_phone = Input(&phone_str, "Nhap so dien thoai...");

      auto submit_action = [&] {
        if (is_empty_string(name_str)) {
          error_msg = "Loi: Ten khong duoc de trong!";
          is_saved = false;
          return;
        }
        if (is_empty_string(phone_str)) {
          error_msg = "Loi: SDT khong duoc de trong!";
          is_saved = false;
          return;
        }
        if (!is_valid_phone_number(phone_str)) {
          error_msg = "Loi: SDT khong hop le (10 so, bat dau tu 0)!";
          is_saved = false;
          return;
        }
        if (is_customer_duplicate(phone_str)) {
          error_msg = "Loi: So dien thoai da ton tai trong he thong!";
          is_saved = false;
          return;
        }

        Customer new_c;
        strncpy(new_c.name, name_str.c_str(), sizeof(new_c.name) - 1);
        new_c.name[sizeof(new_c.name) - 1] = '\0';
        strncpy(new_c.phone, phone_str.c_str(), sizeof(new_c.phone) - 1);
        new_c.phone[sizeof(new_c.phone) - 1] = '\0';
        new_c.is_deleted = false;

        add_customer(new_c);
        is_saved = true;
        error_msg = "Them khach hang thanh cong! Nhan Huy hoac ESC de thoat.";
      };

      auto submit_button = Button("Xac nhan & Luu", submit_action, ButtonOption::Animated());

      auto cancel_button = Button("Huy & Tro ve", [&] {
        form_screen.ExitLoopClosure()();
      }, ButtonOption::Animated());

      auto container = Container::Vertical({
        input_name,
        input_phone,
        Container::Horizontal({submit_button, cancel_button})
      });

      auto container_with_event = CatchEvent(container, [&](Event event) {
        if (event == Event::Escape) {
          form_screen.ExitLoopClosure()();
          return true;
        }
        if (event == Event::Return) {
          if (input_name->Focused()) {
            input_phone->TakeFocus();
            return true;
          }
          if (input_phone->Focused()) {
            submit_action();
            if (is_saved) {
              form_screen.ExitLoopClosure()();
            }
            return true;
          }
        }
        return false;
      });

      auto renderer = Renderer(container_with_event, [&] {
        return vbox({
            text("--- THEM KHACH HANG MOI ---") | bold | center,
            separator(),
            hbox(text(" Ten KH:      "), input_name->Render()),
            hbox(text(" So DT:       "), input_phone->Render()),
            separator(),
            text(error_msg) | color(is_saved ? Color::Green : Color::Red) | center,
            separator(),
            hbox(submit_button->Render(), text("   "), cancel_button->Render()) | center
        }) | border;
      });

      form_screen.Loop(renderer);

    } else if (selected == 2) {
      system("cls");
      int id = select_customer_ui("CHON KHACH HANG DE SUA");
      if (id == -1) continue;

      system("cls");
      Customer c_to_edit;
      if (get_customer_by_id(id, c_to_edit)) {
        auto form_screen = ScreenInteractive::TerminalOutput();
        
        std::string name_str = c_to_edit.name;
        std::string phone_str = c_to_edit.phone;
        std::string orig_phone = c_to_edit.phone;
        std::string error_msg = "";
        bool is_saved = false;

        Component input_name = Input(&name_str, "Nhap ten moi...");
        Component input_phone = Input(&phone_str, "Nhap so dien thoai moi...");

        auto submit_action = [&] {
          if (is_empty_string(name_str)) {
            error_msg = "Loi: Ten khong duoc de trong!";
            is_saved = false;
            return;
          }
          if (is_empty_string(phone_str)) {
            error_msg = "Loi: SDT khong duoc de trong!";
            is_saved = false;
            return;
          }
          if (!is_valid_phone_number(phone_str)) {
            error_msg = "Loi: SDT khong hop le (10 so, bat dau tu 0)!";
            is_saved = false;
            return;
          }
          if (phone_str != orig_phone && is_customer_duplicate(phone_str)) {
            error_msg = "Loi: So dien thoai da bi trung voi khach khac!";
            is_saved = false;
            return;
          }

          strncpy(c_to_edit.name, name_str.c_str(), sizeof(c_to_edit.name) - 1);
          c_to_edit.name[sizeof(c_to_edit.name) - 1] = '\0';
          strncpy(c_to_edit.phone, phone_str.c_str(), sizeof(c_to_edit.phone) - 1);
          c_to_edit.phone[sizeof(c_to_edit.phone) - 1] = '\0';

          if (update_customer(c_to_edit)) {
            is_saved = true;
            error_msg = "Cap nhat thanh cong! Nhan Huy hoac ESC de thoat.";
          } else {
            error_msg = "Loi he thong khi cap nhat!";
            is_saved = false;
          }
        };

        auto submit_button = Button("Cap nhat", submit_action, ButtonOption::Animated());

        auto cancel_button = Button("Huy & Tro ve", [&] {
          form_screen.ExitLoopClosure()();
        }, ButtonOption::Animated());

        auto container = Container::Vertical({
          input_name,
          input_phone,
          Container::Horizontal({submit_button, cancel_button})
        });

        auto container_with_event = CatchEvent(container, [&](Event event) {
          if (event == Event::Escape) {
            form_screen.ExitLoopClosure()();
            return true;
          }
          if (event == Event::Return) {
            if (input_name->Focused()) {
              input_phone->TakeFocus();
              return true;
            }
            if (input_phone->Focused()) {
              submit_action();
              if (is_saved) {
                form_screen.ExitLoopClosure()();
              }
              return true;
            }
          }
          return false;
        });

        auto renderer = Renderer(container_with_event, [&] {
          return vbox({
              text("--- SUA THONG TIN KHACH HANG ---") | bold | center,
              separator(),
              text(" Dang sua: " + std::string(c_to_edit.name) + " (" + orig_phone + ")") | center,
              separator(),
              hbox(text(" Ten moi:     "), input_name->Render()),
              hbox(text(" SDT moi:     "), input_phone->Render()),
              separator(),
              text(error_msg) | color(is_saved ? Color::Green : Color::Red) | center,
              separator(),
              hbox(submit_button->Render(), text("   "), cancel_button->Render()) | center
          }) | border;
        });

        form_screen.Loop(renderer);
      } else {
        std::cout << "Khong tim thay khach hang ID nay!\n";
        get_string_input("Nhan Enter de tiep tuc...");
      }
    } else if (selected == 3) {
      system("cls");
      std::cout << "\n--- XOA KHACH HANG ---\n";
      int id = select_customer_ui("CHON KHACH HANG DE XOA");
      if (id == -1) continue;

      system("cls");
      if (delete_customer(id)) {
        std::cout << "Xoa khach hang thanh cong!\n";
      } else {
        std::cout << "Loi khi xoa khach!\n";
      }
      get_string_input("Nhan Enter de tiep tuc...");
    } else if (selected == 4) {
      system("cls");
      break;
    }
  }
}
