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

void render_customer_table(const std::vector<Customer> &customers) {
  if (customers.empty()) {
    std::cout << "Khong co du lieu khach hang.\n";
    return;
  }

  std::vector<std::vector<std::string>> table_data;
  table_data.push_back({"Hinh Thuc", "Ten Khach Hang", "So Dien Thoai"});

  bool has_data = false;
  for (const auto &c : customers) {
    if (!c.is_deleted) {
      has_data = true;
      table_data.push_back({std::to_string(c.id), c.name, c.phone});
    }
  }

  if (!has_data) {
    std::cout << "Khong co du lieu (hoac da bi xoa).\n";
    return;
  }

  auto table = Table(table_data);
  table.SelectAll().Border(LIGHT);
  table.SelectRow(0).Decorate(bold);
  table.SelectRow(0).SeparatorVertical(LIGHT);
  table.SelectRow(0).Border(DOUBLE);

  auto document = table.Render();
  auto screen = Screen::Create(Dimension::Fit(document), Dimension::Fit(document));
  Render(screen, document);
  screen.Print();
  std::cout << "\n";
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

  if (selected == entries.size() - 1) {
    return -1;
  }
  return found_list[selected].id;
}

void render_customer_menu() {
  auto screen = ScreenInteractive::TerminalOutput();

  std::vector<std::string> entries = {
      "1. Xem danh sach khach", "2. Them khach hang",
      "3. Sua khach hang", "4. Xoa khach hang",
      "5. Tim kiem khach hang",      "6. Tro ve"};
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
    screen.Loop(renderer);

    if (selected == 0) {
      system("cls");
      std::cout << "\n--- DANH SACH KHACH HANG ---\n";
      std::vector<Customer> all = read_all_customers();
      render_customer_table(all);
      get_string_input("Nhan Enter de tiep tuc...");
    } else if (selected == 1) {
      system("cls");
      std::cout << "\n--- THEM KHACH HANG MOI ---\n";
      Customer new_c;

      std::string name = get_string_input("Nhap ten Khach: ");
      if (name == "[ESC]") continue;
      if (is_empty_string(name)) {
        std::cout << "Loi: Ten khong duoc de trong!\n";
        get_string_input("Nhan Enter de tiep tuc...");
        continue;
      }
      std::string phone = get_string_input("Nhap So dien thoai: ");
      if (phone == "[ESC]") continue;
      if (is_empty_string(phone)) {
        std::cout << "Loi: SDT khong duoc de trong!\n";
        get_string_input("Nhan Enter de tiep tuc...");
        continue;
      }

      if (is_customer_duplicate(phone)) {
        std::cout << "Loi: So dien thoai da ton tai trong he thong!\n";
        get_string_input("Nhan Enter de tiep tuc...");
        continue;
      }

      strncpy(new_c.name, name.c_str(), sizeof(new_c.name) - 1);
      new_c.name[sizeof(new_c.name) - 1] = '\0';

      strncpy(new_c.phone, phone.c_str(), sizeof(new_c.phone) - 1);
      new_c.phone[sizeof(new_c.phone) - 1] = '\0';

      new_c.is_deleted = false;

      add_customer(new_c);
      std::cout << "Them khach hang thanh cong!\n";
      get_string_input("Nhan Enter de tiep tuc...");
    } else if (selected == 2) {
      system("cls");
      std::cout << "\n--- SUA KHACH HANG ---\n";
      int id = select_customer_ui("CHON KHACH HANG DE SUA");
      if (id == -1) continue;

      system("cls");
      std::cout << "\n--- SUA THONG TIN KHACH HANG ---\n";
      Customer c_to_edit;
      if (get_customer_by_id(id, c_to_edit)) {
        std::cout << "Dang sua khach: " << c_to_edit.name << " (" << c_to_edit.phone << ")\n";
        std::string name = get_string_input("Nhap ten moi: ");
        if (name == "[ESC]") continue;
        if (is_empty_string(name)) {
            std::cout << "Loi: Ten khong duoc de trong!\n";
            get_string_input("Nhan Enter... \n"); continue;
        }
        std::string phone = get_string_input("Nhap so dien thoai moi: ");
        if (phone == "[ESC]") continue;
        if (is_empty_string(phone)) {
            std::cout << "Loi: SDT khong duoc de trong!\n";
            get_string_input("Nhan Enter... \n"); continue;
        }

        if (phone != std::string(c_to_edit.phone) && is_customer_duplicate(phone)) {
            std::cout << "Loi: So dien thoai nay da duoc su dung bi trung!\n";
            get_string_input("Nhan Enter... \n"); continue;
        }

        strncpy(c_to_edit.name, name.c_str(), sizeof(c_to_edit.name) - 1);
        c_to_edit.name[sizeof(c_to_edit.name) - 1] = '\0';

        strncpy(c_to_edit.phone, phone.c_str(), sizeof(c_to_edit.phone) - 1);
        c_to_edit.phone[sizeof(c_to_edit.phone) - 1] = '\0';

        if (update_customer(c_to_edit)) {
          std::cout << "Sua thanh cong!\n";
        } else {
          std::cout << "Loi khi sua!\n";
        }
      }
      get_string_input("Nhan Enter de tiep tuc...");
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
      std::cout << "\n--- TIM KIEM KHACH HANG ---\n";
      std::string kw = get_string_input("Nhap ten khach can tim: ");
      if (kw == "[ESC]") continue;
      std::vector<Customer> res = search_customers_by_name(kw);
      render_customer_table(res);
      get_string_input("Nhan Enter de tiep tuc...");
    } else if (selected == 5) {
      system("cls");
      break;
    }
  }
}
