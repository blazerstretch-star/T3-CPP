#include "../src/ds_algorithms.h"
#include <cassert>
#include <set>

int main() {
    auto is_sorted = [](int arr[], int size) {
        for (int i = 0; i < size - 1; i++) {
            if (arr[i] > arr[i+1]) return false;
        }
        return true;
    };
    
    auto is_permutation = [](int arr[], int original[], int size) {
        std::multiset<int> a(arr, arr + size);
        std::multiset<int> b(original, original + size);
        return a == b;
    };
    
    { int arr[] = {}; Algorithms::quickSort(arr, 0, -1); assert(is_sorted(arr, 0)); }
    { int arr[] = {42}; int orig[] = {42}; Algorithms::quickSort(arr, 0, 0); assert(is_sorted(arr, 1) && is_permutation(arr, orig, 1)); }
    { int arr[] = {1, 2}; int orig[] = {1, 2}; Algorithms::quickSort(arr, 0, 1); assert(is_sorted(arr, 2) && is_permutation(arr, orig, 2)); }
    { int arr[] = {2, 1}; int orig[] = {2, 1}; Algorithms::quickSort(arr, 0, 1); assert(is_sorted(arr, 2) && is_permutation(arr, orig, 2)); }
    { int arr[] = {5, 2, 8, 1, 9}; int orig[] = {5, 2, 8, 1, 9}; Algorithms::quickSort(arr, 0, 4); assert(is_sorted(arr, 5) && is_permutation(arr, orig, 5)); }
    { int arr[] = {1, 2, 3, 4, 5}; int orig[] = {1, 2, 3, 4, 5}; Algorithms::quickSort(arr, 0, 4); assert(is_sorted(arr, 5) && is_permutation(arr, orig, 5)); }
    { int arr[] = {5, 4, 3, 2, 1}; int orig[] = {5, 4, 3, 2, 1}; Algorithms::quickSort(arr, 0, 4); assert(is_sorted(arr, 5) && is_permutation(arr, orig, 5)); }
    { int arr[] = {7, 7, 7, 7}; int orig[] = {7, 7, 7, 7}; Algorithms::quickSort(arr, 0, 3); assert(is_sorted(arr, 4) && is_permutation(arr, orig, 4)); }
    { int arr[] = {3, 1, 4, 1, 5}; int orig[] = {3, 1, 4, 1, 5}; Algorithms::quickSort(arr, 0, 4); assert(is_sorted(arr, 5) && is_permutation(arr, orig, 5)); }
    { int arr[] = {-5, -2, -8}; int orig[] = {-5, -2, -8}; Algorithms::quickSort(arr, 0, 2); assert(is_sorted(arr, 3) && is_permutation(arr, orig, 3)); }
    { int arr[] = {-3, 5, -1, 0, 2}; int orig[] = {-3, 5, -1, 0, 2}; Algorithms::quickSort(arr, 0, 4); assert(is_sorted(arr, 5) && is_permutation(arr, orig, 5)); }
    {
        const int size = 1000;
        int* arr = new int[size];
        int* orig = new int[size];
        for (int i = 0; i < size; i++) arr[i] = orig[i] = (i * 7 + 13) % 1000;
        Algorithms::quickSort(arr, 0, size - 1);
        assert(is_sorted(arr, size) && is_permutation(arr, orig, size));
        delete[] arr; delete[] orig;
    }
    
    return 0;
}
