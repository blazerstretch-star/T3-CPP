#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // Property: After partition, pivot is at correct position
    // Property: All elements left of pivot <= pivot
    // Property: All elements right of pivot >= pivot
    
    auto verify_partition = [](int arr[], int left, int right, int pivot_idx) {
        int pivot = arr[pivot_idx];
        for (int i = left; i < pivot_idx; i++) {
            if (arr[i] > pivot) return false;
        }
        for (int i = pivot_idx + 1; i <= right; i++) {
            if (arr[i] < pivot) return false;
        }
        return true;
    };
    
    // Test 1: Two elements
    {
        int arr[] = {2, 1};
        int pivot = Algorithms::quickSortPartition(arr, 0, 1);
        assert(pivot >= 0 && pivot <= 1 && "Pivot in range");
        assert(verify_partition(arr, 0, 1, pivot) && "Partition property holds");
    }
    
    // Test 2: Already sorted
    {
        int arr[] = {1, 2, 3, 4, 5};
        int pivot = Algorithms::quickSortPartition(arr, 0, 4);
        assert(pivot >= 0 && pivot <= 4 && "Pivot in range");
        assert(verify_partition(arr, 0, 4, pivot) && "Partition property holds");
    }
    
    // Test 3: Reverse sorted
    {
        int arr[] = {5, 4, 3, 2, 1};
        int pivot = Algorithms::quickSortPartition(arr, 0, 4);
        assert(pivot >= 0 && pivot <= 4 && "Pivot in range");
        assert(verify_partition(arr, 0, 4, pivot) && "Partition property holds");
    }
    
    // Test 4: All equal
    {
        int arr[] = {5, 5, 5, 5};
        int pivot = Algorithms::quickSortPartition(arr, 0, 3);
        assert(pivot >= 0 && pivot <= 3 && "Pivot in range");
        assert(verify_partition(arr, 0, 3, pivot) && "Partition property holds");
    }
    
    // Test 5: Random order
    {
        int arr[] = {3, 7, 1, 9, 2};
        int pivot = Algorithms::quickSortPartition(arr, 0, 4);
        assert(pivot >= 0 && pivot <= 4 && "Pivot in range");
        assert(verify_partition(arr, 0, 4, pivot) && "Partition property holds");
    }
    
    // Test 6: Duplicates
    {
        int arr[] = {3, 1, 4, 1, 5};
        int pivot = Algorithms::quickSortPartition(arr, 0, 4);
        assert(pivot >= 0 && pivot <= 4 && "Pivot in range");
        assert(verify_partition(arr, 0, 4, pivot) && "Partition property holds");
    }
    
    // Test 7: Negative numbers
    {
        int arr[] = {-3, -1, -5, -2};
        int pivot = Algorithms::quickSortPartition(arr, 0, 3);
        assert(pivot >= 0 && pivot <= 3 && "Pivot in range");
        assert(verify_partition(arr, 0, 3, pivot) && "Partition property holds");
    }
    
    // Test 8: Mixed signs
    {
        int arr[] = {-3, 5, -1, 0, 2};
        int pivot = Algorithms::quickSortPartition(arr, 0, 4);
        assert(pivot >= 0 && pivot <= 4 && "Pivot in range");
        assert(verify_partition(arr, 0, 4, pivot) && "Partition property holds");
    }
    
    return 0;
}
