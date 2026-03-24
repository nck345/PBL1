#ifndef COMIC_REPO_H
#define COMIC_REPO_H

#include "../../include/models/Comic.h"
#include <vector>
#include <string>

// Header for Comic Repository handling data operations on comics.dat

// Read/write metadata.dat to get auto-incrementing ID
int get_next_comic_id();

// Read all comics from comics.dat
std::vector<Comic> read_all_comics();

// Add a new comic to comics.dat (auto-generates ID)
void add_comic(Comic& comic);

// Update an existing comic's info in comics.dat
bool update_comic(const Comic& updated_comic);

// Soft delete a comic by marking is_deleted = true in comics.dat
bool delete_comic(int id);

// Search for comics by name (case-insensitive)
std::vector<Comic> search_comics_by_name(const std::string& name);

// Retrieve a specific comic by ID
bool get_comic_by_id(int id, Comic& out_comic);

#endif
