#include "../../include/repository/BookCopyRepo.h"
#include "../../include/repository/ComicRepo.h"
#include <fstream>
#include <iostream>
#include <algorithm>

const char* BOOK_COPIES_FILE = "data/book_copies.dat";

std::vector<BookCopy> read_all_book_copies() {
    std::vector<BookCopy> list;
    std::ifstream file(BOOK_COPIES_FILE, std::ios::binary);
    if (!file) return list;

    BookCopy copy;
    while (file.read(reinterpret_cast<char*>(&copy), sizeof(BookCopy))) {
        list.push_back(copy);
    }
    file.close();
    return list;
}

int get_next_copy_id() {
    std::vector<BookCopy> list = read_all_book_copies();
    int max_id = 0;
    for (const auto& c : list) {
        if (c.copy_id > max_id) {
            max_id = c.copy_id;
        }
    }
    return max_id + 1;
}

bool save_book_copy(const BookCopy& copy) {
    std::ofstream file(BOOK_COPIES_FILE, std::ios::binary | std::ios::app);
    if (!file) return false;
    file.write(reinterpret_cast<const char*>(&copy), sizeof(BookCopy));
    file.close();
    return true;
}

bool update_book_copy(const BookCopy& updated_copy) {
    std::fstream file(BOOK_COPIES_FILE, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) return false;

    BookCopy copy;
    bool found = false;
    while (file.read(reinterpret_cast<char*>(&copy), sizeof(BookCopy))) {
        if (copy.copy_id == updated_copy.copy_id) {
            file.seekp(-static_cast<std::streamoff>(sizeof(BookCopy)), std::ios::cur);
            file.write(reinterpret_cast<const char*>(&updated_copy), sizeof(BookCopy));
            found = true;
            break;
        }
    }
    file.close();
    return found;
}

bool add_copies_for_comic(int comic_id, int count) {
    for (int i = 0; i < count; ++i) {
        BookCopy copy;
        copy.copy_id = get_next_copy_id();
        copy.comic_id = comic_id;
        copy.condition_status = 0; // Mới
        copy.is_rented = false;
        if (!save_book_copy(copy)) return false;
    }
    return true;
}

int rent_available_copy(int comic_id) {
    std::fstream file(BOOK_COPIES_FILE, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) return -1;

    BookCopy copy;
    int rented_id = -1;
    while (file.read(reinterpret_cast<char*>(&copy), sizeof(BookCopy))) {
        if (copy.comic_id == comic_id && !copy.is_rented && copy.condition_status != 3) {
            copy.is_rented = true;
            file.seekp(-static_cast<std::streamoff>(sizeof(BookCopy)), std::ios::cur);
            file.write(reinterpret_cast<const char*>(&copy), sizeof(BookCopy));
            rented_id = copy.copy_id;
            break;
        }
    }
    file.close();
    return rented_id;
}

bool return_copy(int copy_id, int condition_status) {
    std::fstream file(BOOK_COPIES_FILE, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) return false;

    BookCopy copy;
    bool found = false;
    while (file.read(reinterpret_cast<char*>(&copy), sizeof(BookCopy))) {
        if (copy.copy_id == copy_id) {
            copy.is_rented = false;
            // condition_status from rental return logic:
            // 1: Trả nguyên vẹn (condition_status = 0: Mới)
            // 2: Rách nhẹ (condition_status = 1: Rách nhẹ)
            // 3: Mất trang (condition_status = 2: Mất trang)
            // 4: Mất hoàn toàn (condition_status = 3: Mất hoàn toàn)
            if (condition_status == 1) {
                // Giữ nguyên hoặc phục hồi thành mới
                copy.condition_status = 0;
            } else if (condition_status == 2) {
                copy.condition_status = 1;
            } else if (condition_status == 3) {
                copy.condition_status = 2;
            } else if (condition_status == 4) {
                copy.condition_status = 3;
            }
            file.seekp(-static_cast<std::streamoff>(sizeof(BookCopy)), std::ios::cur);
            file.write(reinterpret_cast<const char*>(&copy), sizeof(BookCopy));
            found = true;
            break;
        }
    }
    file.close();
    return found;
}

void initialize_book_copies_if_empty() {
    std::vector<BookCopy> copies = read_all_book_copies();
    if (!copies.empty()) return; // Đã khởi tạo

    std::vector<Comic> comics = read_all_comics();
    for (const auto& c : comics) {
        if (!c.is_deleted) {
            for (int i = 0; i < c.total_quantity; ++i) {
                BookCopy copy;
                copy.copy_id = get_next_copy_id();
                copy.comic_id = c.id;
                copy.condition_status = 0;
                copy.is_rented = (i < (c.total_quantity - c.quantity));
                save_book_copy(copy);
            }
        }
    }
}

bool adjust_copies_for_comic(int comic_id, int old_total, int new_total) {
    if (new_total > old_total) {
        return add_copies_for_comic(comic_id, new_total - old_total);
    } else if (new_total < old_total) {
        std::vector<BookCopy> copies = read_all_book_copies();
        int to_remove = old_total - new_total;
        int removed_count = 0;
        for (auto& c : copies) {
            if (c.comic_id == comic_id && !c.is_rented && c.condition_status != 3) {
                c.condition_status = 3; // Đánh dấu đã hủy/mất
                update_book_copy(c);
                removed_count++;
                if (removed_count >= to_remove) break;
            }
        }
        return true;
    }
    return true;
}
