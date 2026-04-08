#include "../../../include/ui/customer/CustomerDeleteUI.h"
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

void handle_delete_customer() {
      system("cls");
      std::cout << "\n--- XOA KHACH HANG ---\n";
      int id = select_customer_ui("CHON KHACH HANG DE XOA");
      if (id == -1) return;

      system("cls");
      if (delete_customer(id)) {
        std::cout << "Xoa khach hang thanh cong!\n";
      } else {
        std::cout << "Loi khi xoa khach!\n";
      }
      get_string_input("Nhan Enter de tiep tuc...");
}
