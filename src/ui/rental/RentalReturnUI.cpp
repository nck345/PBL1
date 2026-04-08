#include "../../../include/ui/rental/RentalReturnUI.h"
#include "../../../include/ui/RentalUI.h"
#include "../../../include/repository/RentalRepo.h"
#include "../../../include/repository/ComicRepo.h"
#include "../../../include/services/RentalService.h"
#include "../../../include/utils/InputHandler.h"
#include "../../../include/utils/ValidationUtils.h"
#include "../../../include/utils/SortUtils.h"
#include "../../../include/ui/ComicUI.h"
#include "../../../include/ui/CustomerUI.h"
#include "../../../include/repository/CustomerRepo.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/terminal.hpp>

using namespace ftxui;

void render_return_comic_screen() {
  system("cls");
  int p_id = select_rental_slip_ui("CHON PHIEU THUE DE TRA");
  if (p_id == -1) return;

  system("cls");
  auto screen = ScreenInteractive::TerminalOutput();

  std::string p_id_str = std::to_string(p_id);
  std::string error_msg = "";
  bool should_submit = false;
  
  std::vector<std::string> options_tt = {
     "1. Trả nguyên vẹn (Hoàn đủ cọc nếu đúng hạn)",
     "2. Rách, móp nhẹ (Khấu trừ 20% giá gốc)",
     "3. Bị khuyết/mất trang (Khấu trừ 50% giá gốc)",
     "4. Mất/Hỏng sách (Mất 100% tiền cọc)"
  };
  int selected_tt = 0;
  Component radio_tt = Radiobox(&options_tt, &selected_tt);

  auto submit_button = Button("Xác nhận & Thanh toán", [&] {
      error_msg = "";
      should_submit = true;
      screen.ExitLoopClosure()();
  }, ButtonOption::Animated());

  auto cancel_button = Button("Hủy & Trở về", [&] {
      should_submit = false;
      screen.ExitLoopClosure()();
  }, ButtonOption::Animated());

  auto container = Container::Vertical(
      {radio_tt,
       Container::Horizontal({submit_button, cancel_button})});

  auto container_with_esc = CatchEvent(container, [&](Event event) {
      if (event == Event::Escape) {
          should_submit = false;
          screen.ExitLoopClosure()();
          return true;
      }
      if (event == Event::Return) {
          if (radio_tt->Focused()) {
              submit_button->TakeFocus(); return true;
          }
          if (submit_button->Focused()) {
              should_submit = true;
              screen.ExitLoopClosure()();
              return true;
          }
      }
      return false;
  });

  auto renderer = Renderer(container_with_esc, [&] {
    time_t t = time(0);
    tm* now = localtime(&t);
    std::string current_date = std::to_string(now->tm_mday) + "/" + std::to_string(now->tm_mon + 1) + "/" + std::to_string(now->tm_year + 1900);

    return vbox({text(" TRẢ TRUYỆN & THANH TOÁN ") | bold | center, separator(),
                 hbox(text(" Phiếu ID:   "), text(p_id_str) | bold),
                 hbox(text(" Ngày trả:   "), text(current_date) | bold | color(Color::Cyan)),
                 separator(),
                 text(" Tình trạng truyền lúc trả: ") | bold,
                 radio_tt->Render(), separator(),
                 error_msg.empty() ? text("") : text(error_msg) | color(Color::Red) | center,
                 hbox(submit_button->Render(), text("   "),
                      cancel_button->Render()) |
                     center}) |
           border;
  });

  screen.Loop(renderer);

  if (should_submit) {
    int tt = selected_tt + 1; // 0,1,2,3 -> 1,2,3,4
    time_t t = time(0);
    tm* now = localtime(&t);
    Date d_tra = {now->tm_mday, now->tm_mon + 1, now->tm_year + 1900};

    system("cls");
    process_return_comic(p_id, d_tra, tt, 0.0);
    get_string_input("Nhan Enter de tiep tuc...");
  }
}

