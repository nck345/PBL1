#include "../../include/ui/RentalUI.h"
#include "../../include/repository/ComicRepo.h"
#include "../../include/repository/RentalRepo.h"
#include "../../include/services/RentalService.h"
#include "../../include/utils/InputHandler.h"
#include "../../include/utils/SortUtils.h"
#include "../../include/utils/ValidationUtils.h"
#include <iostream>
#include <string>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>

using namespace ftxui;

// Helper to parse date strings
Date parse_date_string(const std::string &date_str) {
  Date d = {0, 0, 0};
  if (date_str.length() >= 8) {
    sscanf(date_str.c_str(), "%d/%d/%d", &d.day, &d.month, &d.year);
  }
  return d;
}

// Hàm chọn truyện từ danh sách kết quả tìm kiếm.
// - Nếu tìm thấy đúng 1 kết quả: tự động chọn luôn.
// - Nếu > 1 kết quả: hiện menu để người dùng chọn đúng quyển.
// Trả về Comic được chọn. Nếu không tìm thấy hoặc thoát, id = 0.
Comic select_comic_menu(const std::string &query) {
  std::vector<Comic> results = search_comics_by_name(query);
  if (results.empty())
    return {0, "", "", "", 0, 0, false};
  if (results.size() == 1)
    return results[0];

  auto screen = ScreenInteractive::TerminalOutput();
  std::vector<std::string> entries;
  for (const auto &c : results)
    entries.push_back(std::string(c.comic_name) +
                      " | Gia: " + std::to_string((int)c.price) +
                      "d | Kho: " + std::to_string(c.quantity));
  entries.push_back("[Quay lai]");

  int selected = 0;
  MenuOption opt;
  opt.on_enter = screen.ExitLoopClosure();
  auto menu = Menu(&entries, &selected, opt);
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
    return false;
  });

  auto renderer = Renderer(menu_with_event, [&] {
    return window(text(" Tim thay " + std::to_string(results.size()) +
                       " truyen - Chon dung quyen: "),
                  menu_with_event->Render() | vscroll_indicator | frame);
  });
  screen.Loop(renderer);

  if (selected >= 0 && selected < (int)results.size())
    return results[selected];
  return {0, "", "", "", 0, 0, false}; // Nguoi dung thoat
}

// Hàm gợi ý khách hàng dựa trên lịch sử phiếu thuê đã có.
// - Nếu không thấy khách cũ nào khớp: trả thẳng string vừa nhập.
// - Nếu thấy: hiện menu để chọn khách cũ hoặc dùng thông tin mới nhập.
// Trả về chuỗi thông tin khách. Trả về "" nếu người dùng muốn quay lại.
std::string select_customer_menu(const std::string &query) {
  std::vector<RentalSlip> slips = get_all_rental_slips();
  std::vector<std::string> matches;
  for (const auto &s : slips) {
    std::string info = s.khach_hang;
    if (info.find(query) != std::string::npos) {
      bool dup = false;
      for (const auto &m : matches)
        if (m == info) {
          dup = true;
          break;
        }
      if (!dup)
        matches.push_back(info);
    }
  }
  if (matches.empty())
    return query;

  auto screen = ScreenInteractive::TerminalOutput();
  std::vector<std::string> entries = matches;
  entries.push_back("[Dung thong tin moi]: " + query);
  entries.push_back("[Quay lai]");

  int selected = 0;
  MenuOption opt;
  opt.on_enter = screen.ExitLoopClosure();
  auto menu = Menu(&entries, &selected, opt);
  auto menu_with_event = CatchEvent(menu, [&](Event event) {
    if (event == Event::Escape) {
      selected = entries.size() - 1; // [Quay lai]
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
    return false;
  });

  auto renderer = Renderer(menu_with_event, [&] {
    return window(text(" Goi y khach hang cu (chon hoac nhap moi): "),
                  menu_with_event->Render() | vscroll_indicator | frame);
  });
  screen.Loop(renderer);

  if (selected >= 0 && selected < (int)matches.size())
    return matches[selected];
  if (selected == (int)matches.size())
    return query;
  return ""; // Quay lai
}

void render_new_rental_screen() {
  system("cls");
  auto screen = ScreenInteractive::TerminalOutput();

  std::string ten_truyen_str;
  std::string khach_info_str;
  std::string ngay_muon_str;
  std::string ngay_tra_str;
  std::string error_msg = "";

  InputOption iopt;
  iopt.multiline = false;

  Component input_truyen = Input(&ten_truyen_str, "nhập tên truyện...", iopt);
  Component input_khach = Input(&khach_info_str, "nhập tên hoặc SĐT...", iopt);
  Component input_ngay_muon = Input(&ngay_muon_str, "dd/mm/yyyy", iopt);
  Component input_ngay_tra = Input(&ngay_tra_str, "dd/mm/yyyy", iopt);
  bool should_submit = false;

  auto submit_button = Button("Xác nhận & Cho thuê", [&] {
    if (ten_truyen_str.empty() || khach_info_str.empty() ||
        ngay_muon_str.empty() || ngay_tra_str.empty()) {
      error_msg = "Lỗi: Không được để trống thông tin!";
      return;
    }
    Date d_muon = parse_date_string(ngay_muon_str);
    Date d_tra = parse_date_string(ngay_tra_str);
    if (d_muon.year == 0 || d_tra.year == 0) {
      error_msg = "Lỗi: Định dạng ngày không hợp lệ (nhập dd/mm/yyyy)!";
      return;
    }
    if (date_to_days(d_tra) < date_to_days(d_muon)) {
      error_msg = "Lỗi: Ngày trả dự kiến < ngày mượn!";
      return;
    }
    error_msg = "";
    should_submit = true;
    screen.ExitLoopClosure()();
  });

  auto cancel_button = Button("Hủy & Trở về", [&] {
    should_submit = false;
    screen.ExitLoopClosure()();
  });

  auto container = Container::Vertical(
      {input_truyen, input_khach, input_ngay_muon, input_ngay_tra,
       Container::Horizontal({submit_button, cancel_button})});

  auto container_with_esc = CatchEvent(container, [&](Event event) {
    if (event == Event::Escape) {
      should_submit = false;
      screen.ExitLoopClosure()();
      return true;
    }
    if (event == Event::Return) {
      if (input_truyen->Focused()) {
        input_khach->TakeFocus();
        return true;
      }
      if (input_khach->Focused()) {
        input_ngay_muon->TakeFocus();
        return true;
      }
      if (input_ngay_muon->Focused()) {
        input_ngay_tra->TakeFocus();
        return true;
      }
      if (input_ngay_tra->Focused()) {
        if (ten_truyen_str.empty() || khach_info_str.empty() ||
            ngay_muon_str.empty() || ngay_tra_str.empty()) {
          error_msg = "Lỗi: Không được để trống thông tin!";
          return true;
        }
        Date d_muon = parse_date_string(ngay_muon_str);
        Date d_tra = parse_date_string(ngay_tra_str);
        if (d_muon.year == 0 || d_tra.year == 0) {
          error_msg = "Lỗi: Định dạng ngày không hợp lệ (nhập dd/mm/yyyy)!";
          return true;
        }
        if (date_to_days(d_tra) < date_to_days(d_muon)) {
          error_msg = "Lỗi: Ngày trả dự kiến < ngày mượn!";
          return true;
        }
        should_submit = true;
        screen.ExitLoopClosure()();
        return true;
      }
    }
    return false;
  });

  auto renderer = Renderer(container_with_esc, [&] {
    return vbox({text(" THIẾT LẬP PHIẾU THUÊ ") | bold | center, separator(),
                 hbox(text(" Tên Truyện:      "), input_truyen->Render()),
                 hbox(text(" Khách hàng:      "), input_khach->Render()),
                 hbox(text(" Ngày mượn:      "), input_ngay_muon->Render()),
                 hbox(text(" Hạn trả (dự kiến): "), input_ngay_tra->Render()),
                 separator(),
                 error_msg.empty()
                     ? text("")
                     : text(error_msg) | color(Color::Red) | center,
                 hbox(submit_button->Render(), text("   "),
                      cancel_button->Render()) |
                     center}) |
           border;
  });

  screen.Loop(renderer);

  if (should_submit) {
    Date d_muon = parse_date_string(ngay_muon_str);
    Date d_tra = parse_date_string(ngay_tra_str);

    system("cls");
    // Buoc 1: Chon chinh xac quyen truyen
    Comic chosen_comic = select_comic_menu(ten_truyen_str);
    if (chosen_comic.id == 0) {
      std::cout << "Khong tim thay truyen hoac da quay lai.\n";
      get_string_input("Nhan Enter de thu lai...");
      return;
    }
    // Buoc 2: Xac nhan thong tin khach hang
    std::string final_khach = select_customer_menu(khach_info_str);
    if (final_khach.empty())
      return; // Nguoi dung quay lai

    system("cls");
    process_new_rental(chosen_comic.comic_name, final_khach.c_str(), d_muon,
                       d_tra, 0.0);
    get_string_input("Nhan Enter de tiep tuc...");
  }
}

void render_return_comic_screen() {
  system("cls");
  auto screen = ScreenInteractive::TerminalOutput();

  std::string phieu_id_str;
  std::string ngay_tra_str;
  std::string trang_thai_str; // 1: tra binh thuong, 2: mat hong
  std::string error_msg = "";
  bool should_submit = false;

  InputOption iopt_r;
  iopt_r.multiline = false;

  Component input_phieu = Input(&phieu_id_str, "nhập số ID...", iopt_r);
  Component input_ngay = Input(&ngay_tra_str, "dd/mm/yyyy", iopt_r);
  Component input_tt =
      Input(&trang_thai_str, "1: Bình thường, 2: Mất/Hỏng", iopt_r);

  auto submit_button = Button(
      "Xác nhận & Thanh toán",
      [&] {
        if (phieu_id_str.empty() || ngay_tra_str.empty() ||
            trang_thai_str.empty()) {
          error_msg = "Lỗi: Không được để trống thông tin!";
          return;
        }
        try {
          int id = std::stoi(phieu_id_str);
          if (id <= 0)
            throw std::invalid_argument("<=0");
        } catch (...) {
          error_msg = "Lỗi: ID Phiếu phải là một số nguyên dương!";
          return;
        }
        try {
          int tt = std::stoi(trang_thai_str);
          if (tt != 1 && tt != 2)
            throw std::invalid_argument("not 1 or 2");
        } catch (...) {
          error_msg = "Lỗi: Trạng thái chỉ được nhập 1 hoặc 2!";
          return;
        }
        Date d_tra = parse_date_string(ngay_tra_str);
        if (d_tra.year == 0) {
          error_msg = "Lỗi: Định dạng ngày trả không hợp lệ (nhập dd/mm/yyyy)!";
          return;
        }
        error_msg = "";
        should_submit = true;
        screen.ExitLoopClosure()();
      },
      ButtonOption::Animated());

  auto cancel_button = Button(
      "Hủy & Trở về",
      [&] {
        should_submit = false;
        screen.ExitLoopClosure()();
      },
      ButtonOption::Animated());

  auto container = Container::Vertical(
      {input_phieu, input_ngay, input_tt,
       Container::Horizontal({submit_button, cancel_button})});

  auto container_with_esc = CatchEvent(container, [&](Event event) {
    if (event == Event::Escape) {
      should_submit = false;
      screen.ExitLoopClosure()();
      return true;
    }
    if (event == Event::Return) {
      if (input_phieu->Focused()) {
        input_ngay->TakeFocus();
        return true;
      }
      if (input_ngay->Focused()) {
        input_tt->TakeFocus();
        return true;
      }
      if (input_tt->Focused()) {
        if (phieu_id_str.empty() || ngay_tra_str.empty() ||
            trang_thai_str.empty()) {
          error_msg = "Lỗi: Không được để trống thông tin!";
          return true;
        }
        try {
          int id = std::stoi(phieu_id_str);
          if (id <= 0)
            throw std::invalid_argument("<=0");
        } catch (...) {
          error_msg = "Lỗi: ID Phiếu phải là một số nguyên dương!";
          return true;
        }
        try {
          int tt = std::stoi(trang_thai_str);
          if (tt != 1 && tt != 2)
            throw std::invalid_argument("not 1 or 2");
        } catch (...) {
          error_msg = "Lỗi: Trạng thái chỉ được nhập 1 hoặc 2!";
          return true;
        }
        Date d_tra = parse_date_string(ngay_tra_str);
        if (d_tra.year == 0) {
          error_msg = "Lỗi: Định dạng ngày trả không hợp lệ!";
          return true;
        }
        should_submit = true;
        screen.ExitLoopClosure()();
        return true;
      }
    }
    return false;
  });

  auto renderer = Renderer(container_with_esc, [&] {
    return vbox({text(" TRẢ TRUYỆN & THANH TOÁN ") | bold | center, separator(),
                 hbox(text(" Phiếu ID:   "), input_phieu->Render()),
                 hbox(text(" Ngày trả:   "), input_ngay->Render()),
                 hbox(text(" Tình trạng: "), input_tt->Render()), separator(),
                 error_msg.empty()
                     ? text("")
                     : text(error_msg) | color(Color::Red) | center,
                 hbox(submit_button->Render(), text("   "),
                      cancel_button->Render()) |
                     center}) |
           border;
  });

  screen.Loop(renderer);

  if (should_submit) {
    int p_id = std::stoi(phieu_id_str);
    int tt = std::stoi(trang_thai_str);
    Date d_tra = parse_date_string(ngay_tra_str);

    system("cls");
    process_return_comic(p_id, d_tra, tt, 0.0);

    // Đọc lại phiếu đã được cập nhật để hiển thị hóa đơn
    std::vector<RentalSlip> all_slips = get_all_rental_slips();
    RentalSlip paid_slip;
    bool found_slip = false;
    for (const auto &s : all_slips) {
      if (s.id_phieu == p_id) {
        paid_slip = s;
        found_slip = true;
        break;
      }
    }

    if (found_slip) {
      // Tính lại các mục nhỏ để hiển thị chi tiết
      long so_ngay_thue = date_to_days(paid_slip.ngay_tra_thuc_te) -
                          date_to_days(paid_slip.ngay_muon);
      if (so_ngay_thue < 1)
        so_ngay_thue = 1;

      long so_ngay_qua_han = date_to_days(paid_slip.ngay_tra_thuc_te) -
                             date_to_days(paid_slip.ngay_tra_du_kien);
      if (so_ngay_qua_han < 0)
        so_ngay_qua_han = 0;

      double phi_phat = (paid_slip.trang_thai == 1)
                            ? (so_ngay_qua_han * PHI_PHAT_QUA_HAN_MOT_NGAY)
                            : 0.0;
      double tien_thue =
          (paid_slip.trang_thai == 1)
              ? (so_ngay_thue * TY_LE_THUE_THEO_NGAY * paid_slip.tien_coc)
              : 0.0;
      double tien_coc = paid_slip.tien_coc;
      double tong_tien = paid_slip.tong_tien;
      double tra_lai = tien_coc - tong_tien;
      if (tra_lai < 0)
        tra_lai = 0;

      auto bill_screen = ScreenInteractive::TerminalOutput();
      auto btn_close =
          Button("[ Dong hoa don ]", bill_screen.ExitLoopClosure());

      auto bill_renderer = Renderer(btn_close, [&] {
        bool is_lost = (paid_slip.trang_thai == 2);

        Elements bill_rows;
        bill_rows.push_back(hbox(
            text(" Phieu ID:         "),
            text(std::to_string(paid_slip.id_phieu)) | bold | color(Color::Cyan)));
        bill_rows.push_back(
            hbox(text(" Ten Truyen:       "), text(paid_slip.ten_truyen) | bold));
        bill_rows.push_back(
            hbox(text(" Khach Hang:       "), text(paid_slip.khach_hang) | bold));
        bill_rows.push_back(hbox(
            text(" Ngay Muon:        "),
            text(std::to_string(paid_slip.ngay_muon.day) + "/" +
                 std::to_string(paid_slip.ngay_muon.month) + "/" +
                 std::to_string(paid_slip.ngay_muon.year))));
        bill_rows.push_back(hbox(
            text(" Ngay Tra:         "),
            text(std::to_string(paid_slip.ngay_tra_thuc_te.day) + "/" +
                 std::to_string(paid_slip.ngay_tra_thuc_te.month) + "/" +
                 std::to_string(paid_slip.ngay_tra_thuc_te.year))));
        bill_rows.push_back(separator());

        if (is_lost) {
          bill_rows.push_back(hbox(text(" Tinh Trang:       "),
                                   text("MAT / HONG - Tich thu tien coc!") |
                                       bold | color(Color::Red)));
          bill_rows.push_back(hbox(text(" Tien Coc (giu lai) "),
                                   text(format_currency(tien_coc)) | bold |
                                       color(Color::Red)));
          bill_rows.push_back(hbox(text(" Tong Tien Phat:    "),
                                   text(format_currency(tong_tien)) | bold |
                                       color(Color::Red)));
          bill_rows.push_back(hbox(text(" Tien Tra Lai KH:  "),
                                   text("0 VND (khong hoan coc)") | bold |
                                       color(Color::Red)));
        } else {
          bill_rows.push_back(hbox(text(" So Ngay Thue:     "),
                                   text(std::to_string(so_ngay_thue) + " ngay") |
                                       bold));
          bill_rows.push_back(hbox(text(" Tien Coc Da Thu:  "),
                                   text(format_currency(tien_coc)) |
                                       color(Color::Yellow)));
          bill_rows.push_back(hbox(text(" Tien Thue:        "),
                                   text(format_currency(tien_thue)) |
                                       color(Color::Yellow)));
          if (phi_phat > 0) {
            bill_rows.push_back(
                hbox(text(" Phi Phat Qua Han: "),
                     text("+" + std::string(format_currency(phi_phat))) | bold |
                         color(Color::Red)));
            bill_rows.push_back(hbox(
                text("   ("), text(std::to_string(so_ngay_qua_han)),
                text(" ngay x 5.000 VND/ngay)") | dim));
          }
          bill_rows.push_back(separator());
          bill_rows.push_back(
              hbox(text(" TONG TIEN PHAI THU: "),
                   text(format_currency(tong_tien)) | bold | color(Color::Green)) |
              bold);
          if (tra_lai > 0) {
            bill_rows.push_back(hbox(text(" TIEN TRA LAI KHACH: "),
                                     text(format_currency(tra_lai)) | bold |
                                         color(Color::Cyan)) |
                                bold);
          } else {
            bill_rows.push_back(hbox(text(" TIEN THU THEM:      "),
                                     text(format_currency(tong_tien - tien_coc)) |
                                         bold | color(Color::Red)) |
                                bold);
          }
        }

        bill_rows.push_back(separator());
        bill_rows.push_back(btn_close->Render() | center);

        return window(text(" HOA DON THANH TOAN ") | bold | color(Color::Green),
                      vbox(bill_rows));
      });

      bill_screen.Loop(bill_renderer);
    } else {
      get_string_input("Nhan Enter de tiep tuc...");
    }
  }
}

void render_rental_menu() {
  auto screen = ScreenInteractive::TerminalOutput();

  std::vector<std::string> entries = {
      "1. Cho thue truyen moi", "2. Tra truyen & Thanh toan", "3. Tro ve"};
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
    return false;
  });

  auto renderer = Renderer(menu_with_event, [&] {
    return window(text(" QUAN LY PHIEU THUE "),
                  menu_with_event->Render() | vscroll_indicator | frame) |
           bold;
  });

  while (true) {
    system("cls");
    screen.Loop(renderer);

    if (selected == 0) {
      render_new_rental_screen();
    } else if (selected == 1) {
      render_return_comic_screen();
    } else if (selected == 2) {
      system("cls");
      break;
    }
  }
}

// --- SUB-SCREENS CHO THONG KE ---
void render_revenue_chart_screen() {
  auto screen = ScreenInteractive::TerminalOutput();
  Date today = get_current_date();

  int target_d = today.day;
  int target_m = today.month;
  int target_y = today.year;

  int end_m = today.month;
  int end_y = today.year;
  int start_m = today.month - 5;
  int start_y = today.year;
  if (start_m <= 0) {
    start_m += 12;
    start_y -= 1;
  }

  std::string start_m_str = std::to_string(start_m);
  std::string start_y_str = std::to_string(start_y);
  std::string end_m_str = std::to_string(end_m);
  std::string end_y_str = std::to_string(end_y);

  InputOption iopt;
  iopt.multiline = false;

  Component input_sm = Input(&start_m_str, "MM", iopt);
  Component input_sy = Input(&start_y_str, "YYYY", iopt);
  Component input_em = Input(&end_m_str, "MM", iopt);
  Component input_ey = Input(&end_y_str, "YYYY", iopt);

  auto safe_stoi = [](const std::string &str, int fallback) {
    if (str.empty())
      return fallback;
    try {
      return std::stoi(str);
    } catch (...) {
      return fallback;
    }
  };

  rental_statistics stats =
      compute_all_statistics(today, start_m, start_y, end_m, end_y);

  auto refresh_action = [&] {
    start_m = safe_stoi(start_m_str, start_m);
    start_y = safe_stoi(start_y_str, start_y);
    end_m = safe_stoi(end_m_str, end_m);
    end_y = safe_stoi(end_y_str, end_y);
    stats = compute_all_statistics(today, start_m, start_y, end_m, end_y);
  };

  auto btn_refresh = Button("[ Loc / Lam moi ]", refresh_action);
  auto btn_exit = Button("[ Tro ve ]", screen.ExitLoopClosure());

  auto filter_container = Container::Vertical(
      {Container::Horizontal({input_sm, input_sy, input_em, input_ey}),
       Container::Horizontal({btn_refresh, btn_exit})});

  auto renderer = Renderer(filter_container, [&] {
    int max_val = 1;
    for (int v : stats.chart_data) {
      if (v > max_val)
        max_val = v;
    }

    Elements chart_rows;
    for (int i = 0; i < (int)stats.chart_data.size(); ++i) {
      int month_idx = start_m + i;
      int year_idx = start_y;
      while (month_idx > 12) {
        month_idx -= 12;
        year_idx += 1;
      }

      float ratio = (float)stats.chart_data[i] / max_val;
      std::string label = " T" + std::to_string(month_idx) + "/" +
                          std::to_string(year_idx) + " ";
      std::string val_str = " (" + format_currency(stats.chart_data[i]) + ") ";

      chart_rows.push_back(
          hbox({text(label) | size(WIDTH, EQUAL, 12),
                gauge(ratio) | color(Color::Green) | size(WIDTH, EQUAL, 50),
                text(val_str)}));
    }

    if (chart_rows.empty()) {
      chart_rows.push_back(
          text("Khong co du lieu trong khoang thoi gian nay.") | center | dim);
    }

    return vbox(
        {text(" BIEU DO DOANH THU THEO THANG ") | bold | center, separator(),
         vbox(chart_rows) | border, separator(),
         hbox({vbox({text(" Loc Doanh Thu (Khoang Thoi Gian) ") | bold,
                     hbox(text(" Tu Thang: "),
                          input_sm->Render() | size(WIDTH, LESS_THAN, 5),
                          text(" Nam: "),
                          input_sy->Render() | size(WIDTH, LESS_THAN, 7)),
                     hbox(text(" Den Thang: "),
                          input_em->Render() | size(WIDTH, LESS_THAN, 5),
                          text(" Nam: "),
                          input_ey->Render() | size(WIDTH, LESS_THAN, 7))}) |
                   border,
               filler(),
               vbox({text(" Tuong Tac ") | bold, separator(),
                     hbox(btn_refresh->Render(), text("   "),
                          btn_exit->Render())}) |
                   border})});
  });

  screen.Loop(renderer);
}

void render_revenue_details_screen() {
  auto screen = ScreenInteractive::TerminalOutput();
  std::vector<RentalSlip> slips = get_all_rental_slips();

  Date today = get_current_date();
  int target_d = today.day;
  int target_m = today.month;
  int target_y = today.year;

  std::string day_str = std::to_string(target_d);
  std::string month_str = std::to_string(target_m);
  std::string year_str = std::to_string(target_y);

  InputOption iopt;
  iopt.multiline = false;

  Component input_day = Input(&day_str, "DD", iopt);
  Component input_month = Input(&month_str, "MM", iopt);
  Component input_year = Input(&year_str, "YYYY", iopt);

  auto safe_stoi = [](const std::string &str, int fallback) {
    if (str.empty())
      return fallback;
    try {
      return std::stoi(str);
    } catch (...) {
      return fallback;
    }
  };

  rental_statistics stats = compute_all_statistics(
      today, today.month, today.year, today.month, today.year);

  auto refresh_action = [&] {
    int d = safe_stoi(day_str, today.day);
    int m = safe_stoi(month_str, today.month);
    int y = safe_stoi(year_str, today.year);
    Date target = {d, m, y};
    stats = compute_all_statistics(target, m, y, m, y);
  };

  auto btn_refresh = Button("[ Loc / Lam moi ]", refresh_action);
  auto btn_exit = Button("[ Tro ve ]", screen.ExitLoopClosure());

  auto filter_container = Container::Vertical(
      {Container::Horizontal({input_day, input_month, input_year}),
       Container::Horizontal({btn_refresh, btn_exit})});

  quick_sort(slips, compare_revenue_desc);

  std::vector<std::vector<std::string>> main_table_data;
  main_table_data.push_back({"  ID  ", "       Ten Truyen       ",
                             "      Khach Hang      ", " Ngay Muon ",
                             "  Han Tra  ", "  Thuc Te  ", "  Tien Coc  ",
                             "  Tong Tien  ", " Trang Thai "});
  for (const auto &s : slips) {
    std::string ngay_m = std::to_string(s.ngay_muon.day) + "/" +
                         std::to_string(s.ngay_muon.month) + "/" +
                         std::to_string(s.ngay_muon.year);
    std::string ngay_d = std::to_string(s.ngay_tra_du_kien.day) + "/" +
                         std::to_string(s.ngay_tra_du_kien.month) + "/" +
                         std::to_string(s.ngay_tra_du_kien.year);
    std::string ngay_t = (s.ngay_tra_thuc_te.year > 1900)
                             ? (std::to_string(s.ngay_tra_thuc_te.day) + "/" +
                                std::to_string(s.ngay_tra_thuc_te.month) + "/" +
                                std::to_string(s.ngay_tra_thuc_te.year))
                             : "---";
    std::string tt = (s.trang_thai == 1)   ? "Da Tra"
                     : (s.trang_thai == 2) ? "Mat/Hong"
                                           : "Dang Thue";

    main_table_data.push_back({std::to_string(s.id_phieu), s.ten_truyen,
                               s.khach_hang, ngay_m, ngay_d, ngay_t,
                               format_currency(s.tien_coc),
                               format_currency(s.tong_tien), tt});
  }

  auto renderer = Renderer(filter_container, [&] {
    auto main_tbl_obj = Table(main_table_data);
    main_tbl_obj.SelectAll().SeparatorVertical();
    main_tbl_obj.SelectRow(0).Decorate(bold);
    main_tbl_obj.SelectRow(0).SeparatorHorizontal();
    main_tbl_obj.SelectAll().Border(LIGHT);
    auto main_tbl_element = main_tbl_obj.Render();

    return vbox(
        {text(" CHI TIET DOANH THU & DANH SACH PHIEU ") | bold | center,
         separator(),

         hbox({vbox({text(" Loc Doanh Thu Theo Ngay ") | bold,
                     hbox(text(" Ngay: "),
                          input_day->Render() | size(WIDTH, LESS_THAN, 5),
                          text(" Thang: "),
                          input_month->Render() | size(WIDTH, LESS_THAN, 5),
                          text(" Nam: "),
                          input_year->Render() | size(WIDTH, LESS_THAN, 7))}) |
                   border,
               filler(),
               text(" Danh thu ngay: " +
                    std::string(format_currency(stats.target_daily_revenue))) |
                   bold | color(Color::Cyan) | border,
               vbox({text(" Tuong Tac ") | bold, separator(),
                     hbox(btn_refresh->Render(), text("   "),
                          btn_exit->Render())}) |
                   border}),

         separator(), main_tbl_element | center});
  });

  screen.Loop(renderer);
}

void render_overdue_screen() {
  auto screen = ScreenInteractive::TerminalOutput();
  std::vector<RentalSlip> slips = get_all_rental_slips();
  Date today = get_current_date();

  std::vector<RentalSlip> overdue_slips;
  for (const auto &s : slips) {
    if (s.trang_thai == 0 &&
        date_to_days(today) > date_to_days(s.ngay_tra_du_kien)) {
      overdue_slips.push_back(s);
    }
  }
  quick_sort(overdue_slips, compare_overdue_priority_desc);

  std::vector<std::vector<std::string>> overdue_table_data;
  overdue_table_data.push_back({"  ID  ", "       Ten Truyen       ",
                                "      Khach Hang      ", "  Han Tra  ",
                                " So Ngay Tre "});
  for (const auto &s : overdue_slips) {
    std::string delay_str = "N/A";
    if (s.ngay_tra_du_kien.year > 1900) {
      long delay = date_to_days(today) - date_to_days(s.ngay_tra_du_kien);
      delay_str = std::to_string(delay) + " ngay";
    } else {
      delay_str = "Loi du lieu";
    }
    overdue_table_data.push_back(
        {std::to_string(s.id_phieu), s.ten_truyen, s.khach_hang,
         std::to_string(s.ngay_tra_du_kien.day) + "/" +
             std::to_string(s.ngay_tra_du_kien.month) + "/" +
             std::to_string(s.ngay_tra_du_kien.year),
         delay_str});
  }

  auto btn_exit = Button("[ Tro ve ]", screen.ExitLoopClosure());

  auto renderer = Renderer(btn_exit, [&] {
    auto ovd_tbl_obj = Table(overdue_table_data);
    ovd_tbl_obj.SelectAll().SeparatorVertical();
    ovd_tbl_obj.SelectRow(0).Decorate(bold);
    ovd_tbl_obj.SelectRow(0).SeparatorHorizontal();
    ovd_tbl_obj.SelectAll().Border(LIGHT);
    ovd_tbl_obj.SelectAll().Decorate(color(Color::Red));
    auto ovd_tbl_element = ovd_tbl_obj.Render();

    return vbox({text(" DANH SACH PHIEU QUA HAN (Can Uu Tien Doi) ") | bold |
                     color(Color::Red) | center,
                 separator(),
                 overdue_table_data.size() > 1
                     ? ovd_tbl_element | center
                     : text(" (Khong co phieu nao qua han) ") | center | dim,
                 separator(), btn_exit->Render() | center});
  });

  screen.Loop(renderer);
}

// Hàm chính cho Thống kê Menu
void render_statistics_screen() {
  auto screen = ScreenInteractive::TerminalOutput();
  int selected = 0;
  std::vector<std::string> entries = {
      "1. Bieu do Doanh thu Thang", "2. Chi tiet Bang ke Doanh thu",
      "3. Danh sach Phan Hoi / Qua Han", "4. Quay lai"};

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
      if (c >= '1' && c <= '4') {
        selected = c - '1';
        screen.ExitLoopClosure()();
        return true;
      }
    }
    return false;
  });

  auto renderer = Renderer(menu_with_event, [&] {
    return window(text(" THONG KE & BAO CAO "),
                  menu_with_event->Render() | vscroll_indicator | frame) |
           bold;
  });

  while (true) {
    system("cls");
    screen.Loop(renderer);

    if (selected == 0) {
      render_revenue_chart_screen();
    } else if (selected == 1) {
      render_revenue_details_screen();
    } else if (selected == 2) {
      render_overdue_screen();
    } else if (selected == 3) {
      break;
    }
  }
}
