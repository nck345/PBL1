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
#include <ftxui/screen/terminal.hpp>

using namespace ftxui;

Element build_comic_table_element(const std::vector<Comic>& comics);

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

  auto form_screen = ScreenInteractive::Fullscreen();
  
  std::string search_name = "";
  std::string search_type = "";
  
  std::vector<std::string> stock_options = {"Tat ca", "Con hang", "Het hang"};
  int selected_stock = 0;

  std::vector<std::string> sort_options = {"-", "Tang dan", "Giam dan"};
  std::vector<std::string> sort_options_az = {"-", "A -> Z", "Z -> A"};
  
  int sort_id_opt = 0;
  int sort_name_opt = 0;
  int sort_price_opt = 0;
  int sort_type_opt = 0;

  int prev_sort_id = 0;
  int prev_sort_name = 0;
  int prev_sort_price = 0;
  int prev_sort_type = 0;
  int last_clicked_sort = 0;

  auto check_toggle_changes = [&] {
     if (sort_id_opt != prev_sort_id) { last_clicked_sort = 1; prev_sort_id = sort_id_opt; }
     if (sort_name_opt != prev_sort_name) { last_clicked_sort = 2; prev_sort_name = sort_name_opt; }
     if (sort_price_opt != prev_sort_price) { last_clicked_sort = 3; prev_sort_price = sort_price_opt; }
     if (sort_type_opt != prev_sort_type) { last_clicked_sort = 4; prev_sort_type = sort_type_opt; }
  };

  std::vector<std::string> type_options = build_type_options(active_comics);
  std::vector<std::string> filtered_type_options;
  int selected_type_option = 0;

  std::vector<Comic> filtered_comics = active_comics;

  auto apply_filter_sort = [&] {
    filtered_comics.clear();
    std::string s_name = to_lower_text(trim(search_name));
    std::string s_type = "";
    
    if (!is_empty_string(search_type)) {
       s_type = to_lower_text(trim(search_type));
    }

    for (const auto& comic : active_comics) {
      bool name_ok = s_name.empty() || to_lower_text(comic.comic_name).find(s_name) != std::string::npos;
      bool type_ok = s_type.empty() || to_lower_text(comic.type).find(s_type) != std::string::npos;
      bool stock_ok = true;
      if (selected_stock == 1) stock_ok = comic.quantity > 0;
      if (selected_stock == 2) stock_ok = comic.quantity <= 0;

      if (name_ok && type_ok && stock_ok) {
        filtered_comics.push_back(comic);
      }
    }

    if (last_clicked_sort == 1) { 
      if (sort_id_opt == 1) quick_sort(filtered_comics, compare_comic_by_id_asc);
      else if (sort_id_opt == 2) quick_sort(filtered_comics, compare_comic_by_id_desc);
    } else if (last_clicked_sort == 2) { 
      if (sort_name_opt == 1) quick_sort(filtered_comics, compare_comic_by_name_asc);
      else if (sort_name_opt == 2) quick_sort(filtered_comics, compare_comic_by_name_desc);
    } else if (last_clicked_sort == 3) {
      if (sort_price_opt == 1) quick_sort(filtered_comics, compare_comic_by_price_asc);
      else if (sort_price_opt == 2) quick_sort(filtered_comics, compare_comic_by_price_desc);
    } else if (last_clicked_sort == 4) {
      if (sort_type_opt == 1) quick_sort(filtered_comics, compare_comic_by_type_asc);
      else if (sort_type_opt == 2) quick_sort(filtered_comics, compare_comic_by_type_desc);
    }
  };

  Component input_search_name = Input(&search_name, "Tim theo ten...");
  Component input_search_type = Input(&search_type, "Go de tim the loai...");
  Component stock_radiobox = Radiobox(&stock_options, &selected_stock);
  
  auto type_menu_raw = Menu(&filtered_type_options, &selected_type_option);
  auto type_menu_c = CatchEvent(type_menu_raw, [&](Event event) {
    if (event.is_mouse() && event.mouse().motion == Mouse::Moved) return true;
    if (event == Event::Return) {
       if (!filtered_type_options.empty() && is_valid_type_suggestion(filtered_type_options[selected_type_option])) {
           search_type = filtered_type_options[selected_type_option];
       }
       stock_radiobox->TakeFocus();
       return true; 
    }
    return false;
  });

  auto toggle_id_comp = Toggle(&sort_options, &sort_id_opt);
  auto toggle_name_comp = Toggle(&sort_options_az, &sort_name_opt);
  auto toggle_price_comp = Toggle(&sort_options, &sort_price_opt);
  auto toggle_type_comp = Toggle(&sort_options_az, &sort_type_opt);

  int selected_comic_index = 0;
  int final_chosen_id = -1;
  std::vector<std::string> dummy_entries;

  MenuOption comic_menu_opt;
  comic_menu_opt.on_enter = [&] {
      if (!filtered_comics.empty() && selected_comic_index >= 0 && selected_comic_index < (int)filtered_comics.size()) {
          final_chosen_id = filtered_comics[selected_comic_index].id;
          form_screen.ExitLoopClosure()();
      }
  };
  // Empty transform to save CPU, actual drawing is done by the Table inside the active Renderer
  comic_menu_opt.entries_option.transform = [](const EntryState& state) { return text(""); };
  Component comic_menu = Menu(&dummy_entries, &selected_comic_index, comic_menu_opt);

  Component exit_button = Button("Huy & Tro Ve (ESC)", [&] {
      final_chosen_id = -1;
      form_screen.ExitLoopClosure()(); 
  }, ButtonOption::Animated());

  auto left_controls = Container::Vertical({
      input_search_name,
      input_search_type,
      type_menu_c,
      stock_radiobox,
      toggle_id_comp,
      toggle_name_comp,
      toggle_price_comp,
      toggle_type_comp,
      exit_button
  });

  auto main_container = Container::Horizontal({
      left_controls,
      comic_menu
  });

  ftxui::Box table_box;

  auto controls_with_event = CatchEvent(main_container, [&](Event event) {
    if (event == Event::Escape) {
      final_chosen_id = -1;
      form_screen.ExitLoopClosure()();
      return true;
    }
    if (event.is_mouse() && table_box.Contain(event.mouse().x, event.mouse().y)) {
       int hovered_row = event.mouse().y - table_box.y_min - 3;
       int max_idx = (int)filtered_comics.size() - 1;
       if (hovered_row >= 0 && hovered_row <= max_idx) {
           if (event.mouse().button == Mouse::Left && event.mouse().motion == Mouse::Pressed) {
               selected_comic_index = hovered_row;
               final_chosen_id = filtered_comics[hovered_row].id;
               form_screen.ExitLoopClosure()();
               return true;
           } else if (event.mouse().motion == Mouse::Moved) {
               selected_comic_index = hovered_row;
               // We just update hovered index so it highlights following the mouse
           }
       }
    }
    return false;
  });

  auto renderer = Renderer(controls_with_event, [&] {
    refresh_type_suggestions(type_options, search_type, filtered_type_options, selected_type_option);
    check_toggle_changes();
    apply_filter_sort();

    auto filter_panel = vbox({
        text("--- BỘ LỌC TÌM KIẾM ---") | bold,
        separator(),
        hbox(text(" Tên truyện: "), input_search_name->Render()),
        hbox(text(" Thể loại: "), input_search_type->Render()),
        text(" Gợi ý (Enter de chon):"), type_menu_c->Render() | size(HEIGHT, EQUAL, 5) | frame | border, 
        separator(),
        text(" Tồn kho:") | bold,
        stock_radiobox->Render(),
        separator(),
        text("--- SẮP XẾP ---") | bold,
        hbox(text(" ID:       "), toggle_id_comp->Render()),
        hbox(text(" Tên:      "), toggle_name_comp->Render()),
        hbox(text(" Giá:      "), toggle_price_comp->Render()),
        hbox(text(" Thể loại: "), toggle_type_comp->Render()),
        separator(),
        text(" Dùng Phím -> để Chuyển sang chọn") | color(Color::Green) | bold,
        exit_button->Render() | center,
    }) | border | size(WIDTH, GREATER_THAN, 40);

    // Dummy entries resize must happen in render exactly before rendering
    dummy_entries.resize(filtered_comics.size(), "");
    if (selected_comic_index >= (int)filtered_comics.size()) selected_comic_index = std::max(0, (int)filtered_comics.size() - 1);

    Element table_element;
    if (filtered_comics.empty()) {
        table_element = text("Khong co du lieu phu hop.") | center;
    } else {
        std::vector<std::vector<std::string>> table_data;
        table_data.push_back({"ID", "Ten Truyen", "Tac Gia", "The Loai", "Gia", "Ton/Tong"});
        for (const auto& c : filtered_comics) {
            table_data.push_back({
                std::to_string(c.id), truncate_text(c.comic_name, 25),
                truncate_text(c.author, 15), truncate_text(c.type, 15),
                format_currency(c.price), std::to_string(c.quantity) + "/" + std::to_string(c.total_quantity)
            });
        }
        
        auto table = Table(table_data);
        table.SelectAll().Border(LIGHT);
        table.SelectRow(0).Decorate(bold);
        table.SelectAll().SeparatorVertical(LIGHT);
        table.SelectRow(0).Border(DOUBLE);
        
        int row_index = selected_comic_index + 1;
        if (comic_menu->Focused()) {
            table.SelectRow(row_index).Decorate(inverted);
        } else {
            table.SelectRow(row_index).Decorate(bold);
        }
        table_element = table.Render() | reflect(table_box);
    }

    auto table_panel = window(
        text(" DANH SÁCH (" + std::to_string(filtered_comics.size()) + ") - BẤM ENTER ĐỂ CHỌN ") | bold | center,
        table_element | vscroll_indicator | frame | flex
    ) | flex;

    return window(text(" " + title + " ") | bold | center, hbox({ filter_panel, table_panel }));
  });

  form_screen.Loop(renderer);
  system("cls");

  return final_chosen_id;
}

void render_comic_table(const std::vector<Comic> &comics) {
  if (comics.empty()) {
    std::cout << "Khong co du lieu truyen.\n";
    return;
  }

  std::vector<std::vector<std::string>> table_data;
  table_data.push_back({"ID", "Ten Truyen", "Tac Gia", "The Loai", "Gia", "Ton/Tong"});

  bool has_data = false;
  for (const auto &comic : comics) {
    if (!comic.is_deleted) {
      has_data = true;
      table_data.push_back({std::to_string(comic.id), truncate_text(comic.comic_name, 25),
                            truncate_text(comic.author, 15), truncate_text(comic.type, 15), format_currency(comic.price),
                            std::to_string(comic.quantity) + "/" + std::to_string(comic.total_quantity)});
    }
  }

  if (!has_data) {
    std::cout << "Khong co du lieu truyen (hoac da bi xoa).\n";
    return;
  }

  auto table = Table(table_data);
  table.SelectAll().Border(LIGHT);

  table.SelectRow(0).Decorate(bold);
  table.SelectAll().SeparatorVertical(LIGHT);
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

Element build_comic_table_element(const std::vector<Comic>& comics) {
  if (comics.empty()) {
    return text("Khong co du lieu truyen.") | center;
  }

  std::vector<std::vector<std::string>> table_data;
  table_data.push_back({"ID", "Ten Truyen", "Tac Gia", "The Loai", "Gia", "Ton/Tong"});

  for (const auto &comic : comics) {
    if (!comic.is_deleted) {
      table_data.push_back({std::to_string(comic.id), truncate_text(comic.comic_name, 25),
                            truncate_text(comic.author, 15), truncate_text(comic.type, 15), format_currency(comic.price),
                            std::to_string(comic.quantity) + "/" + std::to_string(comic.total_quantity)});
    }
  }

  auto table = Table(table_data);
  table.SelectAll().Border(LIGHT);
  table.SelectRow(0).Decorate(bold);
  table.SelectAll().SeparatorVertical(LIGHT);
  table.SelectRow(0).Border(DOUBLE);

  return table.Render();
}


#include "../../include/ui/comic/ComicViewUI.h"
#include "../../include/ui/comic/ComicAddUI.h"
#include "../../include/ui/comic/ComicEditUI.h"
#include "../../include/ui/comic/ComicDeleteUI.h"

void render_comic_menu() {
  auto screen = ScreenInteractive::TerminalOutput();

  std::vector<std::string> entries = {
      "1. Xem danh sach truyen", "2. Them truyen moi",
      "3. Sua thong tin truyen", "4. Xoa truyen",
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
      if (event.is_mouse() && event.mouse().button == ftxui::Mouse::Left && event.mouse().motion == ftxui::Mouse::Pressed) {
          if (menu->OnEvent(event)) {
              if (option.on_enter) {
                  option.on_enter();
              }
              return true;
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
      handle_view_comics();
    } else if (selected == 1) {
      handle_add_comic();
    } else if (selected == 2) {
      handle_edit_comic();
    } else if (selected == 3) {
      handle_delete_comic();
    } else if (selected == 4) {
      system("cls");
      break;
    }
  }
}
