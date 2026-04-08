#include "../../../include/ui/rental/RentalViewUI.h"
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

void handle_view_rentals() {
            system("cls");
            std::vector<RentalSlip> all_slips = get_all_rental_slips();
            if (all_slips.empty()) {
                std::cout << "Khong co phieu thue nao trong he thong!\n";
                get_string_input("Nhan Enter de tiep tuc...");
                return;
            }

            auto form_screen = ScreenInteractive::Fullscreen();
            std::string search_comic = "";
            std::string search_author = "";
            std::string search_cu = "";
            std::string search_type = "";
            std::vector<std::string> status_options = {"Tat ca", "Dang thue", "Da tra/Khac"};
            int selected_status = 0;
            
            std::vector<std::string> sort_options = {"-", "Tang dan", "Giam dan"};
            std::vector<std::string> sort_options_az = {"-", "A -> Z", "Z -> A"};
            int sort_id = 0, sort_c = 0, sort_cu = 0, sort_date = 0;
            int last_clicked_sort = 0;
            
            std::vector<RentalSlip> filtered_slips;
            
            std::vector<Comic> all_c = read_all_comics();
            std::vector<Customer> all_cu = read_all_customers();

            std::vector<std::string> type_options = build_type_options(all_c);
            std::vector<std::string> filtered_type_options;
            int selected_type_option = 0;
            
            std::vector<RentalUIRow> active_rows;
            for (const auto& s : all_slips) {
                RentalUIRow r;
                r.slip = s;
                r.cu_name = get_cu_name(s.customer_id, all_cu);
                r.c_name = get_c_name(s.comic_id, all_c);
                r.c_author = get_c_author(s.comic_id, all_c);
                r.c_type = get_c_type(s.comic_id, all_c);
                active_rows.push_back(r);
            }

            auto apply_filter = [&] {
                filtered_slips.clear();
                std::vector<RentalUIRow> t_rows;
                std::string s_comic = search_comic; for(auto&c : s_comic) c = std::tolower(c);
                std::string s_author = search_author; for(auto&c : s_author) c = std::tolower(c);
                std::string cu_kw = search_cu; for(auto&c : cu_kw) c = std::tolower(c);
                std::string ty_kw = search_type; for(auto&c : ty_kw) c = std::tolower(c);
                
                for (const auto& r : active_rows) {
                    std::string tn = r.c_name; for(auto&c : tn) c = std::tolower(c);
                    std::string ta = r.c_author; for(auto&c : ta) c = std::tolower(c);
                    std::string tk = r.cu_name; for(auto&c : tk) c = std::tolower(c);
                    std::string tl = r.c_type; for(auto&c : tl) c = std::tolower(c);
                    
                    bool m_comic = s_comic.empty() || tn.find(s_comic) != std::string::npos || std::to_string(r.slip.id_phieu) == s_comic;
                    bool m_author = s_author.empty() || ta.find(s_author) != std::string::npos;
                    bool m_cu = cu_kw.empty() || tk.find(cu_kw) != std::string::npos;
                    bool m_ty = ty_kw.empty() || tl.find(ty_kw) != std::string::npos;
                    bool m_st = true;
                    if (selected_status == 1) m_st = (r.slip.trang_thai == 0);
                    if (selected_status == 2) m_st = (r.slip.trang_thai != 0);

                    if (m_comic && m_author && m_cu && m_ty && m_st) {
                        t_rows.push_back(r);
                    }
                }
                
                if (last_clicked_sort == 1) {
                    if (sort_id == 1) quick_sort(t_rows, cmp_rui_id_asc);
                    else if (sort_id == 2) quick_sort(t_rows, cmp_rui_id_desc);
                } else if (last_clicked_sort == 2) {
                    if (sort_c == 1) quick_sort(t_rows, cmp_rui_c_asc);
                    else if (sort_c == 2) quick_sort(t_rows, cmp_rui_c_desc);
                } else if (last_clicked_sort == 3) {
                    if (sort_cu == 1) quick_sort(t_rows, cmp_rui_cu_asc);
                    else if (sort_cu == 2) quick_sort(t_rows, cmp_rui_cu_desc);
                } else if (last_clicked_sort == 4) {
                    if (sort_date == 1) quick_sort(t_rows, cmp_rui_date_asc);
                    else if (sort_date == 2) quick_sort(t_rows, cmp_rui_date_desc);
                }
                
                for (auto& r : t_rows) filtered_slips.push_back(r.slip);
            };

            Component input_search_comic = Input(&search_comic, "Tim ID hoac Ten Truyen...");
            Component input_search_author = Input(&search_author, "Tim theo Tac Gia...");
            Component input_search_cu = Input(&search_cu, "Tim Khach hang/SDT...");
            Component input_search_ty = Input(&search_type, "Go de tim The Loai...");
            Component status_radiobox = Radiobox(&status_options, &selected_status);
            
            auto type_menu_raw = Menu(&filtered_type_options, &selected_type_option);
            auto type_menu_c = CatchEvent(type_menu_raw, [&](Event event) {
              if (event.is_mouse() && event.mouse().motion == Mouse::Moved) return true;
              if (event == Event::Return) {
                 if (!filtered_type_options.empty() && is_valid_type_suggestion(filtered_type_options[selected_type_option])) {
                     search_type = filtered_type_options[selected_type_option];
                 }
                 return true; 
              }
              return false;
            });

            auto t_id = Toggle(&sort_options, &sort_id);
            auto t_c = Toggle(&sort_options_az, &sort_c);
            auto t_cu = Toggle(&sort_options_az, &sort_cu);
            auto t_d = Toggle(&sort_options, &sort_date);
            
            t_id |= CatchEvent([&](Event e) { if(e==Event::Return || e.is_mouse()) { last_clicked_sort=1; sort_c=0; sort_cu=0; sort_date=0; } return false; });
            t_c |= CatchEvent([&](Event e) { if(e==Event::Return || e.is_mouse()) { last_clicked_sort=2; sort_id=0; sort_cu=0; sort_date=0; } return false; });
            t_cu |= CatchEvent([&](Event e) { if(e==Event::Return || e.is_mouse()) { last_clicked_sort=3; sort_id=0; sort_c=0; sort_date=0; } return false; });
            t_d |= CatchEvent([&](Event e) { if(e==Event::Return || e.is_mouse()) { last_clicked_sort=4; sort_id=0; sort_c=0; sort_cu=0; } return false; });

            Component exit_button = Button("Tro Ve", [&] { form_screen.ExitLoopClosure()(); }, ButtonOption::Animated());

            auto controls = Container::Vertical({
                input_search_comic, input_search_author, input_search_cu, input_search_ty, type_menu_c, status_radiobox,
                t_id, t_c, t_cu, t_d, exit_button
            });

            auto controls_with_event = CatchEvent(controls, [&](Event event) {
                if (event == Event::Escape) {
                    form_screen.ExitLoopClosure()();
                    return true;
                }
                return false;
            });

            auto ui_renderer = Renderer(controls_with_event, [&] {
                refresh_type_suggestions(type_options, search_type, filtered_type_options, selected_type_option);
                apply_filter();
                
                auto type_dropdown_box = vbox({
                    input_search_ty->Render(),
                    type_menu_c->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 5) | borderEmpty
                });

                auto filter_panel = vbox({
                    text("--- BỘ LỌC === ") | bold,
                    separator(),
                    hbox(text(" Tên Truyện: "), input_search_comic->Render()),
                    hbox(text(" Tác Giả:    "), input_search_author->Render()),
                    hbox(text(" Khách:      "), input_search_cu->Render()),
                    hbox(text(" Thể loại:   "), type_dropdown_box),
                    separator(),
                    text(" Trang thai: ") | bold,
                    status_radiobox->Render(),
                    separator(),
                    text("--- SẮP XẾP ---") | bold,
                    hbox(text(" ID Phiếu:   "), t_id->Render()),
                    hbox(text(" Tên Truyện: "), t_c->Render()),
                    hbox(text(" Tên Khách:  "), t_cu->Render()),
                    hbox(text(" Ngày Mượn:  "), t_d->Render()),
                    separator(),
                    exit_button->Render() | center
                }) | border | size(WIDTH, GREATER_THAN, 35);

                auto table_panel = window(
                    text(" DANH SACH TẤT CẢ PHIẾU (" + std::to_string(filtered_slips.size()) + ")") | bold | center,
                    build_rental_table_element(filtered_slips, all_c, all_cu) | vscroll_indicator | frame
                ) | flex;

                return hbox({filter_panel, table_panel});
            });

            form_screen.Loop(ui_renderer);
}
