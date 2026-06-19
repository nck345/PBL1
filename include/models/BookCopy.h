#ifndef BOOK_COPY_H
#define BOOK_COPY_H

struct BookCopy {
  int copy_id;          // Unique ID of this book copy
  int comic_id;         // Comic catalog ID
  int condition_status; // 0: Mới, 1: Rách nhẹ, 2: Mất trang, 3: Đã mất hoàn toàn
  bool is_rented;       // True if currently rented
};

#endif
