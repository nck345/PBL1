#ifndef CUSTOMER_H
#define CUSTOMER_H

// Struct khách hàng (thuần túy - không dùng OOP/Class)
struct Customer {
    int id;
    char name[100];
    char phone[20];   // Số điện thoại - dùng để định danh duy nhất
    bool is_deleted;  // Soft delete
};

#endif
