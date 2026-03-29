#include "../../include/ui/ComicUI.h"
#include "../../include/repository/ComicRepo.h"
#include "../../include/utils/InputHandler.h"
#include "../../include/utils/ValidationUtils.h"
#include "../../include/utils/SortUtils.h"
#include "../../include/utils/SearchUtils.h"
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

int select_comic_ui(const std::string& title) {
  std::vector<Comic> all_comics = read_all_comics();
  std::vector<Comic> active_comics;
  for (const auto& c : all_comics) {
    if (!c.is_deleted) {
      active_comics.push_back(c);
    }
  }

  if (active_comics.empty()) {
    std::cout << "Khong co truyen nao trong he thong!\n";
    return -1;
  }

  auto screen = ScreenInteractive::TerminalOutput();
  std::vector<std::string> entries;
  for (size_t i = 0; i < active_comics.size(); ++i) {
    std::string item = std::to_string(i + 1) + ". " + std::string(active_comics[i].comic_name) + " - " + active_comics[i].author + " (SL: " + std::to_string(active_comics[i].quantity) + ")";
    entries.push_back(item);
  }
  entries.push_back(std::to_string(active_comics.size() + 1) + ". -> Huy thao tac");

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
  return active_comics[selected].id;
}

void render_comic_table(const std::vector<Comic> &comics) {
  if (comics.empty()) {
    std::cout << "Khong co du lieu truyen.\n";
    return;
  }

  std::vector<std::vector<std::string>> table_data;
  table_data.push_back({"ID", "Ten Truyen", "Tac Gia", "Gia", "So Luong"});

  bool has_data = false;
  for (const auto &comic : comics) {
    if (!comic.is_deleted) {
      has_data = true;
      table_data.push_back({std::to_string(comic.id), comic.comic_name,
                            comic.author, format_currency(comic.price),
                            std::to_string(comic.quantity)});
    }
  }

  if (!has_data) {
    std::cout << "Khong co du lieu truyen (hoac da bi xoa).\n";
    return;
  }

  auto table = Table(table_data);
  table.SelectAll().Border(LIGHT);

  table.SelectRow(0).Decorate(bold);
  table.SelectRow(0).SeparatorVertical(LIGHT);
  table.SelectRow(0).Border(DOUBLE);

  // Alignment
  // table.SelectColumn(0).DecorateCells(align_right);

  auto document = table.Render();
  auto screen =
      Screen::Create(Dimension::Fit(document), Dimension::Fit(document));
  Render(screen, document);
  screen.Print();
  std::cout << "\n";
}

void render_comic_menu() {
  auto screen = ScreenInteractive::TerminalOutput();

  std::vector<std::string> entries = {
      "1. Xem danh sach truyen", "2. Them truyen moi",
      "3. Sua thong tin truyen", "4. Xoa truyen",
      "5. Tim kiem truyen",      "6. Tro ve"};
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
    return window(text(" QUAN LY TRUYEN "),
                  menu_with_event->Render() | vscroll_indicator | frame) |
           bold;
  });

  while (true) {
    system("cls");
    screen.Loop(renderer);

    if (selected == 0) {
      system("cls");

      std::vector<std::string> sort_entries = {
          "1. Truyen co san (Nguon goc)",
          "2. ID (Tang dan)", "3. ID (Giam dan)",
          "4. Ten (A-Z)", "5. Gia (Tang dan)", "6. Gia (Giam dan)", "7. Tro ve"
      };
      int sort_selected = 0;
      MenuOption sort_option;
      auto screen_sort = ScreenInteractive::TerminalOutput();
      sort_option.on_enter = screen_sort.ExitLoopClosure();
      auto sort_menu = Menu(&sort_entries, &sort_selected, sort_option);
      auto sort_menu_with_event = CatchEvent(sort_menu, [&](Event event) {
          if (event == Event::Escape) { sort_selected = 6; screen_sort.ExitLoopClosure()(); return true; }
          if (event.is_character() && event.character()[0] >= '1' && event.character()[0] <= '7') {
              sort_selected = event.character()[0] - '1'; screen_sort.ExitLoopClosure()(); return true;
          }
          return false;
      });
      auto sort_renderer = Renderer(sort_menu_with_event, [&] {
        return window(text(" CHON TIEU CHI SAP XEP "), sort_menu_with_event->Render() | vscroll_indicator | frame) | bold;
      });
      screen_sort.Loop(sort_renderer);

      if (sort_selected != 6) {
          system("cls");
          std::cout << "\n--- DANH SACH TRUYEN ---\n";
          std::vector<Comic> comics = read_all_comics();
          std::vector<Comic> active_comics;
          for (const auto& c : comics) {
              if (!c.is_deleted) active_comics.push_back(c);
          }
          
          if (sort_selected == 1) quick_sort(active_comics, compare_comic_by_id_asc);
          else if (sort_selected == 2) quick_sort(active_comics, compare_comic_by_id_desc);
          else if (sort_selected == 3) quick_sort(active_comics, compare_comic_by_name_asc);
          else if (sort_selected == 4) quick_sort(active_comics, compare_comic_by_price_asc);
          else if (sort_selected == 5) quick_sort(active_comics, compare_comic_by_price_desc);
          
          render_comic_table(active_comics);
          get_string_input("Nhan Enter de tiep tuc...");
      }
    } else if (selected == 1) {
      system("cls");
      auto form_screen = ScreenInteractive::TerminalOutput();
      
      std::string name_str = "";
      std::string author_str = "";
      std::string price_str = "";
      std::string quantity_str = "";
      std::string error_msg = "";
      bool is_saved = false;

      Component input_name = Input(&name_str, "Nhap ten truyen...");
      Component input_author = Input(&author_str, "Nhap tac gia...");
      Component input_price = Input(&price_str, "Nhap gia...");
      Component input_quantity = Input(&quantity_str, "Nhap so luong...");

      auto submit_button = Button("Xac nhan & Luu", [&] {
        if (is_empty_string(name_str)) {
            error_msg = "Loi: Ten khong duoc de trong!"; is_saved = false; return;
        }
        if (is_empty_string(author_str)) {
            error_msg = "Loi: Tac gia khong duoc de trong!"; is_saved = false; return;
        }
        if (is_comic_duplicate(name_str.c_str(), author_str.c_str())) {
            error_msg = "Loi: Truyen co ten va tac gia nay da ton tai!"; is_saved = false; return;
        }

        double price = 0.0;
        try {
            size_t pos;
            price = std::stod(price_str, &pos);
            if (pos != price_str.length()) throw std::invalid_argument("Invalid");
        } catch (...) {
            error_msg = "Loi: Gia phai la mot so thuc hop le!"; is_saved = false; return;
        }
        if (is_negative(price)) {
            error_msg = "Loi: Gia khong duoc nho hon 0!"; is_saved = false; return;
        }

        int quantity = 0;
        try {
            size_t pos;
            quantity = std::stoi(quantity_str, &pos);
            if (pos != quantity_str.length()) throw std::invalid_argument("Invalid");
        } catch (...) {
            error_msg = "Loi: So luong phai la mot so nguyen hop le!"; is_saved = false; return;
        }
        if (is_negative(quantity)) {
            error_msg = "Loi: So luong khong duoc nho hon 0!"; is_saved = false; return;
        }

        Comic new_comic;
        strncpy(new_comic.comic_name, name_str.c_str(), sizeof(new_comic.comic_name) - 1);
        new_comic.comic_name[sizeof(new_comic.comic_name) - 1] = '\0';
        strncpy(new_comic.author, author_str.c_str(), sizeof(new_comic.author) - 1);
        new_comic.author[sizeof(new_comic.author) - 1] = '\0';
        new_comic.price = price;
        new_comic.quantity = quantity;
        new_comic.is_deleted = false;

        add_comic(new_comic);
        is_saved = true;
        error_msg = "Them truyen thanh cong! Nhan Huy hoac ESC de thoat.";
      }, ButtonOption::Animated());

      auto cancel_button = Button("Huy & Tro ve", [&] {
        form_screen.ExitLoopClosure()();
      }, ButtonOption::Animated());

      auto container = Container::Vertical({
        input_name, input_author, input_price, input_quantity,
        Container::Horizontal({submit_button, cancel_button})
      });

      auto container_with_event = CatchEvent(container, [&](Event event) {
        if (event == Event::Escape) { form_screen.ExitLoopClosure()(); return true; }
        return false;
      });

      auto renderer = Renderer(container_with_event, [&] {
        return vbox({
            text("--- THEM TRUYEN MOI ---") | bold | center,
            separator(),
            hbox(text(" Ten truyen:  "), input_name->Render()),
            hbox(text(" Tac gia:     "), input_author->Render()),
            hbox(text(" Gia (VND):   "), input_price->Render()),
            hbox(text(" So luong:    "), input_quantity->Render()),
            separator(),
            text(error_msg) | color(is_saved ? Color::Green : Color::Red) | center,
            separator(),
            hbox(submit_button->Render(), text("   "), cancel_button->Render()) | center
        }) | border;
      });

      form_screen.Loop(renderer);

    } else if (selected == 2) {
      system("cls");
      int id = select_comic_ui("CHON TRUYEN DE SUA");
      if (id == -1) continue;

      system("cls");
      Comic comic_to_edit;
      if (get_comic_by_id(id, comic_to_edit)) {
        auto form_screen = ScreenInteractive::TerminalOutput();
        
        std::string name_str = comic_to_edit.comic_name;
        std::string author_str = comic_to_edit.author;
        std::string price_str = std::to_string((int)comic_to_edit.price); // Avoid huge precision
        std::string quantity_str = std::to_string(comic_to_edit.quantity);
        std::string orig_name = comic_to_edit.comic_name;
        std::string orig_author = comic_to_edit.author;
        std::string error_msg = "";
        bool is_saved = false;

        Component input_name = Input(&name_str, "Nhap ten truyen...");
        Component input_author = Input(&author_str, "Nhap tac gia...");
        Component input_price = Input(&price_str, "Nhap gia...");
        Component input_quantity = Input(&quantity_str, "Nhap so luong...");

        auto submit_button = Button("Cap nhat", [&] {
          if (is_empty_string(name_str)) {
              error_msg = "Loi: Ten khong duoc de trong!"; is_saved = false; return;
          }
          if (is_empty_string(author_str)) {
              error_msg = "Loi: Tac gia khong duoc de trong!"; is_saved = false; return;
          }
          if ((name_str != orig_name || author_str != orig_author) && is_comic_duplicate(name_str.c_str(), author_str.c_str())) {
              error_msg = "Loi: Truyen co ten va tac gia nay da ton tai!"; is_saved = false; return;
          }

          double price = 0.0;
          try {
              size_t pos;
              price = std::stod(price_str, &pos);
              if (pos != price_str.length()) throw std::invalid_argument("Invalid");
          } catch (...) {
              error_msg = "Loi: Gia phai la mot so thuc hop le!"; is_saved = false; return;
          }
          if (is_negative(price)) {
              error_msg = "Loi: Gia khong duoc nho hon 0!"; is_saved = false; return;
          }

          int quantity = 0;
          try {
              size_t pos;
              quantity = std::stoi(quantity_str, &pos);
              if (pos != quantity_str.length()) throw std::invalid_argument("Invalid");
          } catch (...) {
              error_msg = "Loi: So luong phai la mot so nguyen hop le!"; is_saved = false; return;
          }
          if (is_negative(quantity)) {
              error_msg = "Loi: So luong khong duoc nho hon 0!"; is_saved = false; return;
          }

          strncpy(comic_to_edit.comic_name, name_str.c_str(), sizeof(comic_to_edit.comic_name) - 1);
          comic_to_edit.comic_name[sizeof(comic_to_edit.comic_name) - 1] = '\0';
          strncpy(comic_to_edit.author, author_str.c_str(), sizeof(comic_to_edit.author) - 1);
          comic_to_edit.author[sizeof(comic_to_edit.author) - 1] = '\0';
          comic_to_edit.price = price;
          comic_to_edit.quantity = quantity;

          if (update_comic(comic_to_edit)) {
              is_saved = true;
              error_msg = "Cap nhat thanh cong! Nhan Huy hoac ESC de thoat.";
          } else {
              error_msg = "Loi he thong khi cap nhat!"; is_saved = false;
          }
        }, ButtonOption::Animated());

        auto cancel_button = Button("Huy & Tro ve", [&] {
          form_screen.ExitLoopClosure()();
        }, ButtonOption::Animated());

        auto container = Container::Vertical({
          input_name, input_author, input_price, input_quantity,
          Container::Horizontal({submit_button, cancel_button})
        });

        auto container_with_event = CatchEvent(container, [&](Event event) {
          if (event == Event::Escape) { form_screen.ExitLoopClosure()(); return true; }
          return false;
        });

        auto renderer = Renderer(container_with_event, [&] {
          return vbox({
              text("--- SUA THONG TIN TRUYEN ---") | bold | center,
              separator(),
              text(" Dang sua: " + orig_name) | center,
              separator(),
              hbox(text(" Ten moi:     "), input_name->Render()),
              hbox(text(" Tac gia:     "), input_author->Render()),
              hbox(text(" Gia (VND):   "), input_price->Render()),
              hbox(text(" So luong:    "), input_quantity->Render()),
              separator(),
              text(error_msg) | color(is_saved ? Color::Green : Color::Red) | center,
              separator(),
              hbox(submit_button->Render(), text("   "), cancel_button->Render()) | center
          }) | border;
        });

        form_screen.Loop(renderer);
      } else {
        std::cout << "Khong tim thay truyen thuoc ID nay!\n";
        get_string_input("Nhan Enter de tiep tuc...");
      }
    } else if (selected == 3) {
      system("cls");
      int id = select_comic_ui("CHON TRUYEN DE XOA");
      if (id == -1) continue;

      system("cls");
      std::cout << "\n--- XOA TRUYEN ---\n";
      if (delete_comic(id)) {
        std::cout << "Xoa truyen thanh cong!\n";
      } else {
        std::cout << "Loi khi xoa truyen nguyen do file nhieu du lieu bi hong!\n";
      }
      get_string_input("Nhan Enter de tiep tuc...");
    } else if (selected == 4) {
      system("cls");
      std::vector<std::string> search_entries = {
          "1. Tim theo Ten (Tuyet doi, Binary Search)",
          "2. Tim theo Tieu de - Tu khoa (Tuong doi, Linear Search)",
          "3. Tim theo ID (Tuyet doi, Binary Search)",
          "4. Tro ve"
      };
      int search_selected = 0;
      MenuOption search_option;
      auto screen_search = ScreenInteractive::TerminalOutput();
      search_option.on_enter = screen_search.ExitLoopClosure();
      auto search_menu = Menu(&search_entries, &search_selected, search_option);
      auto search_menu_with_event = CatchEvent(search_menu, [&](Event event) {
          if (event == Event::Escape) { search_selected = 3; screen_search.ExitLoopClosure()(); return true; }
          if (event.is_character() && event.character()[0] >= '1' && event.character()[0] <= '4') {
              search_selected = event.character()[0] - '1'; screen_search.ExitLoopClosure()(); return true;
          }
          return false;
      });
      auto search_renderer = Renderer(search_menu_with_event, [&] {
        return window(text(" CHON KIEU TIM KIEM TRUYEN "), search_menu_with_event->Render() | vscroll_indicator | frame) | bold;
      });
      screen_search.Loop(search_renderer);

      if (search_selected != 3) {
          system("cls");
          std::cout << "\n--- TIM KIEM TRUYEN ---\n";
          
          if (search_selected == 1) { // Tu khoa
              std::string kw = get_string_input("Nhap tu khoa can tim: ");
              if (kw != "[ESC]") {
                  std::vector<Comic> res = search_comics_by_name(kw);
                  render_comic_table(res);
              }
          } else {
              std::vector<Comic> comics = read_all_comics();
              std::vector<Comic> active_comics;
              for (const auto& c : comics) {
                  if (!c.is_deleted) active_comics.push_back(c);
              }

              if (search_selected == 0) { // Exact Name
                  std::string kw = get_string_input("Nhap chinh xac ten truyen: ");
                  if (kw != "[ESC]") {
                      quick_sort(active_comics, compare_comic_by_name_asc);
                      int idx = binary_search_idx<Comic, std::string>(active_comics, kw, search_cmp_comic_name);
                      if (idx != -1) {
                          std::vector<Comic> res = { active_comics[idx] };
                          render_comic_table(res);
                      } else {
                          std::cout << "Khong tim thay truyen thuoc ten nay (Binary Search).\n";
                      }
                  }
              } else if (search_selected == 2) { // ID
                  int id_kw = get_int_input("Nhap ID can tim: ");
                  if (id_kw != -999999) {
                      quick_sort(active_comics, compare_comic_by_id_asc);
                      int idx = binary_search_idx<Comic, int>(active_comics, id_kw, search_cmp_comic_id);
                      if (idx != -1) {
                          std::vector<Comic> res = { active_comics[idx] };
                          render_comic_table(res);
                      } else {
                          std::cout << "Khong tim thay truyen thuoc ID nay (Binary Search).\n";
                      }
                  }
              }
          }
          get_string_input("Nhan Enter de tiep tuc...");
      }
    } else if (selected == 5) {
      system("cls");
      break;
    }
  }
}
