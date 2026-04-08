#include "../../../include/ui/customer/CustomerViewUI.h"
#include "../../../include/ui/CustomerUI.h"
#include "../../../include/repository/CustomerRepo.h"
#include "../../../include/utils/InputHandler.h"
#include "../../../include/utils/ValidationUtils.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdlib>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/terminal.hpp>

using namespace ftxui;

void handle_view_customers() {
      system("cls");
      std::vector<Customer> all = read_all_customers();
      if (all.empty()) {
         std::cout << "Khong co khach hang nao trong he thong!\n";
         get_string_input("Nhan Enter de tiep tuc...");
         return;
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
}
