#include "../../../include/ui/customer/CustomerAddUI.h"
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

void handle_add_customer() {
      system("cls");
      auto form_screen = ScreenInteractive::TerminalOutput();
      
      std::string name_str = "";
      std::string phone_str = "";
      std::string error_msg = "";
      bool is_saved = false;

      Component input_name = Input(&name_str, "Nhap ten khach hang...");
      Component input_phone = Input(&phone_str, "Nhap so dien thoai...");

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
        if (is_customer_duplicate(phone_str)) {
          error_msg = "Loi: So dien thoai da ton tai trong he thong!";
          is_saved = false;
          return;
        }

        Customer new_c;
        strncpy(new_c.name, name_str.c_str(), sizeof(new_c.name) - 1);
        new_c.name[sizeof(new_c.name) - 1] = '\0';
        strncpy(new_c.phone, phone_str.c_str(), sizeof(new_c.phone) - 1);
        new_c.phone[sizeof(new_c.phone) - 1] = '\0';
        new_c.is_deleted = false;

        add_customer(new_c);
        is_saved = true;
        error_msg = "Them khach hang thanh cong! Nhan Huy hoac ESC de thoat.";
      };

      auto submit_button = Button("Xac nhan & Luu", submit_action, ButtonOption::Animated());

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
            text("--- THEM KHACH HANG MOI ---") | bold | center,
            separator(),
            hbox(text(" Ten KH:      "), input_name->Render()),
            hbox(text(" So DT:       "), input_phone->Render()),
            separator(),
            text(error_msg) | color(is_saved ? Color::Green : Color::Red) | center,
            separator(),
            hbox(submit_button->Render(), text("   "), cancel_button->Render()) | center
        }) | border;
      });

      form_screen.Loop(renderer);

}
