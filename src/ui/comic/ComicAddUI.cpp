#include "../../../include/ui/comic/ComicAddUI.h"
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

void handle_add_comic() {
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
      Component input_quantity = Input(&quantity_str, "Nhap tong so luong...");

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
        new_comic.total_quantity = quantity;
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
            hbox(text(" Tong so luong:"), input_quantity->Render()),
            separator(),
            text(error_msg) | color(is_saved ? Color::Green : Color::Red) | center,
            separator(),
            hbox(submit_button->Render(), text("   "), cancel_button->Render()) | center
        }) | border;
      });

      form_screen.Loop(renderer);

}
