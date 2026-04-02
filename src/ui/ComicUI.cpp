#include "../../include/ui/ComicUI.h"
#include "../../include/repository/ComicRepo.h"
#include "../../include/utils/InputHandler.h"
#include "../../include/utils/ValidationUtils.h"
#include "../../include/utils/SortUtils.h"
#include "../../include/utils/SearchUtils.h"
#include <algorithm>
#include <cctype>
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

std::string to_lower_text(const std::string& text) {
  std::string lowered = text;
  std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                 [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
  return lowered;
}

void copy_text_to_buffer(char* target, size_t target_size, const std::string& source) {
  if (target_size == 0) {
    return;
  }
  std::strncpy(target, source.c_str(), target_size - 1);
  target[target_size - 1] = '\0';
}

std::vector<Comic> get_active_comics() {
  std::vector<Comic> all_comics = read_all_comics();
  std::vector<Comic> active_comics;
  for (const auto& comic : all_comics) {
    if (!comic.is_deleted) {
      active_comics.push_back(comic);
    }
  }
  return active_comics;
}

std::vector<std::string> get_unique_comic_types(const std::vector<Comic>& comics) {
  std::vector<std::string> unique_types;
  for (const auto& comic : comics) {
    std::string comic_type = comic.type;
    if (is_empty_string(comic_type)) {
      continue;
    }

    std::string lowered = to_lower_text(comic_type);
    bool existed = false;
    for (const auto& current : unique_types) {
      if (to_lower_text(current) == lowered) {
        existed = true;
        break;
      }
    }
    if (!existed) {
      unique_types.push_back(comic_type);
    }
  }
  std::sort(unique_types.begin(), unique_types.end(), [](const std::string& a, const std::string& b) {
    return to_lower_text(a) < to_lower_text(b);
  });
  return unique_types;
}

std::vector<std::string> get_predefined_comic_types() {
  return {
      "Action",       "Adventure",   "Comedy",      "Drama",
      "Fantasy",      "Horror",      "Mystery",     "Romance",
      "Sci-Fi",       "Slice of Life", "Sports",    "Supernatural",
      "Historical",   "School Life", "Shounen",     "Seinen",
      "Shoujo",       "Josei",
  };
}

std::vector<std::string> build_type_options(const std::vector<Comic>& active_comics) {
  std::vector<std::string> options = get_predefined_comic_types();
  std::vector<std::string> dynamic_types = get_unique_comic_types(active_comics);

  for (const auto& type_value : dynamic_types) {
    bool existed = false;
    std::string lowered = to_lower_text(type_value);
    for (const auto& option : options) {
      if (to_lower_text(option) == lowered) {
        existed = true;
        break;
      }
    }
    if (!existed) {
      options.push_back(type_value);
    }
  }

  std::sort(options.begin(), options.end(), [](const std::string& a, const std::string& b) {
    return to_lower_text(a) < to_lower_text(b);
  });
  return options;
}

void refresh_type_suggestions(const std::vector<std::string>& type_options,
                              const std::string& query,
                              std::vector<std::string>& filtered_options,
                              int& selected_index) {
  filtered_options.clear();
  std::string keyword = to_lower_text(trim(query));

  for (const auto& type_value : type_options) {
    std::string lowered = to_lower_text(type_value);
    if (keyword.empty() || lowered.find(keyword) != std::string::npos) {
      filtered_options.push_back(type_value);
    }
  }

  if (filtered_options.empty()) {
    filtered_options.push_back("[Khong tim thay the loai]");
  }

  if (selected_index < 0 || selected_index >= static_cast<int>(filtered_options.size())) {
    selected_index = 0;
  }
}

bool is_valid_type_suggestion(const std::string& value) {
  return value != "[Khong tim thay the loai]";
}

int select_comic_ui(const std::string& title) {
  std::vector<Comic> active_comics = get_active_comics();

  if (active_comics.empty()) {
    std::cout << "Khong co truyen nao trong he thong!\n";
    return -1;
  }

  auto screen = ScreenInteractive::TerminalOutput();
  std::vector<std::string> entries;
  for (size_t i = 0; i < active_comics.size(); ++i) {
    std::string item = std::to_string(i + 1) + ". " + std::string(active_comics[i].comic_name) +
                       " - " + active_comics[i].author + " | TL: " + active_comics[i].type +
                       " (SL: " + std::to_string(active_comics[i].quantity) + ")";
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

  if (static_cast<size_t>(selected) == entries.size() - 1) {
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
  table_data.push_back({"ID", "Ten Truyen", "Tac Gia", "The Loai", "Gia", "So Luong"});

  bool has_data = false;
  for (const auto &comic : comics) {
    if (!comic.is_deleted) {
      has_data = true;
      table_data.push_back({std::to_string(comic.id), comic.comic_name,
                            comic.author, comic.type, format_currency(comic.price),
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
      std::vector<Comic> active_comics = get_active_comics();
      if (active_comics.empty()) {
        std::cout << "Khong co truyen nao trong he thong!\n";
        get_string_input("Nhan Enter de tiep tuc...");
        continue;
      }

      auto screen_filter = ScreenInteractive::TerminalOutput();
      std::vector<std::string> type_options = {"Tat ca"};
      std::vector<std::string> unique_types = get_unique_comic_types(active_comics);
      type_options.insert(type_options.end(), unique_types.begin(), unique_types.end());
      int selected_type = 0;

      std::vector<std::string> stock_options = {
          "Tat ca", "Con hang", "Het hang"};
      int selected_stock = 0;

      std::vector<std::string> sort_entries = {
          "Nguon goc",
          "ID (Tang dan)",
          "ID (Giam dan)",
          "Ten (A-Z)",
          "Gia (Tang dan)",
          "Gia (Giam dan)",
          "The loai (A-Z)",
      };
      int selected_sort = 0;
      bool should_apply = false;

      Component type_dropdown = Dropdown(&type_options, &selected_type);
      Component stock_radiobox = Radiobox(&stock_options, &selected_stock);
      Component sort_radiobox = Radiobox(&sort_entries, &selected_sort);
      Component apply_button = Button("Ap dung", [&] {
        should_apply = true;
        screen_filter.ExitLoopClosure()();
      }, ButtonOption::Animated());
      Component back_button = Button("Tro ve", [&] {
        should_apply = false;
        screen_filter.ExitLoopClosure()();
      }, ButtonOption::Animated());

      auto controls = Container::Vertical({
          type_dropdown,
          stock_radiobox,
          sort_radiobox,
          Container::Horizontal({apply_button, back_button}),
      });

      auto controls_with_event = CatchEvent(controls, [&](Event event) {
        if (event == Event::Escape) {
          should_apply = false;
          screen_filter.ExitLoopClosure()();
          return true;
        }
        return false;
      });

      auto filter_renderer = Renderer(controls_with_event, [&] {
        return vbox({
                   text("--- LOC & SAP XEP DANH SACH TRUYEN ---") | bold | center,
                   separator(),
                   text("The loai:") | bold,
                   type_dropdown->Render(),
                   separator(),
                   text("Trang thai ton kho:") | bold,
                   stock_radiobox->Render(),
                   separator(),
                   text("Tieu chi sap xep:") | bold,
                   sort_radiobox->Render(),
                   separator(),
                   hbox({apply_button->Render(), text("   "), back_button->Render()}) | center,
               }) |
               border;
      });

      screen_filter.Loop(filter_renderer);

      if (!should_apply) {
        continue;
      }

      std::vector<Comic> filtered_comics;
      std::string selected_type_value =
          selected_type > 0 ? type_options[selected_type] : "Tat ca";
      std::string selected_type_value_lower = to_lower_text(selected_type_value);

      for (const auto& comic : active_comics) {
        bool type_ok = true;
        bool stock_ok = true;

        if (selected_type > 0) {
          type_ok = to_lower_text(comic.type) == selected_type_value_lower;
        }

        if (selected_stock == 1) {
          stock_ok = comic.quantity > 0;
        } else if (selected_stock == 2) {
          stock_ok = comic.quantity <= 0;
        }

        if (type_ok && stock_ok) {
          filtered_comics.push_back(comic);
        }
      }

      if (selected_sort == 1) {
        quick_sort(filtered_comics, compare_comic_by_id_asc);
      } else if (selected_sort == 2) {
        quick_sort(filtered_comics, compare_comic_by_id_desc);
      } else if (selected_sort == 3) {
        quick_sort(filtered_comics, compare_comic_by_name_asc);
      } else if (selected_sort == 4) {
        quick_sort(filtered_comics, compare_comic_by_price_asc);
      } else if (selected_sort == 5) {
        quick_sort(filtered_comics, compare_comic_by_price_desc);
      } else if (selected_sort == 6) {
        quick_sort(filtered_comics, compare_comic_by_type_asc);
      }

      system("cls");
      std::cout << "\n--- DANH SACH TRUYEN ---\n";
      render_comic_table(filtered_comics);
      get_string_input("Nhan Enter de tiep tuc...");
    } else if (selected == 1) {
      system("cls");
      auto form_screen = ScreenInteractive::TerminalOutput();
      
      std::string name_str = "";
      std::string author_str = "";
      std::string type_str = "";
      std::string type_query_str = "";
      std::string price_str = "";
      std::string quantity_str = "";
      std::string error_msg = "";
      bool is_saved = false;

      std::vector<std::string> type_options = build_type_options(get_active_comics());
      std::vector<std::string> filtered_type_options;
      int selected_type_option = 0;
      refresh_type_suggestions(type_options, type_query_str, filtered_type_options, selected_type_option);

      Component input_name = Input(&name_str, "Nhap ten truyen...");
      Component input_author = Input(&author_str, "Nhap tac gia...");
      Component input_type_query = Input(&type_query_str, "Go de tim the loai...");
      Component type_menu_raw = Menu(&filtered_type_options, &selected_type_option);
      Component type_menu = CatchEvent(type_menu_raw, [&](Event event) {
        if (event.is_mouse() && event.mouse().motion == Mouse::Moved) {
          return true;
        }
        return false;
      });
      Component input_price = Input(&price_str, "Nhap gia...");
      Component input_quantity = Input(&quantity_str, "Nhap so luong...");

      auto submit_action = [&] {
        if (is_empty_string(name_str)) {
            error_msg = "Loi: Ten khong duoc de trong!"; is_saved = false; return;
        }
        if (is_empty_string(author_str)) {
            error_msg = "Loi: Tac gia khong duoc de trong!"; is_saved = false; return;
        }
        if (is_empty_string(type_str) && !filtered_type_options.empty() &&
            is_valid_type_suggestion(filtered_type_options[selected_type_option])) {
            type_str = filtered_type_options[selected_type_option];
        }
        if (is_empty_string(type_str) && !is_empty_string(type_query_str)) {
            type_str = trim(type_query_str);
        }
        if (is_empty_string(type_str)) {
            error_msg = "Loi: The loai khong duoc de trong!"; is_saved = false; return;
        }
        if (is_comic_duplicate(name_str.c_str(), author_str.c_str(), type_str.c_str())) {
            error_msg = "Loi: Truyen co Ten + Tac gia + The loai nay da ton tai!"; is_saved = false; return;
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
        copy_text_to_buffer(new_comic.comic_name, sizeof(new_comic.comic_name), name_str);
        copy_text_to_buffer(new_comic.author, sizeof(new_comic.author), author_str);
        copy_text_to_buffer(new_comic.type, sizeof(new_comic.type), type_str);
        new_comic.price = price;
        new_comic.quantity = quantity;
        new_comic.is_deleted = false;

        add_comic(new_comic);
        is_saved = true;
        error_msg = "Them truyen thanh cong! Nhan Huy hoac ESC de thoat.";
      };

      auto submit_button = Button("Xac nhan & Luu", submit_action, ButtonOption::Animated());

      auto cancel_button = Button("Huy & Tro ve", [&] {
        form_screen.ExitLoopClosure()();
      }, ButtonOption::Animated());

      auto container = Container::Vertical({
        input_name, input_author, input_type_query, type_menu, input_price, input_quantity,
        Container::Horizontal({submit_button, cancel_button})
      });

      auto container_with_event = CatchEvent(container, [&](Event event) {
        if (event == Event::Escape) { form_screen.ExitLoopClosure()(); return true; }
        if (event == Event::Return) {
          if (input_name->Focused()) {
            input_author->TakeFocus();
            return true;
          }
          if (input_author->Focused()) {
            input_type_query->TakeFocus();
            return true;
          }
          if (input_type_query->Focused()) {
            type_menu->TakeFocus();
            return true;
          }
          if (type_menu->Focused()) {
            if (!filtered_type_options.empty() &&
                is_valid_type_suggestion(filtered_type_options[selected_type_option])) {
              type_str = filtered_type_options[selected_type_option];
              error_msg = "Da chon the loai: " + type_str;
              input_price->TakeFocus();
            } else {
              error_msg = "Khong co the loai phu hop, vui long doi tu khoa tim kiem.";
            }
            is_saved = false;
            return true;
          }
          if (input_price->Focused()) {
            input_quantity->TakeFocus();
            return true;
          }
          if (input_quantity->Focused()) {
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
        refresh_type_suggestions(type_options, type_query_str, filtered_type_options, selected_type_option);
        return vbox({
            text("--- THEM TRUYEN MOI ---") | bold | center,
            separator(),
            hbox(text(" Ten truyen:  "), input_name->Render()),
            hbox(text(" Tac gia:     "), input_author->Render()),
            hbox(text(" Tim the loai:"), input_type_query->Render()),
            text(" Goi y the loai (Enter de chon):"),
            type_menu->Render() | frame,
            text(" The loai da chon: " + (is_empty_string(type_str) ? std::string("[chua chon]") : type_str)),
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
        std::string type_str = comic_to_edit.type;
        std::string type_query_str = comic_to_edit.type;
        std::string price_str = std::to_string((int)comic_to_edit.price); // Avoid huge precision
        std::string quantity_str = std::to_string(comic_to_edit.quantity);
        std::string orig_name = comic_to_edit.comic_name;
        std::string orig_author = comic_to_edit.author;
        std::string orig_type = comic_to_edit.type;
        std::string error_msg = "";
        bool is_saved = false;

        std::vector<std::string> type_options = build_type_options(get_active_comics());
        std::vector<std::string> filtered_type_options;
        int selected_type_option = 0;
        refresh_type_suggestions(type_options, type_query_str, filtered_type_options, selected_type_option);
        for (int i = 0; i < static_cast<int>(filtered_type_options.size()); ++i) {
          if (to_lower_text(filtered_type_options[i]) == to_lower_text(type_str)) {
            selected_type_option = i;
            break;
          }
        }

        Component input_name = Input(&name_str, "Nhap ten truyen...");
        Component input_author = Input(&author_str, "Nhap tac gia...");
        Component input_type_query = Input(&type_query_str, "Go de tim the loai...");
        Component type_menu_raw = Menu(&filtered_type_options, &selected_type_option);
        Component type_menu = CatchEvent(type_menu_raw, [&](Event event) {
          if (event.is_mouse() && event.mouse().motion == Mouse::Moved) {
            return true;
          }
          return false;
        });
        Component input_price = Input(&price_str, "Nhap gia...");
        Component input_quantity = Input(&quantity_str, "Nhap so luong...");

        auto submit_action = [&] {
          if (is_empty_string(name_str)) {
              error_msg = "Loi: Ten khong duoc de trong!"; is_saved = false; return;
          }
          if (is_empty_string(author_str)) {
              error_msg = "Loi: Tac gia khong duoc de trong!"; is_saved = false; return;
          }
          if (is_empty_string(type_str) && !filtered_type_options.empty() &&
              is_valid_type_suggestion(filtered_type_options[selected_type_option])) {
              type_str = filtered_type_options[selected_type_option];
          }
          if (is_empty_string(type_str) && !is_empty_string(type_query_str)) {
              type_str = trim(type_query_str);
          }
          if (is_empty_string(type_str)) {
              error_msg = "Loi: The loai khong duoc de trong!"; is_saved = false; return;
          }
          if ((name_str != orig_name || author_str != orig_author || type_str != orig_type) &&
              is_comic_duplicate(name_str.c_str(), author_str.c_str(), type_str.c_str())) {
              error_msg = "Loi: Truyen co Ten + Tac gia + The loai nay da ton tai!"; is_saved = false; return;
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

          copy_text_to_buffer(comic_to_edit.comic_name, sizeof(comic_to_edit.comic_name), name_str);
          copy_text_to_buffer(comic_to_edit.author, sizeof(comic_to_edit.author), author_str);
          copy_text_to_buffer(comic_to_edit.type, sizeof(comic_to_edit.type), type_str);
          comic_to_edit.price = price;
          comic_to_edit.quantity = quantity;

          if (update_comic(comic_to_edit)) {
              is_saved = true;
              error_msg = "Cap nhat thanh cong! Nhan Huy hoac ESC de thoat.";
          } else {
              error_msg = "Loi he thong khi cap nhat!"; is_saved = false;
          }
        };

        auto submit_button = Button("Cap nhat", submit_action, ButtonOption::Animated());

        auto cancel_button = Button("Huy & Tro ve", [&] {
          form_screen.ExitLoopClosure()();
        }, ButtonOption::Animated());

        auto container = Container::Vertical({
          input_name, input_author, input_type_query, type_menu, input_price, input_quantity,
          Container::Horizontal({submit_button, cancel_button})
        });

        auto container_with_event = CatchEvent(container, [&](Event event) {
          if (event == Event::Escape) { form_screen.ExitLoopClosure()(); return true; }
          if (event == Event::Return) {
            if (input_name->Focused()) {
              input_author->TakeFocus();
              return true;
            }
            if (input_author->Focused()) {
              input_type_query->TakeFocus();
              return true;
            }
            if (input_type_query->Focused()) {
              type_menu->TakeFocus();
              return true;
            }
            if (type_menu->Focused()) {
              if (!filtered_type_options.empty() &&
                  is_valid_type_suggestion(filtered_type_options[selected_type_option])) {
                type_str = filtered_type_options[selected_type_option];
                error_msg = "Da chon the loai: " + type_str;
                input_price->TakeFocus();
              } else {
                error_msg = "Khong co the loai phu hop, vui long doi tu khoa tim kiem.";
              }
              is_saved = false;
              return true;
            }
            if (input_price->Focused()) {
              input_quantity->TakeFocus();
              return true;
            }
            if (input_quantity->Focused()) {
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
          refresh_type_suggestions(type_options, type_query_str, filtered_type_options, selected_type_option);
          return vbox({
              text("--- SUA THONG TIN TRUYEN ---") | bold | center,
              separator(),
              text(" Dang sua: " + orig_name) | center,
              separator(),
              hbox(text(" Ten moi:     "), input_name->Render()),
              hbox(text(" Tac gia:     "), input_author->Render()),
              hbox(text(" Tim the loai:"), input_type_query->Render()),
              text(" Goi y the loai (Enter de chon):"),
              type_menu->Render() | frame,
              text(" The loai da chon: " + (is_empty_string(type_str) ? std::string("[chua chon]") : type_str)),
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
          "4. Tim theo The loai - Tu khoa (Linear Search)",
          "5. Tro ve"
      };
      int search_selected = 0;
      MenuOption search_option;
      auto screen_search = ScreenInteractive::TerminalOutput();
      search_option.on_enter = screen_search.ExitLoopClosure();
      auto search_menu = Menu(&search_entries, &search_selected, search_option);
      auto search_menu_with_event = CatchEvent(search_menu, [&](Event event) {
          if (event == Event::Escape) { search_selected = 4; screen_search.ExitLoopClosure()(); return true; }
          if (event.is_character() && event.character()[0] >= '1' && event.character()[0] <= '5') {
              search_selected = event.character()[0] - '1'; screen_search.ExitLoopClosure()(); return true;
          }
          return false;
      });
      auto search_renderer = Renderer(search_menu_with_event, [&] {
        return window(text(" CHON KIEU TIM KIEM TRUYEN "), search_menu_with_event->Render() | vscroll_indicator | frame) | bold;
      });
      screen_search.Loop(search_renderer);

      if (search_selected != 4) {
          system("cls");
          std::cout << "\n--- TIM KIEM TRUYEN ---\n";
          
          if (search_selected == 1) { // Tu khoa
              std::string kw = get_string_input("Nhap tu khoa can tim: ");
              if (kw != "[ESC]") {
                  std::vector<Comic> res = search_comics_by_name(kw);
                  render_comic_table(res);
              }
          } else if (search_selected == 3) {
              std::string kw = get_string_input("Nhap the loai can tim: ");
              if (kw != "[ESC]") {
                  std::vector<Comic> res = search_comics_by_type(kw);
                  render_comic_table(res);
              }
          } else {
              std::vector<Comic> active_comics = get_active_comics();

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
