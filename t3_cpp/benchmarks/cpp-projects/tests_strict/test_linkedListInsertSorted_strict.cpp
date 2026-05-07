#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    auto count_nodes = [](ListNode* head) {
        int count = 0;
        while (head) { count++; head = head->next; }
        return count;
    };
    
    auto is_sorted = [](ListNode* head) {
        while (head && head->next) {
            if (head->data > head->next->data) return false;
            head = head->next;
        }
        return true;
    };
    
    // Test 1: Insert into empty list
    {
        ListNode* head = nullptr;
        bool result = Algorithms::linkedListInsertSorted(head, 5);
        assert(result && "Insert successful");
        assert(head != nullptr && "Head created");
        assert(head->data == 5 && "Correct value");
        assert(count_nodes(head) == 1 && "One node");
    }
    
    // Test 2: Insert at beginning
    {
        ListNode* head = new ListNode(10);
        Algorithms::linkedListInsertSorted(head, 5);
        assert(head->data == 5 && "New head");
        assert(is_sorted(head) && "List sorted");
        assert(count_nodes(head) == 2 && "Two nodes");
    }
    
    // Test 3: Insert at end
    {
        ListNode* head = new ListNode(5);
        Algorithms::linkedListInsertSorted(head, 10);
        assert(head->data == 5 && "Head unchanged");
        assert(is_sorted(head) && "List sorted");
        assert(count_nodes(head) == 2 && "Two nodes");
    }
    
    // Test 4: Insert in middle
    {
        ListNode* head = new ListNode(5);
        head->next = new ListNode(15);
        Algorithms::linkedListInsertSorted(head, 10);
        assert(is_sorted(head) && "List sorted");
        assert(count_nodes(head) == 3 && "Three nodes");
    }
    
    // Test 5: Insert multiple values
    {
        ListNode* head = nullptr;
        int values[] = {5, 2, 8, 1, 9, 3};
        for (int v : values) {
            Algorithms::linkedListInsertSorted(head, v);
        }
        assert(is_sorted(head) && "List sorted");
        assert(count_nodes(head) == 6 && "Six nodes");
    }
    
    // Test 6: Insert duplicates
    {
        ListNode* head = nullptr;
        Algorithms::linkedListInsertSorted(head, 5);
        Algorithms::linkedListInsertSorted(head, 5);
        Algorithms::linkedListInsertSorted(head, 5);
        assert(is_sorted(head) && "List sorted");
        assert(count_nodes(head) == 3 && "Three nodes");
    }
    
    // Test 7: Insert in ascending order
    {
        ListNode* head = nullptr;
        for (int i = 1; i <= 5; i++) {
            Algorithms::linkedListInsertSorted(head, i);
        }
        assert(is_sorted(head) && "List sorted");
        assert(count_nodes(head) == 5 && "Five nodes");
    }
    
    // Test 8: Insert in descending order
    {
        ListNode* head = nullptr;
        for (int i = 5; i >= 1; i--) {
            Algorithms::linkedListInsertSorted(head, i);
        }
        assert(is_sorted(head) && "List sorted");
        assert(count_nodes(head) == 5 && "Five nodes");
    }
    
    return 0;
}
