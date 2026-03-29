#ifndef SORT_UTILS_H
#define SORT_UTILS_H

#include <vector>

// Hàm sắp xếp tổng quát
template <typename T>
void quick_sort(std::vector<T>& arr, bool (*compare)(const T&, const T&));

#endif
