#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

// Include UI headers
#include "../include/ui/ComicUI.h"
#include "../include/ui/RentalUI.h"
#include "../include/ui/CustomerUI.h"

// Include FTXUI headers for the main menu
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

int main() {
  auto screen = ScreenInteractive::TerminalOutput();

  std::vector<std::string> entries = {
      "1. Quan ly Truyen", "2. Quan ly Khach hang", "3. Quan ly Phieu Thue",
      "4. Thong ke Phieu & Doanh thu", "5. Thoat chuong trinh"};
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
    return window(text(" HE THONG QUAN LY THUE TRUYEN (PBL1) "),
                  menu_with_event->Render() | vscroll_indicator | frame) |
           bold;
  });

  while (true) {
    system("cls");
    // Run the FTXUI loop
    screen.Loop(renderer);

    // Process the user's selection
    if (selected == 0) {
      system("cls");
      // 1. Quan ly Truyen
      render_comic_menu();
    } else if (selected == 1) {
      system("cls");
      // 2. Quan ly Khach Hang
      render_customer_menu();
    } else if (selected == 2) {
      system("cls");
      // 3. Quan ly Phieu
      render_rental_menu();
    } else if (selected == 3) {
      system("cls");
      // 4. Thong ke
      render_statistics_screen();
    } else if (selected == 4) {
      system("cls");
      // 5. Thoat chuong trinh
      std::cout << "\nDA THOAT CHUONG TRINH. TAM BIET!\n";
      break;
    }
  }

  return 0;
}