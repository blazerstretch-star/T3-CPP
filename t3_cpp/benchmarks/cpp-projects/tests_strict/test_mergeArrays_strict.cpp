#include "../src/ds_algorithms.h"
#include <cassert>
#include <set>

int main() {
    auto is_sorted = [](int arr[], int left, int right) {
        for (int i = left; i < right; i++) {
            if (arr[i] > arr[i+1]) return false;
        }
        return true;
    };
    
    auto is_permutation = [](int arr[], int original[], int left, int right) {
        std::multiset<int> a(arr + left, arr + right + 1);
        std::multiset<int> b(original + left, original + right + 1);
        return a == b;
    };
    
    // Test 1: Merge two single elements
    {
        int arr[] = {2, 1};
        int orig[] = {2, 1};
        Algorithms::mergeArrays(arr, 0, 0, 1);
        assert(is_sorted(arr, 0, 1) && is_permutation(arr, orig, 0, 1));
    }
    
    // Test 2: Merge already sorted
    {
        int arr[] = {1, 2, 3, 4};
        int orig[] = {1, 2, 3, 4};
        Algorithms::mergeArrays(arr, 0, 1, 3);
        assert(is_sorted(arr, 0, 3) && is_permutation(arr, orig, 0, 3));
    }
    
    // Test 3: Merge reverse sorted halves
    {
        int arr[] = {3, 4, 1, 2};
        int orig[] = {3, 4, 1, 2};
        Algorithms::mergeArrays(arr, 0, 1, 3);
        assert(is_sorted(arr, 0, 3) && is_permutation(arr, orig, 0, 3));
    }
    
    // Test 4: Merge with duplicates
    {
        int arr[] = {1, 3, 2, 3};
        int orig[] = {1, 3, 2, 3};
        Algorithms::mergeArrays(arr, 0, 1, 3);
        assert(is_sorted(arr, 0, 3) && is_permutation(arr, orig, 0, 3));
    }
    
    // Test 5: Merge unequal sizes
    {
        int arr[] = {1, 5, 2, 3, 4};
        int orig[] = {1, 5, 2, 3, 4};
        Algorithms::mergeArrays(arr, 0, 1, 4);
        assert(is_sorted(arr, 0, 4) && is_permutation(arr, orig, 0, 4));
    }
    
    // Test 6: Merge with negative numbers
    {
        int arr[] = {-5, -1, -3, 0};
        int orig[] = {-5, -1, -3, 0};
        Algorithms::mergeArrays(arr, 0, 1, 3);
        assert(is_sorted(arr, 0, 3) && is_permutation(arr, orig, 0, 3));
    }
    
    // Test 7: Merge all equal
    {
        int arr[] = {5, 5, 5, 5};
        int orig[] = {5, 5, 5, 5};
        Algorithms::mergeArrays(arr, 0, 1, 3);
        assert(is_sorted(arr, 0, 3) && is_permutation(arr, orig, 0, 3));
    }
    
    // Test 8: Large merge
    {
        const int size = 1000;
        int* arr = new int[size];
        int* orig = new int[size];
        for (int i = 0; i < size/2; i++) arr[i] = orig[i] = i * 2;
        for (int i = size/2; i < size; i++) arr[i] = orig[i] = (i - size/2) * 2 + 1;
        Algorithms::mergeArrays(arr, 0, size/2 - 1, size - 1);
        assert(is_sorted(arr, 0, size - 1) && is_permutation(arr, orig, 0, size - 1));
        delete[] arr; delete[] orig;
    }
    
    return 0;
}
