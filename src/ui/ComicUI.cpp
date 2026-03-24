#include "../../include/ui/ComicUI.h"
#include "../../include/repository/ComicRepo.h"
#include "../../include/utils/InputHandler.h"
#include "../../include/utils/StringUtils.h"
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>

using namespace ftxui;

void render_comic_table(const std::vector<Comic> &comics) {
  if (comics.empty()) {
    std::cout << "Khong co du lieu truyen.\n";
    return;
  }

  std::vector<std::vector<std::string>> table_data;
  table_data.push_back({"ID", "Ten Truyen", "Tac Gia", "Gia", "So Luong"});

  bool has_data = false;
  for (const auto &comic : comics) {
    if (!comic.is_deleted) {
      has_data = true;
      table_data.push_back({std::to_string(comic.id), comic.comic_name,
                            comic.author, format_currency(comic.price),
                            std::to_string(comic.quantity)});
    }
  }

  if (!has_data) {
    std::cout << "Khong co du lieu truyen (hoac da bi xoa).\n";
    return;
  }

  auto table = Table(table_data);
  table.SelectAll().Border(LIGHT);

  table.SelectRow(0).Decorate(bold);
  table.SelectRow(0).SeparatorVertical(LIGHT);
  table.SelectRow(0).Border(DOUBLE);

  // Alignment
  // table.SelectColumn(0).DecorateCells(align_right);

  auto document = table.Render();
  auto screen =
      Screen::Create(Dimension::Fit(document), Dimension::Fit(document));
  Render(screen, document);
  screen.Print();
  std::cout << "\n";
}

void render_comic_menu() {
  auto screen = ScreenInteractive::TerminalOutput();

  std::vector<std::string> entries = {
      "1. Xem danh sach truyen", "2. Them truyen moi",
      "3. Sua thong tin truyen", "4. Xoa truyen",
      "5. Tim kiem truyen",      "6. Tro ve"};
  int selected = 0;

  MenuOption option;
  option.on_enter = screen.ExitLoopClosure();

  auto menu = Menu(&entries, &selected, option);
  auto renderer = Renderer(menu, [&] {
    return window(text(" QUAN LY TRUYEN "),
                  menu->Render() | vscroll_indicator | frame) |
           bold;
  });

  while (true) {
    screen.Loop(renderer);

    if (selected == 0) {
      system("cls");
      std::cout << "\n--- DANH SACH TRUYEN ---\n";
      std::vector<Comic> comics = read_all_comics();
      render_comic_table(comics);
      get_string_input("Nhan Enter de tiep tuc...");
    } else if (selected == 1) {
      system("cls");
      std::cout << "\n--- THEM TRUYEN MOI ---\n";
      Comic new_comic;

      std::string name = get_string_input("Nhap ten truyen: ");
      std::string author = get_string_input("Nhap tac gia: ");
      double price = get_double_input("Nhap gia: ");
      int quantity = get_int_input("Nhap so luong: ");

      strncpy(new_comic.comic_name, name.c_str(),
              sizeof(new_comic.comic_name) - 1);
      new_comic.comic_name[sizeof(new_comic.comic_name) - 1] = '\0';

      strncpy(new_comic.author, author.c_str(), sizeof(new_comic.author) - 1);
      new_comic.author[sizeof(new_comic.author) - 1] = '\0';

      new_comic.price = price;
      new_comic.quantity = quantity;
      new_comic.is_deleted = false;

      add_comic(new_comic);
      std::cout << "Them truyen thanh cong!\n";
      get_string_input("Nhan Enter de tiep tuc...");
    } else if (selected == 2) {
      system("cls");
      std::cout << "\n--- SUA THONG TIN TRUYEN ---\n";
      int id = get_int_input("Nhap ID truyen can sua: ");
      Comic comic_to_edit;
      if (get_comic_by_id(id, comic_to_edit)) {
        std::cout << "Dang sua truyen: " << comic_to_edit.comic_name << "\n";
        std::string name = get_string_input("Nhap ten truyen moi: ");
        std::string author = get_string_input("Nhap tac gia moi: ");
        double price = get_double_input("Nhap gia moi: ");
        int quantity = get_int_input("Nhap so luong moi: ");

        strncpy(comic_to_edit.comic_name, name.c_str(),
                sizeof(comic_to_edit.comic_name) - 1);
        comic_to_edit.comic_name[sizeof(comic_to_edit.comic_name) - 1] = '\0';

        strncpy(comic_to_edit.author, author.c_str(),
                sizeof(comic_to_edit.author) - 1);
        comic_to_edit.author[sizeof(comic_to_edit.author) - 1] = '\0';

        comic_to_edit.price = price;
        comic_to_edit.quantity = quantity;

        if (update_comic(comic_to_edit)) {
          std::cout << "Sua truyen thanh cong!\n";
        } else {
          std::cout << "Loi khi sua truyen!\n";
        }
      } else {
        std::cout << "Khong tim thay truyen thuoc ID nay!\n";
      }
      get_string_input("Nhan Enter de tiep tuc...");
    } else if (selected == 3) {
      system("cls");
      std::cout << "\n--- XOA TRUYEN ---\n";
      int id = get_int_input("Nhap ID truyen can xoa: ");
      if (delete_comic(id)) {
        std::cout << "Xoa truyen thanh cong!\n";
      } else {
        std::cout << "Loi khi xoa truyen nguyen do file nhieu du lieu bi hong "
                     "hoac ID khong ton tai!\n";
      }
      get_string_input("Nhan Enter de tiep tuc...");
    } else if (selected == 4) {
      system("cls");
      std::cout << "\n--- TIM KIEM TRUYEN ---\n";
      std::string kw = get_string_input("Nhap ten truyen can tim: ");
      std::vector<Comic> res = search_comics_by_name(kw);
      render_comic_table(res);
      get_string_input("Nhan Enter de tiep tuc...");
    } else if (selected == 5) {
      system("cls");
      break;
    }
  }
}
