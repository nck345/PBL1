#ifndef COMIC_UI_H
#define COMIC_UI_H

#include "../../include/models/Comic.h"
#include <vector>

void render_comic_table(const std::vector<Comic>& comics);
void render_comic_menu();

#endif
