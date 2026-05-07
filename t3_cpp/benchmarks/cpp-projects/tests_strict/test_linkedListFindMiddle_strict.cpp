#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    auto count_nodes = [](ListNode* head) {
        int count = 0;
        while (head) { count++; head = head->next; }
        return count;
    };
    
    // Test 1: Empty list
    {
        ListNode* head = nullptr;
        ListNode* middle = Algorithms::linkedListFindMiddle(head);
        assert(middle == nullptr && "Empty list has no middle");
    }
    
    // Test 2: Single node
    {
        ListNode* head = new ListNode(5);
        ListNode* middle = Algorithms::linkedListFindMiddle(head);
        assert(middle != nullptr && "Middle found");
        assert(middle->data == 5 && "Single node is middle");
    }
    
    // Test 3: Two nodes
    {
        ListNode* head = new ListNode(1);
        head->next = new ListNode(2);
        ListNode* middle = Algorithms::linkedListFindMiddle(head);
        assert(middle != nullptr && "Middle found");
        assert((middle->data == 1 || middle->data == 2) && "Middle is first or second");
    }
    
    // Test 4: Three nodes (odd)
    {
        ListNode* head = new ListNode(1);
        head->next = new ListNode(2);
        head->next->next = new ListNode(3);
        ListNode* middle = Algorithms::linkedListFindMiddle(head);
        assert(middle != nullptr && "Middle found");
        assert(middle->data == 2 && "Middle is second node");
    }
    
    // Test 5: Four nodes (even)
    {
        ListNode* head = new ListNode(1);
        head->next = new ListNode(2);
        head->next->next = new ListNode(3);
        head->next->next->next = new ListNode(4);
        ListNode* middle = Algorithms::linkedListFindMiddle(head);
        assert(middle != nullptr && "Middle found");
        assert((middle->data == 2 || middle->data == 3) && "Middle is 2nd or 3rd");
    }
    
    // Test 5: Five nodes (odd)
    {
        ListNode* head = new ListNode(1);
        head->next = new ListNode(2);
        head->next->next = new ListNode(3);
        head->next->next->next = new ListNode(4);
        head->next->next->next->next = new ListNode(5);
        ListNode* middle = Algorithms::linkedListFindMiddle(head);
        assert(middle != nullptr && "Middle found");
        assert(middle->data == 3 && "Middle is third node");
    }
    
    // Test 7: Seven nodes
    {
        ListNode* head = new ListNode(1);
        ListNode* curr = head;
        for (int i = 2; i <= 7; i++) {
            curr->next = new ListNode(i);
            curr = curr->next;
        }
        ListNode* middle = Algorithms::linkedListFindMiddle(head);
        assert(middle != nullptr && "Middle found");
        assert(middle->data == 4 && "Middle is fourth node");
    }
    
    // Test 8: Large list
    {
        ListNode* head = new ListNode(1);
        ListNode* curr = head;
        for (int i = 2; i <= 99; i++) {
            curr->next = new ListNode(i);
            curr = curr->next;
        }
        ListNode* middle = Algorithms::linkedListFindMiddle(head);
        assert(middle != nullptr && "Middle found");
        assert(middle->data == 50 && "Middle is 50th node");
    }
    
    return 0;
}
