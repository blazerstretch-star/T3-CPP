#include "../src/ds_algorithms.h"
#include <cassert>
#include <climits>

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
    
    // Test 1: Dequeue from empty
    {
        int arr[10] = {0};
        int size = 0;
        int value;
        bool result = Algorithms::priorityQueueDequeue(arr, size, value);
        assert(!result && "Dequeue from empty fails");
    }
    
    // Test 2: Dequeue single element
    {
        int arr[10] = {5};
        int size = 1;
        int value;
        bool result = Algorithms::priorityQueueDequeue(arr, size, value);
        assert(result && "Dequeue successful");
        assert(value == 5 && "Correct value");
        assert(size == 0 && "Size decreased");
    }
    
    // Test 3: Dequeue returns max
    {
        int arr[10] = {9, 5, 7, 3, 4};
        int size = 5;
        int value;
        Algorithms::priorityQueueDequeue(arr, size, value);
        assert(value == 9 && "Max value dequeued");
        assert(size == 4 && "Size decreased");
        assert(is_max_heap(arr, size) && "Heap property maintained");
    }
    
    // Test 4: Dequeue all elements
    {
        int arr[10] = {9, 7, 5, 3, 1};
        int size = 5;
        int prev = INT_MAX;
        while (size > 0) {
            int value;
            Algorithms::priorityQueueDequeue(arr, size, value);
            assert(value <= prev && "Dequeued in descending order");
            prev = value;
            if (size > 0) assert(is_max_heap(arr, size) && "Heap property");
        }
    }
    
    // Test 5: Enqueue and dequeue
    {
        int arr[10] = {0};
        int size = 0;
        Algorithms::priorityQueueEnqueue(arr, size, 10, 5);
        Algorithms::priorityQueueEnqueue(arr, size, 10, 3);
        Algorithms::priorityQueueEnqueue(arr, size, 10, 7);
        int value;
        Algorithms::priorityQueueDequeue(arr, size, value);
        assert(value == 7 && "Max dequeued");
        assert(size == 2 && "Size correct");
    }
    
    // Test 6: Dequeue maintains heap
    {
        int arr[10] = {0};
        int size = 0;
        for (int i = 1; i <= 7; i++) {
            Algorithms::priorityQueueEnqueue(arr, size, 10, i);
        }
        int value;
        Algorithms::priorityQueueDequeue(arr, size, value);
        assert(is_max_heap(arr, size) && "Heap property after dequeue");
    }
    
    // Test 7: Multiple dequeues
    {
        int arr[10] = {0};
        int size = 0;
        int values[] = {3, 7, 1, 9, 2, 5};
        for (int v : values) {
            Algorithms::priorityQueueEnqueue(arr, size, 10, v);
        }
        int value;
        Algorithms::priorityQueueDequeue(arr, size, value);
        assert(value == 9 && "First dequeue is max");
        Algorithms::priorityQueueDequeue(arr, size, value);
        assert(value == 7 && "Second dequeue is second max");
    }
    
    // Test 8: Dequeue until empty
    {
        int arr[10] = {0};
        int size = 0;
        for (int i = 1; i <= 5; i++) {
            Algorithms::priorityQueueEnqueue(arr, size, 10, i);
        }
        while (size > 0) {
            int value;
            bool result = Algorithms::priorityQueueDequeue(arr, size, value);
            assert(result && "Dequeue successful");
        }
        assert(size == 0 && "Queue empty");
    }
    
    return 0;
}
