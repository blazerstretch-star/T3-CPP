#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // Test 1: Empty list
    {
        ListNode* head = nullptr;
        bool hasCycle = Algorithms::linkedListDetectCycle(head);
        assert(!hasCycle && "Empty list has no cycle");
    }
    
    // Test 2: Single node, no cycle
    {
        ListNode* head = new ListNode(5);
        bool hasCycle = Algorithms::linkedListDetectCycle(head);
        assert(!hasCycle && "Single node has no cycle");
    }
    
    // Test 3: Two nodes, no cycle
    {
        ListNode* head = new ListNode(1);
        head->next = new ListNode(2);
        bool hasCycle = Algorithms::linkedListDetectCycle(head);
        assert(!hasCycle && "Two nodes, no cycle");
    }
    
    // Test 4: Three nodes, no cycle
    {
        ListNode* head = new ListNode(1);
        head->next = new ListNode(2);
        head->next->next = new ListNode(3);
        bool hasCycle = Algorithms::linkedListDetectCycle(head);
        assert(!hasCycle && "Three nodes, no cycle");
    }
    
    // Test 5: Single node cycle (self-loop)
    {
        ListNode* head = new ListNode(5);
        head->next = head;
        bool hasCycle = Algorithms::linkedListDetectCycle(head);
        assert(hasCycle && "Single node cycle detected");
    }
    
    // Test 6: Two nodes cycle
    {
        ListNode* head = new ListNode(1);
        head->next = new ListNode(2);
        head->next->next = head;
        bool hasCycle = Algorithms::linkedListDetectCycle(head);
        assert(hasCycle && "Two nodes cycle detected");
    }
    
    // Test 7: Cycle at end
    {
        ListNode* head = new ListNode(1);
        head->next = new ListNode(2);
        head->next->next = new ListNode(3);
        head->next->next->next = head->next;
        bool hasCycle = Algorithms::linkedListDetectCycle(head);
        assert(hasCycle && "Cycle at end detected");
    }
    
    // Test 8: Large list, no cycle
    {
        ListNode* head = new ListNode(1);
        ListNode* curr = head;
        for (int i = 2; i <= 100; i++) {
            curr->next = new ListNode(i);
            curr = curr->next;
        }
        bool hasCycle = Algorithms::linkedListDetectCycle(head);
        assert(!hasCycle && "Large list, no cycle");
    }
    
    // Test 9: Large list with cycle
    {
        ListNode* head = new ListNode(1);
        ListNode* curr = head;
        ListNode* cycleStart = nullptr;
        for (int i = 2; i <= 100; i++) {
            curr->next = new ListNode(i);
            curr = curr->next;
            if (i == 50) cycleStart = curr;
        }
        curr->next = cycleStart;
        bool hasCycle = Algorithms::linkedListDetectCycle(head);
        assert(hasCycle && "Large list with cycle detected");
    }
    
    return 0;
}
