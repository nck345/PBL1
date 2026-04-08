#include "../../../include/ui/comic/ComicDeleteUI.h"
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

void handle_delete_comic() {
      system("cls");
      int id = select_comic_ui("CHON TRUYEN DE XOA");
      if (id == -1) return;

      system("cls");
      std::cout << "\n--- XOA TRUYEN ---\n";
      if (delete_comic(id)) {
        std::cout << "Xoa truyen thanh cong!\n";
      } else {
        std::cout << "Loi khi xoa truyen nguyen do file nhieu du lieu bi hong!\n";
      }
      get_string_input("Nhan Enter de tiep tuc...");
}
