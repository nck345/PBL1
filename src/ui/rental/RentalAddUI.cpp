#include "../../../include/ui/rental/RentalAddUI.h"
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

void render_new_rental_screen() {
  system("cls");
  std::cout << "--- BUOC 1: CHON TRUYEN ---\n";
  int comic_id = select_comic_ui("CHON TRUYEN DE CHO THUE");
  if (comic_id <= 0) {
    std::cout << "Huy thao tac mượn truyện. Nhấn Enter de thoat...\n";
    get_string_input("");
    return;
  }

  Comic popup_comic;
  get_comic_by_id(comic_id, popup_comic);

  bool is_reservation = false;
  Date reservation_start_date = {0, 0, 0};

  if (popup_comic.quantity <= 0) {
      Date earliest = {0,0,0};
      if (get_earliest_return_date(comic_id, earliest)) {
          std::cout << "\n[!] CANH BAO: Truyen nay da het hang trong kho!\n";
          std::cout << "    Ngay du kien co hang som nhat (khach tra): " 
                    << earliest.day << "/" << earliest.month << "/" << earliest.year << "\n";
          
          std::string ans = get_string_input("Ban co muon DAT TRUOC khong? (y/n)");
          if (ans == "y" || ans == "Y") {
              is_reservation = true;
              reservation_start_date = add_days(earliest, 1); // Cho phep lay vao 1 ngay sau luot khach tước
          } else {
              std::cout << "Huy thao tac.\n";
              return;
          }
      } else {
          std::cout << "\n[!] LOI: Truyen nay khong ton tai!\n";
          get_string_input("Nhan Enter de thoat...");
          return;
      }
  }

  system("cls");
  std::cout << "--- BUOC 2: CHON KHACH HANG ---\n";
  int customer_id = select_customer_ui("CHON KHACH HANG MƯỢN TRUYỆN");
  if (customer_id <= 0) {
    std::cout << "Huy thao tac mượn truyện. Nhấn Enter de thoat...\n";
    get_string_input("");
    return;
  }

  system("cls");
  auto screen = ScreenInteractive::TerminalOutput();
  
  double tien_coc = popup_comic.price * 1.2;
  double phi_1_ngay = popup_comic.price * 0.05;
  double phi_3_ngay = popup_comic.price * 0.10;
  double phi_7_ngay = popup_comic.price * 0.20;

  std::string days_str = "";
  Component input_days = Input(&days_str, "Nhập số ngày...");

  std::string error_msg = "";
  bool should_submit = false;

  auto submit_button = Button(is_reservation ? "Xác nhận Đặt trước" : "Xác nhận & Cho thuê", [&] {
      if (days_str.empty()) {
          error_msg = "Lỗi: Vui lòng nhập số ngày thuê!";
          return;
      }
      try {
          int d = std::stoi(days_str);
          if (d <= 0) throw std::invalid_argument("invalid");
      } catch(...) {
          error_msg = "Lỗi: Số ngày thuê không hợp lệ!";
          return;
      }
      error_msg = "";
      should_submit = true;
      screen.ExitLoopClosure()();
  }, ButtonOption::Animated());

  auto cancel_button = Button("Hủy & Trở về", [&] {
      should_submit = false;
      screen.ExitLoopClosure()();
  }, ButtonOption::Animated());

  auto container = Container::Vertical({
       input_days,
       Container::Horizontal({submit_button, cancel_button})
  });

  auto container_with_esc = CatchEvent(container, [&](Event event) {
      if (event == Event::Escape) {
          should_submit = false;
          screen.ExitLoopClosure()();
          return true;
      }
      if (event == Event::Return) {
          if (input_days->Focused()) {
              submit_button->TakeFocus(); return true;
          }
      }
      return false;
  });

  auto renderer = Renderer(container_with_esc, [&] {
    int days = 0;
    try { if (!days_str.empty()) days = std::stoi(days_str); } catch(...) {}
    if (days < 0) days = 0;
    
    int weeks = days / 7;
    int three_days = (days % 7) / 3;
    int one_days = (days % 7) % 3;
    
    double current_phi = weeks * phi_7_ngay + three_days * phi_3_ngay + one_days * phi_1_ngay;
    double tong_tra = tien_coc;
      
    return vbox({text(is_reservation ? " PHIẾU ĐẶT TRƯỚC " : " THIẾT LẬP GÓI THUÊ ") | bold | center, separator(),
                 hbox(text(" Truyện: "), text(popup_comic.comic_name)),
                 is_reservation ? text(" Bắt đầu mượn (dự kiến): " + std::to_string(reservation_start_date.day) + "/" + std::to_string(reservation_start_date.month) + "/" + std::to_string(reservation_start_date.year)) | color(Color::Green) : text(""),
                 hbox(text(" Tiền cọc (120%): "), text(format_currency(tien_coc)) | color(Color::Yellow)),
                 separator(),
                 text(" Mức giá: 1 Ngày(" + format_currency(phi_1_ngay) + "), Combo 3 Ngày(" + format_currency(phi_3_ngay) + "), Combo 1 Tuần(" + format_currency(phi_7_ngay) + ")") | dim,
                 hbox(text(" CHỌN SỐ NGÀY THUÊ: ") | bold, input_days->Render()),
                 text(" Hệ thống tự phân chia: " + std::to_string(weeks) + " tuần, " + std::to_string(three_days) + " combo 3 ngày, " + std::to_string(one_days) + " ngày lẻ.") | color(Color::Blue),
                 separator(),
                 text(" HÓA ĐƠN TẠM TÍNH: ") | bold | color(Color::Cyan),
                 hbox(text(" Tiền cọc (phải đóng): "), text(format_currency(tien_coc))),
                 hbox(text(" Phí thuê dự kiến: "), text(format_currency(current_phi))),
                 hbox(text(" TỔNG PHẢI TRẢ LÚC NÀY (Chỉ cọc): ") | bold, text(format_currency(tong_tra)) | bold | color(Color::Red)),
                 hbox(text(" Số tiền hoàn lại lúc trả (ước tính): ") | color(Color::Green), text(format_currency(tien_coc - current_phi >= 0 ? tien_coc - current_phi : 0)) | color(Color::Green)),
                 separator(),
                 error_msg.empty() ? text("") : text(error_msg) | color(Color::Red) | center,
                 hbox(submit_button->Render(), text("   "),
                      cancel_button->Render()) | center}) | border;
  });

  screen.Loop(renderer);

  if (should_submit) {
    int days_to_add = 0;
    try { days_to_add = std::stoi(days_str); } catch(...) {}
    if (days_to_add <= 0) days_to_add = 1;

    int weeks = days_to_add / 7;
    int three_days = (days_to_add % 7) / 3;
    int one_days = (days_to_add % 7) % 3;
    double expected_phi = weeks * phi_7_ngay + three_days * phi_3_ngay + one_days * phi_1_ngay;
    
    Date d_hien_tai;
    if (is_reservation) {
        d_hien_tai = reservation_start_date;
    } else {
        time_t t = time(0);
        tm* now = localtime(&t);
        d_hien_tai = {now->tm_mday, now->tm_mon + 1, now->tm_year + 1900};
    }
    Date d_tra = add_days(d_hien_tai, days_to_add);

    system("cls");
    int slip_id = process_new_rental(comic_id, customer_id, d_tra, tien_coc, expected_phi, is_reservation, reservation_start_date);
    
    if (slip_id > 0) {
        // Render a beautiful receipt
        Customer c; get_customer_by_id(customer_id, c);
        auto receipt_screen = ScreenInteractive::TerminalOutput();
        auto ok_btn = Button("Đóng Biên Lai", [&] { receipt_screen.ExitLoopClosure()(); }, ButtonOption::Animated());
        
        auto renderer = Renderer(ok_btn, [&]() -> Element {
            return window(is_reservation ? text(" CHI TIẾT ĐẶT TRƯỚC HỆ THỐNG ") | bold : text(" CHI TIẾT PHIẾU THUÊ ") | bold,
                vbox({
                    text(" THÀNH CÔNG! HỆ THỐNG ĐÃ GHI NHẬN: MÃ PHIẾU #" + std::to_string(slip_id)) | bold | color(Color::Green) | center,
                    separator(),
                    text(" Khách hàng: " + std::string(c.name) + " (" + std::string(c.phone) + ")"),
                    text(" Truyện: " + std::string(popup_comic.comic_name) + " - Tác giả: " + std::string(popup_comic.author)),
                    text(" Ngày bắt đầu mượn" + std::string(is_reservation ? " (dự kiến): " : ": ") + std::to_string(d_hien_tai.day) + "/" + std::to_string(d_hien_tai.month) + "/" + std::to_string(d_hien_tai.year)),
                    text(" Hạn trả (dự kiến): " + std::to_string(d_tra.day) + "/" + std::to_string(d_tra.month) + "/" + std::to_string(d_tra.year)) | color(Color::Cyan),
                    text(" Tiền cọc đã nhận: " + format_currency(tien_coc)) | color(Color::Yellow),
                    separator(),
                    ok_btn->Render() | center
                }) | borderEmpty
            ) | center;
        });
        
        receipt_screen.Loop(renderer);
    } else {
        get_string_input("Co loi xay ra, vui long nhan Enter de tiep tuc...");
    }
  }
}

