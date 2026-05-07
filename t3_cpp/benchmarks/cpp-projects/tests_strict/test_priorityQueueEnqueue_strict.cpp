#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    auto is_max_heap = [](int arr[], int size) {
        for (int i = 0; i < size; i++) {
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            if (left < size && arr[i] < arr[left]) return false;
            if (right < size && arr[i] < arr[right]) return false;
        }
        return true;
    };
    
    // Test 1: Enqueue into empty
    {
        int arr[10] = {0};
        int size = 0;
        bool result = Algorithms::priorityQueueEnqueue(arr, size, 10, 5);
        assert(result && "Enqueue successful");
        assert(size == 1 && "Size increased");
        assert(arr[0] == 5 && "Value inserted");
        assert(is_max_heap(arr, size) && "Heap property");
    }
    
    // Test 2: Enqueue multiple values
    {
        int arr[10] = {0};
        int size = 0;
        Algorithms::priorityQueueEnqueue(arr, size, 10, 5);
        Algorithms::priorityQueueEnqueue(arr, size, 10, 3);
        Algorithms::priorityQueueEnqueue(arr, size, 10, 7);
        assert(size == 3 && "Three elements");
        assert(is_max_heap(arr, size) && "Heap property");
    }
    
    // Test 3: Enqueue in ascending order
    {
        int arr[10] = {0};
        int size = 0;
        for (int i = 1; i <= 5; i++) {
            Algorithms::priorityQueueEnqueue(arr, size, 10, i);
        }
        assert(size == 5 && "Five elements");
        assert(is_max_heap(arr, size) && "Heap property");
    }
    
    // Test 4: Enqueue in descending order
    {
        int arr[10] = {0};
        int size = 0;
        for (int i = 5; i >= 1; i--) {
            Algorithms::priorityQueueEnqueue(arr, size, 10, i);
        }
        assert(size == 5 && "Five elements");
        assert(is_max_heap(arr, size) && "Heap property");
    }
    
    // Test 5: Enqueue to capacity
    {
        int arr[5] = {0};
        int size = 0;
        for (int i = 1; i <= 5; i++) {
            bool result = Algorithms::priorityQueueEnqueue(arr, size, 5, i);
            assert(result && "Enqueue successful");
        }
        assert(size == 5 && "At capacity");
        assert(is_max_heap(arr, size) && "Heap property");
    }
    
    // Test 6: Enqueue beyond capacity
    {
        int arr[5] = {0};
        int size = 5;
        bool result = Algorithms::priorityQueueEnqueue(arr, size, 5, 10);
        assert(!result && "Enqueue fails when full");
        assert(size == 5 && "Size unchanged");
    }
    
    // Test 7: Enqueue duplicates
    {
        int arr[10] = {0};
        int size = 0;
        Algorithms::priorityQueueEnqueue(arr, size, 10, 5);
        Algorithms::priorityQueueEnqueue(arr, size, 10, 5);
        Algorithms::priorityQueueEnqueue(arr, size, 10, 5);
        assert(size == 3 && "Three elements");
        assert(is_max_heap(arr, size) && "Heap property");
    }
    
    // Test 8: Max always at root
    {
        int arr[10] = {0};
        int size = 0;
        int values[] = {3, 7, 1, 9, 2, 5};
        for (int v : values) {
            Algorithms::priorityQueueEnqueue(arr, size, 10, v);
            assert(arr[0] >= v && "Max at root");
        }
    }
    
    return 0;
}
