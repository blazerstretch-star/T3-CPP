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
    
    // Test 1: Empty
    { int arr[] = {}; Algorithms::insertionSort(arr, 0); assert(is_sorted(arr, 0)); }
    
    // Test 2: Single
    { int arr[] = {42}; int orig[] = {42}; Algorithms::insertionSort(arr, 1); 
      assert(is_sorted(arr, 1) && is_permutation(arr, orig, 1)); }
    
    // Test 3: Two ascending
    { int arr[] = {1, 2}; int orig[] = {1, 2}; Algorithms::insertionSort(arr, 2); 
      assert(is_sorted(arr, 2) && is_permutation(arr, orig, 2)); }
    
    // Test 4: Two descending
    { int arr[] = {2, 1}; int orig[] = {2, 1}; Algorithms::insertionSort(arr, 2); 
      assert(is_sorted(arr, 2) && is_permutation(arr, orig, 2)); }
    
    // Test 5: Normal
    { int arr[] = {5, 2, 8, 1, 9}; int orig[] = {5, 2, 8, 1, 9}; Algorithms::insertionSort(arr, 5); 
      assert(is_sorted(arr, 5) && is_permutation(arr, orig, 5)); }
    
    // Test 6: Already sorted
    { int arr[] = {1, 2, 3, 4, 5}; int orig[] = {1, 2, 3, 4, 5}; Algorithms::insertionSort(arr, 5); 
      assert(is_sorted(arr, 5) && is_permutation(arr, orig, 5)); }
    
    // Test 7: Reverse sorted
    { int arr[] = {5, 4, 3, 2, 1}; int orig[] = {5, 4, 3, 2, 1}; Algorithms::insertionSort(arr, 5); 
      assert(is_sorted(arr, 5) && is_permutation(arr, orig, 5)); }
    
    // Test 8: All equal
    { int arr[] = {7, 7, 7, 7}; int orig[] = {7, 7, 7, 7}; Algorithms::insertionSort(arr, 4); 
      assert(is_sorted(arr, 4) && is_permutation(arr, orig, 4)); }
    
    // Test 9: Duplicates
    { int arr[] = {3, 1, 4, 1, 5}; int orig[] = {3, 1, 4, 1, 5}; Algorithms::insertionSort(arr, 5); 
      assert(is_sorted(arr, 5) && is_permutation(arr, orig, 5)); }
    
    // Test 10: Negative
    { int arr[] = {-5, -2, -8}; int orig[] = {-5, -2, -8}; Algorithms::insertionSort(arr, 3); 
      assert(is_sorted(arr, 3) && is_permutation(arr, orig, 3)); }
    
    // Test 11: Mixed signs
    { int arr[] = {-3, 5, -1, 0, 2}; int orig[] = {-3, 5, -1, 0, 2}; Algorithms::insertionSort(arr, 5); 
      assert(is_sorted(arr, 5) && is_permutation(arr, orig, 5)); }
    
    // Test 12: Large
    {
        const int size = 1000;
        int* arr = new int[size];
        int* orig = new int[size];
        for (int i = 0; i < size; i++) arr[i] = orig[i] = (i * 7 + 13) % 1000;
        Algorithms::insertionSort(arr, size);
        assert(is_sorted(arr, size) && is_permutation(arr, orig, size));
        delete[] arr; delete[] orig;
    }
    
    return 0;
}
