#ifndef COMIC_UI_H
#define COMIC_UI_H

#include "../../include/models/Comic.h"
#include <vector>
#include <string>

void render_comic_menu();
int select_comic_ui(const std::string& title);

std::string to_lower_text(const std::string& text);
std::vector<std::string> build_type_options(const std::vector<Comic>& active_comics);
void refresh_type_suggestions(const std::vector<std::string>& type_options,
                              const std::string& query,
                              std::vector<std::string>& filtered_options,
                              int& selected_index);
bool is_valid_type_suggestion(const std::string& value);

#endif
