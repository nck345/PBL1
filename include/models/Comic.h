#ifndef COMIC_H
#define COMIC_H

struct Comic {
  int id;
  char comic_name[100]; // using char array since file I/O usually prefers fixed size for binary
  char author[50];
  char type[50];
  double price;
  int quantity;
  bool is_deleted;
};

#endif
