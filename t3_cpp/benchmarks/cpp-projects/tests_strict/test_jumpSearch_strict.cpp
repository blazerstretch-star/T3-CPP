#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    auto verify_found = [](const int arr[], int size, int target, int result) {
        if (result != -1) {
            assert(result >= 0 && result < size && "Index in bounds");
            assert(arr[result] == target && "Found correct element");
        } else {
            bool exists = false;
            for (int i = 0; i < size; i++) {
                if (arr[i] == target) exists = true;
            }
            assert(!exists && "Target should not exist");
        }
    };
    
    { const int arr[] = {}; int result = Algorithms::jumpSearch(arr, 0, 42); assert(result == -1); }
    { const int arr[] = {42}; int result = Algorithms::jumpSearch(arr, 1, 42); verify_found(arr, 1, 42, result); assert(result == 0); }
    { const int arr[] = {42}; int result = Algorithms::jumpSearch(arr, 1, 99); verify_found(arr, 1, 99, result); assert(result == -1); }
    { const int arr[] = {1, 2, 3, 4, 5}; int result = Algorithms::jumpSearch(arr, 5, 1); verify_found(arr, 5, 1, result); assert(result == 0); }
    { const int arr[] = {1, 2, 3, 4, 5}; int result = Algorithms::jumpSearch(arr, 5, 5); verify_found(arr, 5, 5, result); assert(result == 4); }
    { const int arr[] = {1, 2, 3, 4, 5}; int result = Algorithms::jumpSearch(arr, 5, 3); verify_found(arr, 5, 3, result); assert(result == 2); }
    { const int arr[] = {1, 2, 3, 4, 5}; int result = Algorithms::jumpSearch(arr, 5, 0); verify_found(arr, 5, 0, result); assert(result == -1); }
    { const int arr[] = {1, 2, 3, 4, 5}; int result = Algorithms::jumpSearch(arr, 5, 99); verify_found(arr, 5, 99, result); assert(result == -1); }
    { const int arr[] = {1, 3, 5, 7, 9}; int result = Algorithms::jumpSearch(arr, 5, 4); verify_found(arr, 5, 4, result); assert(result == -1); }
    {
        const int size = 1000;
        int* arr = new int[size];
        for (int i = 0; i < size; i++) arr[i] = i * 2;
        int result = Algorithms::jumpSearch(arr, size, 500);
        verify_found(arr, size, 500, result);
        assert(result == 250);
        delete[] arr;
    }
    
    return 0;
}
