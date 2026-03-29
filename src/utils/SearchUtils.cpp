#include "../../include/utils/SearchUtils.h"
#include "../../include/models/Comic.h"
#include <vector>
#include <string>

template <typename T, typename KeyType>
int binary_search_idx(const std::vector<T>& arr, const KeyType& key, int (*compare)(const T&, const KeyType&)) {
    int left = 0;
    int right = arr.size() - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = compare(arr[mid], key);

        if (cmp == 0) {
            return mid; // Tìm thấy
        }
        if (cmp < 0) {
            left = mid + 1; // arr[mid] < key
        } else {
            right = mid - 1; // arr[mid] > key
        }
    }
    return -1; // Không tìm thấy
}

// Explicit instantiation cho Comic
template int binary_search_idx<Comic, int>(const std::vector<Comic>&, const int&, int (*)(const Comic&, const int&));
template int binary_search_idx<Comic, std::string>(const std::vector<Comic>&, const std::string&, int (*)(const Comic&, const std::string&));
template int binary_search_idx<Comic, double>(const std::vector<Comic>&, const double&, int (*)(const Comic&, const double&));
