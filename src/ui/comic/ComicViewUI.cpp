#include "../../../include/ui/comic/ComicViewUI.h"
#include "../../../include/ui/ComicUI.h"
#include "../../../include/repository/ComicRepo.h"
#include "../../../include/utils/InputHandler.h"
#include "../../../include/utils/ValidationUtils.h"
#include "../../../include/utils/SortUtils.h"
#include "../../../include/utils/SearchUtils.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/terminal.hpp>

using namespace ftxui;

extern Element build_comic_table_element(const std::vector<Comic>& comics);

void handle_view_comics() {
      system("cls");
      std::vector<Comic> active_comics = get_active_comics();
      if (active_comics.empty()) {
        std::cout << "Khong co truyen nao trong he thong!\n";
        get_string_input("Nhan Enter de tiep tuc...");
        return;
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
}
