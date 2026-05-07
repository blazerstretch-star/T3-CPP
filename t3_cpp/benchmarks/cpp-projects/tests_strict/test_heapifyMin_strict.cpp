#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    auto is_min_heap_at = [](int arr[], int n, int root) {
        int left = 2 * root + 1;
        int right = 2 * root + 2;
        if (left < n && arr[root] > arr[left]) return false;
        if (right < n && arr[root] > arr[right]) return false;
        return true;
    };
    
    // Test 1: Single element
    { int arr[] = {5}; Algorithms::heapifyMin(arr, 1, 0); assert(is_min_heap_at(arr, 1, 0)); }
    
    // Test 2: Root with left child only
    { int arr[] = {5, 3}; Algorithms::heapifyMin(arr, 2, 0); assert(is_min_heap_at(arr, 2, 0)); }
    
    // Test 3: Root with two children
    { int arr[] = {5, 3, 4}; Algorithms::heapifyMin(arr, 3, 0); assert(is_min_heap_at(arr, 3, 0)); }
    
    // Test 4: Already min heap
    { int arr[] = {1, 5, 4}; Algorithms::heapifyMin(arr, 3, 0); assert(is_min_heap_at(arr, 3, 0)); assert(arr[0] == 1); }
    
    // Test 5: Needs swap with left
    { int arr[] = {9, 3, 4}; Algorithms::heapifyMin(arr, 3, 0); assert(is_min_heap_at(arr, 3, 0)); }
    
    // Test 6: Needs swap with right
    { int arr[] = {9, 4, 3}; Algorithms::heapifyMin(arr, 3, 0); assert(is_min_heap_at(arr, 3, 0)); }
    
    // Test 7: Larger heap
    { int arr[] = {9, 1, 2, 5, 6, 7, 4}; Algorithms::heapifyMin(arr, 7, 0); assert(is_min_heap_at(arr, 7, 0)); }
    
    // Test 8: All equal
    { int arr[] = {5, 5, 5, 5}; Algorithms::heapifyMin(arr, 4, 0); assert(is_min_heap_at(arr, 4, 0)); }
    
    return 0;
}
