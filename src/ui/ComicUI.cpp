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
  comic_menu_opt.entries_option.transform = [&](const EntryState& state) {
      if (state.index >= (int)filtered_comics.size()) return text("");
      auto& c = filtered_comics[state.index];
      
      int w_id = 5;
      int w_name = 30;
      int w_author = 20;
      int w_type = 15;
      int w_price = 15;
      int w_qty = 5;

      auto row = hbox({
          text(std::to_string(c.id)) | size(WIDTH, EQUAL, w_id), text(" \xe2\x94\x82 "),
          text(truncate_text(c.comic_name, w_name)) | size(WIDTH, EQUAL, w_name), text(" \xe2\x94\x82 "),
          text(truncate_text(c.author, w_author)) | size(WIDTH, EQUAL, w_author), text(" \xe2\x94\x82 "),
          text(truncate_text(c.type, w_type)) | size(WIDTH, EQUAL, w_type), text(" \xe2\x94\x82 "),
          text(format_currency(c.price)) | size(WIDTH, EQUAL, w_price), text(" \xe2\x94\x82 "),
          text(std::to_string(c.quantity)) | size(WIDTH, EQUAL, w_qty),
          filler()
      });
      if (state.focused) { row = row | inverted; }
      if (state.active) { row = row | bold; }
      return row;
  };
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

  auto controls_with_event = CatchEvent(main_container, [&](Event event) {
    if (event == Event::Escape) {
      final_chosen_id = -1;
      form_screen.ExitLoopClosure()();
      return true;
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

    int w_id = 5;
    int w_name = 30;
    int w_author = 20;
    int w_type = 15;
    int w_price = 15;
    int w_qty = 5;

    auto header = hbox({
        text("ID") | size(WIDTH, EQUAL, w_id), text(" \xe2\x94\x82 "),
        text(truncate_text("Ten Truyen", w_name)) | size(WIDTH, EQUAL, w_name), text(" \xe2\x94\x82 "),
        text(truncate_text("Tac Gia", w_author)) | size(WIDTH, EQUAL, w_author), text(" \xe2\x94\x82 "),
        text(truncate_text("The Loai", w_type)) | size(WIDTH, EQUAL, w_type), text(" \xe2\x94\x82 "),
        text("Gia") | size(WIDTH, EQUAL, w_price), text(" \xe2\x94\x82 "),
        text("Ton") | size(WIDTH, EQUAL, w_qty),
        filler()
    }) | bold;

    auto table_panel = window(
        text(" DANH SÁCH (" + std::to_string(filtered_comics.size()) + ") - BẤM ENTER ĐỂ CHỌN ") | bold | center,
        vbox({
            header,
            separatorLight(),
            comic_menu->Render() | vscroll_indicator | frame | flex
        }) | border
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
  table_data.push_back({"ID", "Ten Truyen", "Tac Gia", "The Loai", "Gia", "So Luong"});

  bool has_data = false;
  for (const auto &comic : comics) {
    if (!comic.is_deleted) {
      has_data = true;
      table_data.push_back({std::to_string(comic.id), truncate_text(comic.comic_name, 25),
                            truncate_text(comic.author, 15), truncate_text(comic.type, 15), format_currency(comic.price),
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
  table_data.push_back({"ID", "Ten Truyen", "Tac Gia", "The Loai", "Gia", "Ton"});

  for (const auto &comic : comics) {
    if (!comic.is_deleted) {
      table_data.push_back({std::to_string(comic.id), truncate_text(comic.comic_name, 25),
                            truncate_text(comic.author, 15), truncate_text(comic.type, 15), format_currency(comic.price),
                            std::to_string(comic.quantity)});
    }
  }

  auto table = Table(table_data);
  table.SelectAll().Border(LIGHT);
  table.SelectRow(0).Decorate(bold);
  table.SelectAll().SeparatorVertical(LIGHT);
  table.SelectRow(0).Border(DOUBLE);

  return table.Render();
}

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
      int last_clicked_sort = 0; // 0: None, 1: ID, 2: Name, 3: Price, 4: Type

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

        if (last_clicked_sort == 1) { // ID
          if (sort_id_opt == 1) quick_sort(filtered_comics, compare_comic_by_id_asc);
          else if (sort_id_opt == 2) quick_sort(filtered_comics, compare_comic_by_id_desc);
        } else if (last_clicked_sort == 2) { // Name
          if (sort_name_opt == 1) quick_sort(filtered_comics, compare_comic_by_name_asc);
          else if (sort_name_opt == 2) quick_sort(filtered_comics, compare_comic_by_name_desc);
        } else if (last_clicked_sort == 3) { // Price
          if (sort_price_opt == 1) quick_sort(filtered_comics, compare_comic_by_price_asc);
          else if (sort_price_opt == 2) quick_sort(filtered_comics, compare_comic_by_price_desc);
        } else if (last_clicked_sort == 4) { // Type
          if (sort_type_opt == 1) quick_sort(filtered_comics, compare_comic_by_type_asc);
          else if (sort_type_opt == 2) quick_sort(filtered_comics, compare_comic_by_type_desc);
        }
      };

      Component input_search_name = Input(&search_name, "Tim theo ten...");
      Component input_search_type = Input(&search_type, "Go de tim the loai...");
      
      Component stock_radiobox = Radiobox(&stock_options, &selected_stock);
      
      auto type_menu_raw = Menu(&filtered_type_options, &selected_type_option);
      auto type_menu = CatchEvent(type_menu_raw, [&](Event event) {
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

      Component exit_button = Button("Tro Ve", [&] { form_screen.ExitLoopClosure()(); }, ButtonOption::Animated());

      auto controls = Container::Vertical({
          input_search_name,
          input_search_type,
          type_menu,
          stock_radiobox,
          toggle_id_comp,
          toggle_name_comp,
          toggle_price_comp,
          toggle_type_comp,
          exit_button
      });

      auto controls_with_event = CatchEvent(controls, [&](Event event) {
        if (event == Event::Escape) {
          form_screen.ExitLoopClosure()();
          return true;
        }
        if (event == Event::Return) {
          if (input_search_name->Focused()) {
            input_search_type->TakeFocus();
            return true;
          }
          if (input_search_type->Focused()) {
            type_menu->TakeFocus();
            return true;
          }
          if (stock_radiobox->Focused()) {
            toggle_id_comp->TakeFocus();
            return true;
          }
          if (toggle_id_comp->Focused()) {
            toggle_name_comp->TakeFocus();
            return true;
          }
          if (toggle_name_comp->Focused()) {
            toggle_price_comp->TakeFocus();
            return true;
          }
          if (toggle_price_comp->Focused()) {
            toggle_type_comp->TakeFocus();
            return true;
          }
          if (toggle_type_comp->Focused()) {
            exit_button->TakeFocus();
            return true;
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
            text(" Gợi ý (Enter de chon):"), type_menu->Render() | size(HEIGHT, EQUAL, 5) | frame | border, 
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
            exit_button->Render() | center,
        }) | border | size(WIDTH, GREATER_THAN, 45);

        auto table_panel = window(
            text(" DANH SÁCH TRUYỆN (" + std::to_string(filtered_comics.size()) + ")") | bold | center,
            build_comic_table_element(filtered_comics) | vscroll_indicator | frame
        ) | flex;

        return hbox({
            filter_panel,
            table_panel
        });
      });

      form_screen.Loop(renderer);
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
      break;
    }
  }
}
