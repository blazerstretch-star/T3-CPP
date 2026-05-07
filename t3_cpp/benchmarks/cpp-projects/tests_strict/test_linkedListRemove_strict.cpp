#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    auto count_nodes = [](ListNode* head) {
        int count = 0;
        while (head) { count++; head = head->next; }
        return count;
    };
    
    auto contains = [](ListNode* head, int value) {
        while (head) {
            if (head->data == value) return true;
            head = head->next;
        }
        return false;
    };
    
    // Test 1: Remove from empty list
    {
        ListNode* head = nullptr;
        bool result = Algorithms::linkedListRemove(head, 5);
        assert(!result && "Remove from empty fails");
        assert(head == nullptr && "Head still null");
    }
    
    // Test 2: Remove only node
    {
        ListNode* head = new ListNode(5);
        bool result = Algorithms::linkedListRemove(head, 5);
        assert(result && "Remove successful");
        assert(head == nullptr && "List empty");
    }
    
    // Test 3: Remove head
    {
        ListNode* head = new ListNode(5);
        head->next = new ListNode(10);
        bool result = Algorithms::linkedListRemove(head, 5);
        assert(result && "Remove successful");
        assert(head->data == 10 && "New head");
        assert(count_nodes(head) == 1 && "One node left");
    }
    
    // Test 4: Remove tail
    {
        ListNode* head = new ListNode(5);
        head->next = new ListNode(10);
        bool result = Algorithms::linkedListRemove(head, 10);
        assert(result && "Remove successful");
        assert(head->data == 5 && "Head unchanged");
        assert(count_nodes(head) == 1 && "One node left");
    }
    
    // Test 5: Remove middle
    {
        ListNode* head = new ListNode(5);
        head->next = new ListNode(10);
        head->next->next = new ListNode(15);
        bool result = Algorithms::linkedListRemove(head, 10);
        assert(result && "Remove successful");
        assert(!contains(head, 10) && "Value removed");
        assert(count_nodes(head) == 2 && "Two nodes left");
    }
    
    // Test 6: Remove non-existing
    {
        ListNode* head = new ListNode(5);
        head->next = new ListNode(10);
        bool result = Algorithms::linkedListRemove(head, 99);
        assert(!result && "Remove fails");
        assert(count_nodes(head) == 2 && "No nodes removed");
    }
    
    // Test 7: Remove first occurrence of duplicate
    {
        ListNode* head = new ListNode(5);
        head->next = new ListNode(10);
        head->next->next = new ListNode(10);
        bool result = Algorithms::linkedListRemove(head, 10);
        assert(result && "Remove successful");
        assert(count_nodes(head) == 2 && "One node removed");
    }
    
    // Test 8: Remove multiple values
    {
        ListNode* head = new ListNode(1);
        head->next = new ListNode(2);
        head->next->next = new ListNode(3);
        head->next->next->next = new ListNode(4);
        Algorithms::linkedListRemove(head, 2);
        Algorithms::linkedListRemove(head, 4);
        assert(count_nodes(head) == 2 && "Two nodes removed");
        assert(!contains(head, 2) && !contains(head, 4) && "Values removed");
    }
    
    return 0;
}
