#include "../../include/utils/SortUtils.h"
#include "../../include/models/Comic.h"
#include "../../include/models/RentalSlip.h"
#include "../../include/models/Customer.h"
#include <vector>
#include <algorithm> // for std::swap

template <typename T>
int partition(std::vector<T>& arr, int low, int high, bool (*compare)(const T&, const T&)) {
    T pivot = arr[high];
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++) {
        if (compare(arr[j], pivot)) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    return (i + 1);
}

template <typename T>
void quick_sort_recursive(std::vector<T>& arr, int low, int high, bool (*compare)(const T&, const T&)) {
    if (low < high) {
        int pi = partition(arr, low, high, compare);
        quick_sort_recursive(arr, low, pi - 1, compare);
        quick_sort_recursive(arr, pi + 1, high, compare);
    }
}

template <typename T>
void quick_sort(std::vector<T>& arr, bool (*compare)(const T&, const T&)) {
    if (!arr.empty()) {
        quick_sort_recursive(arr, 0, arr.size() - 1, compare);
    }
}

// Explicit Instantiation cho Comic
template void quick_sort<Comic>(std::vector<Comic>&, bool (*)(const Comic&, const Comic&));
// Phép khai báo thêm để khỏi bị lỗi link cho bảng khác nếu Nhu Y dung (tuỳ chọn)
template void quick_sort<RentalSlip>(std::vector<RentalSlip>&, bool (*)(const RentalSlip&, const RentalSlip&));
template void quick_sort<Customer>(std::vector<Customer>&, bool (*)(const Customer&, const Customer&));
