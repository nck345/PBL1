#include "../../include/repository/ComicRepo.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>

const char* COMICS_FILE = "data/comics.dat";
const char* METADATA_FILE = "data/metadata.dat";

std::string to_lower_copy(const std::string& text) {
    std::string lower = text;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return lower;
}

int get_next_comic_id() {
    int id = 1;
    std::ifstream inFile(METADATA_FILE, std::ios::binary);
    if (inFile.is_open()) {
        inFile.read(reinterpret_cast<char*>(&id), sizeof(int));
        inFile.close();
    }
    
    int next_id = id + 1;
    std::ofstream outFile(METADATA_FILE, std::ios::binary);
    if (outFile.is_open()) {
        outFile.write(reinterpret_cast<const char*>(&next_id), sizeof(int));
        outFile.close();
    }
    
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

    std::string query = to_lower_copy(name);

    Comic comic;
    while (file.read(reinterpret_cast<char*>(&comic), sizeof(Comic))) {
        if (comic.is_deleted) continue;

        std::string comic_name = to_lower_copy(comic.comic_name);

        if (comic_name.find(query) != std::string::npos) {
            results.push_back(comic);
        }
    }
    file.close();
    return results;
}

std::vector<Comic> search_comics_by_type(const std::string& type_keyword) {
    std::vector<Comic> results;
    std::ifstream file(COMICS_FILE, std::ios::binary);
    if (!file) return results;

    std::string query = to_lower_copy(type_keyword);

    Comic comic;
    while (file.read(reinterpret_cast<char*>(&comic), sizeof(Comic))) {
        if (comic.is_deleted) continue;

        std::string comic_type = to_lower_copy(comic.type);
        if (comic_type.find(query) != std::string::npos) {
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

bool is_comic_duplicate(const char* name, const char* author, const char* type) {
    std::vector<Comic> all_comics = read_all_comics();
    std::string kw_name = to_lower_copy(name);
    std::string kw_author = to_lower_copy(author);
    std::string kw_type = to_lower_copy(type);

    for (const auto& c : all_comics) {
        if (!c.is_deleted) {
            std::string c_name = to_lower_copy(c.comic_name);
            std::string c_author = to_lower_copy(c.author);
            std::string c_type = to_lower_copy(c.type);
            if (c_name == kw_name && c_author == kw_author && c_type == kw_type) {
                return true;
            }
        }
    }
    return false;
}

bool compare_comic_by_id_asc(const Comic& a, const Comic& b) { return a.id < b.id; }
bool compare_comic_by_id_desc(const Comic& a, const Comic& b) { return a.id > b.id; }
bool compare_comic_by_name_asc(const Comic& a, const Comic& b) {
    std::string name_a = to_lower_copy(a.comic_name);
    std::string name_b = to_lower_copy(b.comic_name);
    return name_a < name_b;
}
bool compare_comic_by_name_desc(const Comic& a, const Comic& b) {
    std::string name_a = to_lower_copy(a.comic_name);
    std::string name_b = to_lower_copy(b.comic_name);
    return name_a > name_b;
}
bool compare_comic_by_type_asc(const Comic& a, const Comic& b) {
    std::string type_a = to_lower_copy(a.type);
    std::string type_b = to_lower_copy(b.type);
    return type_a < type_b;
}
bool compare_comic_by_type_desc(const Comic& a, const Comic& b) {
    std::string type_a = to_lower_copy(a.type);
    std::string type_b = to_lower_copy(b.type);
    return type_a > type_b;
}
bool compare_comic_by_price_asc(const Comic& a, const Comic& b) { return a.price < b.price; }
bool compare_comic_by_price_desc(const Comic& a, const Comic& b) { return a.price > b.price; }

int search_cmp_comic_id(const Comic& c, const int& key) {
    if (c.id == key) return 0;
    return c.id < key ? -1 : 1;
}
int search_cmp_comic_name(const Comic& c, const std::string& key) {
    std::string c_name = to_lower_copy(c.comic_name);
    std::string k_name = to_lower_copy(key);
    if (c_name == k_name) return 0;
    return c_name < k_name ? -1 : 1;
}
int search_cmp_comic_price(const Comic& c, const double& key) {
    if (c.price == key) return 0;
    return c.price < key ? -1 : 1;
}
