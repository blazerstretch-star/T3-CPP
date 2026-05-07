#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    auto is_min_heap = [](int arr[], int size) {
        for (int i = 0; i < size; i++) {
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            if (left < size && arr[i] > arr[left]) return false;
            if (right < size && arr[i] > arr[right]) return false;
        }
        return true;
    };
    
    // Test 1: Empty
    { int arr[] = {}; Algorithms::buildMinHeap(arr, 0); assert(is_min_heap(arr, 0)); }
    
    // Test 2: Single
    { int arr[] = {5}; Algorithms::buildMinHeap(arr, 1); assert(is_min_heap(arr, 1)); }
    
    // Test 3: Two elements
    { int arr[] = {5, 3}; Algorithms::buildMinHeap(arr, 2); assert(is_min_heap(arr, 2)); }
    
    // Test 4: Three elements
    { int arr[] = {5, 3, 4}; Algorithms::buildMinHeap(arr, 3); assert(is_min_heap(arr, 3)); }
    
    // Test 5: Already min heap
    { int arr[] = {1, 2, 3, 4, 5}; Algorithms::buildMinHeap(arr, 5); assert(is_min_heap(arr, 5)); }
    
    // Test 6: Reverse sorted
    { int arr[] = {5, 4, 3, 2, 1}; Algorithms::buildMinHeap(arr, 5); assert(is_min_heap(arr, 5)); }
    
    // Test 7: Random order
    { int arr[] = {3, 1, 4, 1, 5, 9, 2, 6}; Algorithms::buildMinHeap(arr, 8); assert(is_min_heap(arr, 8)); }
    
    // Test 8: All equal
    { int arr[] = {5, 5, 5, 5, 5}; Algorithms::buildMinHeap(arr, 5); assert(is_min_heap(arr, 5)); }
    
    // Test 9: Large
    {
        int arr[100];
        for (int i = 0; i < 100; i++) arr[i] = 99 - i;
        Algorithms::buildMinHeap(arr, 100);
        assert(is_min_heap(arr, 100));
    }
    
    return 0;
}
