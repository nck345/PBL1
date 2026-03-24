#include "../../include/repository/ComicRepo.h"
#include <fstream>
#include <iostream>
#include <algorithm>

const char* COMICS_FILE = "data/comics.dat";
const char* METADATA_FILE = "data/metadata.dat";

int get_next_comic_id() {
    int id = 1;
    std::fstream file(METADATA_FILE, std::ios::in | std::ios::out | std::ios::binary);
    
    // Nếu file chưa tồn tại, tạo mới với ID bắt đầu = 1
    if (!file) {
        std::ofstream out_file(METADATA_FILE, std::ios::binary);
        out_file.write(reinterpret_cast<const char*>(&id), sizeof(int));
        out_file.close();
        return id;
    }

    // Đọc ID hiện tại
    file.read(reinterpret_cast<char*>(&id), sizeof(int));
    
    // Tăng ID lên 1 để chuẩn bị cho lần lấy tiếp theo
    int next_id = id + 1;
    file.seekp(0, std::ios::beg);
    file.write(reinterpret_cast<const char*>(&next_id), sizeof(int));
    file.close();
    
    return id;
}

std::vector<Comic> read_all_comics() {
    std::vector<Comic> comics;
    std::ifstream file(COMICS_FILE, std::ios::binary);
    if (!file) return comics; // Trả về mảng rỗng nếu chưa có file

    Comic comic;
    while (file.read(reinterpret_cast<char*>(&comic), sizeof(Comic))) {
        comics.push_back(comic);
    }
    file.close();
    return comics;
}

void add_comic(Comic& comic) {
    comic.id = get_next_comic_id(); // Lấy ID tự động tăng
    comic.is_deleted = false;
    
    std::ofstream file(COMICS_FILE, std::ios::app | std::ios::binary);
    if (file) {
        file.write(reinterpret_cast<const char*>(&comic), sizeof(Comic));
        file.close();
    } else {
        std::cerr << "Loi: Khong the mo file " << COMICS_FILE << " de ghi.\n";
    }
}

bool update_comic(const Comic& updated_comic) {
    std::fstream file(COMICS_FILE, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) return false;

    Comic comic;
    bool found = false;
    while (file.read(reinterpret_cast<char*>(&comic), sizeof(Comic))) {
        if (comic.id == updated_comic.id && !comic.is_deleted) {
            // Nhảy lùi file pointer lại đúng block sizeof(Comic) vừa đọc qua để ghi đè
            file.seekp(-static_cast<std::streamoff>(sizeof(Comic)), std::ios::cur);
            file.write(reinterpret_cast<const char*>(&updated_comic), sizeof(Comic));
            found = true;
            break;
        }
    }
    file.close();
    return found;
}

bool delete_comic(int id) {
    std::fstream file(COMICS_FILE, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) return false;

    Comic comic;
    bool found = false;
    while (file.read(reinterpret_cast<char*>(&comic), sizeof(Comic))) {
        if (comic.id == id && !comic.is_deleted) {
            comic.is_deleted = true; // Chuyển cờ thành true, không xóa vật lý
            file.seekp(-static_cast<std::streamoff>(sizeof(Comic)), std::ios::cur);
            file.write(reinterpret_cast<const char*>(&comic), sizeof(Comic));
            found = true;
            break;
        }
    }
    file.close();
    return found;
}

std::vector<Comic> search_comics_by_name(const std::string& name) {
    std::vector<Comic> results;
    std::ifstream file(COMICS_FILE, std::ios::binary);
    if (!file) return results;

    // Chuyển query sang in thường
    std::string query = name;
    std::transform(query.begin(), query.end(), query.begin(), ::tolower);

    Comic comic;
    while (file.read(reinterpret_cast<char*>(&comic), sizeof(Comic))) {
        if (comic.is_deleted) continue;

        std::string comic_name = comic.comic_name;
        std::transform(comic_name.begin(), comic_name.end(), comic_name.begin(), ::tolower);

        if (comic_name.find(query) != std::string::npos) {
            results.push_back(comic);
        }
    }
    file.close();
    return results;
}

bool get_comic_by_id(int id, Comic& out_comic) {
    std::ifstream file(COMICS_FILE, std::ios::binary);
    if (!file) return false;

    Comic comic;
    while (file.read(reinterpret_cast<char*>(&comic), sizeof(Comic))) {
        if (comic.id == id && !comic.is_deleted) {
            out_comic = comic;
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}
