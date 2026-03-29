#ifndef SEARCH_UTILS_H
#define SEARCH_UTILS_H

#include <vector>

// Hàm tìm kiếm nhị phân tổng quát
// compare(a, key): trả về 0 nếu bằng, <0 nếu a < key, >0 nếu a > key
template <typename T, typename KeyType>
int binary_search_idx(const std::vector<T>& arr, const KeyType& key, int (*compare)(const T&, const KeyType&));

#endif
