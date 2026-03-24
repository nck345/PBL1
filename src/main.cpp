#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

// Include UI headers
#include "../include/ui/ComicUI.h"

// Include FTXUI headers for the main menu
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

int main() {
  auto screen = ScreenInteractive::TerminalOutput();

  std::vector<std::string> entries = {
      "1. Quan ly Truyen", "2. Quan ly Phieu Thue",
      "3. Thong ke Phieu & Doanh thu", "4. Thoat chuong trinh"};
  int selected = 0;

  MenuOption option;
  option.on_enter = screen.ExitLoopClosure();

  auto menu = Menu(&entries, &selected, option);

  auto renderer = Renderer(menu, [&] {
    return window(text(" HE THONG QUAN LY THUE TRUYEN (PBL1) "),
                  menu->Render() | vscroll_indicator | frame) |
           bold;
  });

  while (true) {
    // Run the FTXUI loop
    screen.Loop(renderer);

    // Process the user's selection
    if (selected == 0) {
      system("cls");
      // 1. Quan ly Truyen
      render_comic_menu();
    } else if (selected == 1) {
      system("cls");
      // 2. Quan ly Phieu Thue
      std::cout << "\n[Tinh nang 'Quan ly Phieu Thue' dang duoc Nhu Y phat "
                   "trien...]\n";
      std::cout << "Nhan Enter de quay lai Menu Chinh...";
      std::cin.get();
    } else if (selected == 2) {
      system("cls");
      // 3. Thong ke Doanh thu & Ton kho
      std::cout << "\n[Tinh nang 'Thong ke' dang duoc Nhu Y phat trien...]\n";
      std::cout << "Nhan Enter de quay lai Menu Chinh...";
      std::cin.get();
    } else if (selected == 3) {
      system("cls");
      // 4. Thoat chuong trinh
      std::cout << "\nDA THOAT CHUONG TRINH. TAM BIET!\n";
      break;
    }
  }

  return 0;
}