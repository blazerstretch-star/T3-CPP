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
    
    // Test 1: Empty array
    { const int arr[] = {}; int result = Algorithms::binarySearchRecursive(arr, 0, -1, 42); assert(result == -1); }
    
    // Test 2: Single element - found
    { const int arr[] = {42}; int result = Algorithms::binarySearchRecursive(arr, 0, 0, 42); verify_found(arr, 1, 42, result); assert(result == 0); }
    
    // Test 3: Single element - not found
    { const int arr[] = {42}; int result = Algorithms::binarySearchRecursive(arr, 0, 0, 99); verify_found(arr, 1, 99, result); assert(result == -1); }
    
    // Test 4: Found at beginning
    { const int arr[] = {1, 2, 3, 4, 5}; int result = Algorithms::binarySearchRecursive(arr, 0, 4, 1); verify_found(arr, 5, 1, result); assert(result == 0); }
    
    // Test 5: Found at end
    { const int arr[] = {1, 2, 3, 4, 5}; int result = Algorithms::binarySearchRecursive(arr, 0, 4, 5); verify_found(arr, 5, 5, result); assert(result == 4); }
    
    // Test 6: Found in middle
    { const int arr[] = {1, 2, 3, 4, 5}; int result = Algorithms::binarySearchRecursive(arr, 0, 4, 3); verify_found(arr, 5, 3, result); assert(result == 2); }
    
    // Test 7: Not found - too small
    { const int arr[] = {1, 2, 3, 4, 5}; int result = Algorithms::binarySearchRecursive(arr, 0, 4, 0); verify_found(arr, 5, 0, result); assert(result == -1); }
    
    // Test 8: Not found - too large
    { const int arr[] = {1, 2, 3, 4, 5}; int result = Algorithms::binarySearchRecursive(arr, 0, 4, 99); verify_found(arr, 5, 99, result); assert(result == -1); }
    
    // Test 9: Not found - in gap
    { const int arr[] = {1, 3, 5, 7, 9}; int result = Algorithms::binarySearchRecursive(arr, 0, 4, 4); verify_found(arr, 5, 4, result); assert(result == -1); }
    
    // Test 10: Large array
    {
        const int size = 1000;
        int* arr = new int[size];
        for (int i = 0; i < size; i++) arr[i] = i * 2;
        int result = Algorithms::binarySearchRecursive(arr, 0, size - 1, 500);
        verify_found(arr, size, 500, result);
        assert(result == 250);
        delete[] arr;
    }
    
    return 0;
}
