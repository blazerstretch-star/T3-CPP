#include "../src/movie_functions.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <climits>
#include <set>

using namespace MovieRecommendation;

// PROPERTY 1: Array is sorted (implementation-agnostic - works for any order)
bool is_sorted_any_order(int* arr, int n) {
    if (n <= 1) return true;
    
    // Check if ascending
    bool ascending = true;
    for (int i = 0; i < n-1; i++) {
        if (arr[i] > arr[i+1]) {
            ascending = false;
            break;
        }
    }
    if (ascending) return true;
    
    // Check if descending
    bool descending = true;
    for (int i = 0; i < n-1; i++) {
        if (arr[i] < arr[i+1]) {
            descending = false;
            break;
        }
    }
    return descending;
}

// PROPERTY 2: All elements preserved (no data loss)
bool verify_elements_preserved(int* arr, int n, std::vector<int> original) {
    std::vector<int> sorted_arr(arr, arr + n);
    std::sort(sorted_arr.begin(), sorted_arr.end());
    std::sort(original.begin(), original.end());
    return sorted_arr == original;
}

// PROPERTY 3: Permutation of original (same multiset)
bool is_permutation(int* arr, int n, std::vector<int> original) {
    std::multiset<int> arr_set(arr, arr + n);
    std::multiset<int> orig_set(original.begin(), original.end());
    return arr_set == orig_set;
}

// PROPERTY 4: Stability check (for equal elements, relative order preserved)
// Note: This is optional - not all merge sorts are stable
bool check_stability_if_stable(int* arr, int n, std::vector<std::pair<int, int>>& indexed) {
    // This is informational only - we don't require stability
    return true;
}

int main() {
    // Test 1: Empty array (edge case)
    {
        int* empty = nullptr;
        merge_sort(empty, 0, -1); // Should not crash
        // No assertion needed - just shouldn't crash
    }
    
    // Test 2: Single element
    {
        int single[] = {42};
        std::vector<int> original = {42};
        
        merge_sort(single, 0, 0);
        
        assert(single[0] == 42 && "Single element unchanged");
        assert(verify_elements_preserved(single, 1, original));
    }
    
    // Test 3: Two elements (minimal interaction)
    {
        int two[] = {5, 10};
        std::vector<int> original = {5, 10};
        
        merge_sort(two, 0, 1);
        
        assert(is_sorted_any_order(two, 2) && "Two elements must be sorted");
        assert(verify_elements_preserved(two, 2, original));
    }
    
    // Test 4: Two elements reversed
    {
        int two[] = {10, 5};
        std::vector<int> original = {10, 5};
        
        merge_sort(two, 0, 1);
        
        assert(is_sorted_any_order(two, 2));
        assert(verify_elements_preserved(two, 2, original));
    }
    
    // Test 5: Already sorted (ascending)
    {
        int asc[] = {10, 20, 30, 40, 50};
        std::vector<int> original = {10, 20, 30, 40, 50};
        
        merge_sort(asc, 0, 4);
        
        assert(is_sorted_any_order(asc, 5));
        assert(verify_elements_preserved(asc, 5, original));
    }
    
    // Test 6: Already sorted (descending)
    {
        int desc[] = {90, 80, 70, 60, 50};
        std::vector<int> original = {90, 80, 70, 60, 50};
        
        merge_sort(desc, 0, 4);
        
        assert(is_sorted_any_order(desc, 5));
        assert(verify_elements_preserved(desc, 5, original));
    }
    
    // Test 7: All identical elements
    {
        int identical[] = {7, 7, 7, 7, 7, 7, 7};
        std::vector<int> original = {7, 7, 7, 7, 7, 7, 7};
        
        merge_sort(identical, 0, 6);
        
        for (int i = 0; i < 7; i++) {
            assert(identical[i] == 7);
        }
        assert(is_sorted_any_order(identical, 7));
        assert(verify_elements_preserved(identical, 7, original));
    }
    
    // Test 8: Random order
    {
        int random[] = {45, 12, 89, 23, 67, 34, 91, 5};
        std::vector<int> original = {45, 12, 89, 23, 67, 34, 91, 5};
        
        merge_sort(random, 0, 7);
        
        assert(is_sorted_any_order(random, 8));
        assert(verify_elements_preserved(random, 8, original));
    }
    
    // Test 9: Large array with random data
    {
        const int size = 1000;
        int* large = new int[size];
        std::vector<int> original;
        
        std::random_device rd;
        std::mt19937 gen(42); // Fixed seed for reproducibility
        std::uniform_int_distribution<> dis(1, 10000);
        
        for (int i = 0; i < size; i++) {
            large[i] = dis(gen);
            original.push_back(large[i]);
        }
        
        merge_sort(large, 0, size-1);
        
        assert(is_sorted_any_order(large, size));
        assert(verify_elements_preserved(large, size, original));
        assert(is_permutation(large, size, original));
        
        delete[] large;
    }
    
    // Test 10: Extreme values
    {
        int extreme[] = {INT_MAX, INT_MIN, 0, -1, 1, INT_MAX/2, INT_MIN/2};
        std::vector<int> original = {INT_MAX, INT_MIN, 0, -1, 1, INT_MAX/2, INT_MIN/2};
        
        merge_sort(extreme, 0, 6);
        
        assert(is_sorted_any_order(extreme, 7));
        assert(verify_elements_preserved(extreme, 7, original));
    }
    
    // Test 11: Duplicates mixed
    {
        int dups[] = {5, 3, 8, 3, 1, 8, 5, 3, 8};
        std::vector<int> original = {5, 3, 8, 3, 1, 8, 5, 3, 8};
        
        merge_sort(dups, 0, 8);
        
        assert(is_sorted_any_order(dups, 9));
        assert(verify_elements_preserved(dups, 9, original));
        assert(is_permutation(dups, 9, original));
    }
    
    // Test 12: Negative numbers only
    {
        int negative[] = {-10, -5, -100, -1, -50, -25};
        std::vector<int> original = {-10, -5, -100, -1, -50, -25};
        
        merge_sort(negative, 0, 5);
        
        assert(is_sorted_any_order(negative, 6));
        assert(verify_elements_preserved(negative, 6, original));
    }
    
    // Test 13: Partial sort (sort middle portion)
    {
        int partial[] = {100, 50, 30, 10, 20, 40, 200};
        std::vector<int> original_middle = {50, 30, 10, 20, 40};
        
        merge_sort(partial, 1, 5); // Sort indices 1-5
        
        assert(partial[0] == 100 && "First element unchanged");
        assert(partial[6] == 200 && "Last element unchanged");
        assert(is_sorted_any_order(partial + 1, 5) && "Middle portion sorted");
        
        std::vector<int> sorted_middle(partial + 1, partial + 6);
        assert(is_permutation(partial + 1, 5, original_middle));
    }
    
    // Test 14: Two-element patterns
    {
        int pattern[] = {2, 1, 4, 3, 6, 5, 8, 7};
        std::vector<int> original = {2, 1, 4, 3, 6, 5, 8, 7};
        
        merge_sort(pattern, 0, 7);
        
        assert(is_sorted_any_order(pattern, 8));
        assert(verify_elements_preserved(pattern, 8, original));
    }
    
    // Test 15: Very large array (stress test)
    {
        const int size = 10000;
        int* huge = new int[size];
        std::vector<int> original;
        
        std::mt19937 gen(123);
        std::uniform_int_distribution<> dis(-50000, 50000);
        
        for (int i = 0; i < size; i++) {
            huge[i] = dis(gen);
            original.push_back(huge[i]);
        }
        
        merge_sort(huge, 0, size-1);
        
        assert(is_sorted_any_order(huge, size));
        assert(is_permutation(huge, size, original));
        
        delete[] huge;
    }
    
    // Test 16: Sawtooth pattern
    {
        int sawtooth[] = {1, 5, 2, 6, 3, 7, 4, 8};
        std::vector<int> original = {1, 5, 2, 6, 3, 7, 4, 8};
        
        merge_sort(sawtooth, 0, 7);
        
        assert(is_sorted_any_order(sawtooth, 8));
        assert(verify_elements_preserved(sawtooth, 8, original));
    }
    
    // Test 17: Many duplicates
    {
        int many_dups[100];
        std::vector<int> original;
        
        for (int i = 0; i < 100; i++) {
            many_dups[i] = i % 10; // Only 10 unique values
            original.push_back(many_dups[i]);
        }
        
        merge_sort(many_dups, 0, 99);
        
        assert(is_sorted_any_order(many_dups, 100));
        assert(is_permutation(many_dups, 100, original));
    }
    
    // Test 18: Reverse sorted large array
    {
        int reverse[500];
        std::vector<int> original;
        
        for (int i = 0; i < 500; i++) {
            reverse[i] = 500 - i;
            original.push_back(reverse[i]);
        }
        
        merge_sort(reverse, 0, 499);
        
        assert(is_sorted_any_order(reverse, 500));
        assert(is_permutation(reverse, 500, original));
    }
    
    std::cout << "✅ merge_sort ultra-strict test passed (18 test cases)" << std::endl;
    std::cout << "   - Implementation-agnostic (accepts any sort order)" << std::endl;
    std::cout << "   - Empty array handled" << std::endl;
    std::cout << "   - Single/two elements verified" << std::endl;
    std::cout << "   - Already sorted (both directions) verified" << std::endl;
    std::cout << "   - All identical elements verified" << std::endl;
    std::cout << "   - Random data verified" << std::endl;
    std::cout << "   - Large arrays (1,000 and 10,000 elements) verified" << std::endl;
    std::cout << "   - Extreme values verified" << std::endl;
    std::cout << "   - Duplicates verified" << std::endl;
    std::cout << "   - Negative numbers verified" << std::endl;
    std::cout << "   - Partial sort verified" << std::endl;
    std::cout << "   - Data preservation verified (no loss)" << std::endl;
    std::cout << "   - Permutation property verified" << std::endl;
    return 0;
}
