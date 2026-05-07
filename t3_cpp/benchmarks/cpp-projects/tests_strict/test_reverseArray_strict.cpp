#include "../src/ds_algorithms.h"
#include <cassert>
#include <set>

int main() {
    // ========================================
    // PROPERTY DEFINITIONS
    // ========================================
    
    // Property 1: reverse(reverse(arr)) = arr (involution)
    // Property 2: arr[i] after reverse = arr[size-1-i] before reverse
    // Property 3: Elements unchanged (permutation)
    
    auto is_permutation = [](int arr[], int original[], int size) {
        std::multiset<int> a(arr, arr + size);
        std::multiset<int> b(original, original + size);
        return a == b;
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Empty array
    { 
        int arr[] = {}; 
        Algorithms::reverseArray(arr, 0); 
    }
    
    // Test 2: Single element
    {
        int arr[] = {42};
        int orig[] = {42};
        Algorithms::reverseArray(arr, 1);
        assert(arr[0] == 42 && "Single element unchanged");
        assert(is_permutation(arr, orig, 1) && "Permutation preserved");
    }
    
    // Test 3: Two elements
    {
        int arr[] = {1, 2};
        Algorithms::reverseArray(arr, 2);
        assert(arr[0] == 2 && arr[1] == 1 && "Two elements reversed");
    }
    
    // Test 4: Odd number of elements
    {
        int arr[] = {1, 2, 3, 4, 5};
        int orig[] = {1, 2, 3, 4, 5};
        Algorithms::reverseArray(arr, 5);
        assert(arr[0] == 5 && arr[4] == 1 && "Ends swapped");
        assert(arr[2] == 3 && "Middle unchanged");
        assert(is_permutation(arr, orig, 5) && "Permutation preserved");
    }
    
    // Test 5: Even number of elements
    {
        int arr[] = {1, 2, 3, 4};
        Algorithms::reverseArray(arr, 4);
        assert(arr[0] == 4 && arr[3] == 1 && "Reversed correctly");
        assert(arr[1] == 3 && arr[2] == 2 && "Middle reversed");
    }
    
    // Test 6: Involution property (double reverse)
    {
        int arr[] = {1, 2, 3, 4, 5};
        int orig[] = {1, 2, 3, 4, 5};
        Algorithms::reverseArray(arr, 5);
        Algorithms::reverseArray(arr, 5);
        for (int i = 0; i < 5; i++) {
            assert(arr[i] == orig[i] && "Double reverse returns original");
        }
    }
    
    // Test 7: All equal elements
    {
        int arr[] = {7, 7, 7, 7};
        Algorithms::reverseArray(arr, 4);
        assert(arr[0] == 7 && arr[3] == 7 && "All equal unchanged");
    }
    
    // Test 8: Negative numbers
    {
        int arr[] = {-1, -2, -3};
        Algorithms::reverseArray(arr, 3);
        assert(arr[0] == -3 && arr[2] == -1 && "Negative numbers reversed");
    }
    
    // Test 9: Large array
    {
        const int size = 1000;
        int* arr = new int[size];
        for (int i = 0; i < size; i++) arr[i] = i;
        Algorithms::reverseArray(arr, size);
        assert(arr[0] == 999 && arr[999] == 0 && "Large array reversed");
        delete[] arr;
    }
    
    return 0;
}
