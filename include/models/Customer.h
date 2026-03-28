#ifndef CUSTOMER_H
#define CUSTOMER_H

struct Customer {
  int id;
  char name[100];
  char phone[20];
  bool is_deleted;
};

#endif
