#include "../../../include/ui/customer/CustomerEditUI.h"
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

void handle_edit_customer() {
      system("cls");
      int id = select_customer_ui("CHON KHACH HANG DE SUA");
      if (id == -1) return;

      system("cls");
      Customer c_to_edit;
      if (get_customer_by_id(id, c_to_edit)) {
        auto form_screen = ScreenInteractive::TerminalOutput();
        
        std::string name_str = c_to_edit.name;
        std::string phone_str = c_to_edit.phone;
        std::string orig_phone = c_to_edit.phone;
        std::string error_msg = "";
        bool is_saved = false;

        Component input_name = Input(&name_str, "Nhap ten moi...");
        Component input_phone = Input(&phone_str, "Nhap so dien thoai moi...");

        auto submit_action = [&] {
          if (is_empty_string(name_str)) {
            error_msg = "Loi: Ten khong duoc de trong!";
            is_saved = false;
            return;
          }
          if (is_empty_string(phone_str)) {
            error_msg = "Loi: SDT khong duoc de trong!";
            is_saved = false;
            return;
          }
          if (!is_valid_phone_number(phone_str)) {
            error_msg = "Loi: SDT khong hop le (10 so, bat dau tu 0)!";
            is_saved = false;
            return;
          }
          if (phone_str != orig_phone && is_customer_duplicate(phone_str)) {
            error_msg = "Loi: So dien thoai da bi trung voi khach khac!";
            is_saved = false;
            return;
          }

          strncpy(c_to_edit.name, name_str.c_str(), sizeof(c_to_edit.name) - 1);
          c_to_edit.name[sizeof(c_to_edit.name) - 1] = '\0';
          strncpy(c_to_edit.phone, phone_str.c_str(), sizeof(c_to_edit.phone) - 1);
          c_to_edit.phone[sizeof(c_to_edit.phone) - 1] = '\0';

          if (update_customer(c_to_edit)) {
            is_saved = true;
            error_msg = "Cap nhat thanh cong! Nhan Huy hoac ESC de thoat.";
          } else {
            error_msg = "Loi he thong khi cap nhat!";
            is_saved = false;
          }
        };

        auto submit_button = Button("Cap nhat", submit_action, ButtonOption::Animated());

        auto cancel_button = Button("Huy & Tro ve", [&] {
          form_screen.ExitLoopClosure()();
        }, ButtonOption::Animated());

        auto container = Container::Vertical({
          input_name,
          input_phone,
          Container::Horizontal({submit_button, cancel_button})
        });

        auto container_with_event = CatchEvent(container, [&](Event event) {
          if (event == Event::Escape) {
            form_screen.ExitLoopClosure()();
            return true;
          }
          if (event == Event::Return) {
            if (input_name->Focused()) {
              input_phone->TakeFocus();
              return true;
            }
            if (input_phone->Focused()) {
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
          return vbox({
              text("--- SUA THONG TIN KHACH HANG ---") | bold | center,
              separator(),
              text(" Dang sua: " + std::string(c_to_edit.name) + " (" + orig_phone + ")") | center,
              separator(),
              hbox(text(" Ten moi:     "), input_name->Render()),
              hbox(text(" SDT moi:     "), input_phone->Render()),
              separator(),
              text(error_msg) | color(is_saved ? Color::Green : Color::Red) | center,
              separator(),
              hbox(submit_button->Render(), text("   "), cancel_button->Render()) | center
          }) | border;
        });

        form_screen.Loop(renderer);
      } else {
        std::cout << "Khong tim thay khach hang ID nay!\n";
        get_string_input("Nhan Enter de tiep tuc...");
      }
}
