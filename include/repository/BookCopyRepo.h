#ifndef BOOK_COPY_REPO_H
#define BOOK_COPY_REPO_H

#include "../models/BookCopy.h"
#include <vector>

int get_next_copy_id();
std::vector<BookCopy> read_all_book_copies();
bool save_book_copy(const BookCopy& copy);
bool update_book_copy(const BookCopy& updated_copy);
bool add_copies_for_comic(int comic_id, int count);
int rent_available_copy(int comic_id);
bool return_copy(int copy_id, int condition_status);
bool adjust_copies_for_comic(int comic_id, int old_total, int new_total);
void initialize_book_copies_if_empty();

#endif
