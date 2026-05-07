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
    
    // Test 1: Both empty
    {
        ListNode* l1 = nullptr;
        ListNode* l2 = nullptr;
        ListNode* merged = Algorithms::linkedListMergeSorted(l1, l2);
        assert(merged == nullptr && "Merge of empty lists is empty");
    }
    
    // Test 2: First empty
    {
        ListNode* l1 = nullptr;
        ListNode* l2 = new ListNode(5);
        ListNode* merged = Algorithms::linkedListMergeSorted(l1, l2);
        assert(merged != nullptr && "Merged list not empty");
        assert(merged->data == 5 && "Correct value");
        assert(count_nodes(merged) == 1 && "One node");
    }
    
    // Test 3: Second empty
    {
        ListNode* l1 = new ListNode(5);
        ListNode* l2 = nullptr;
        ListNode* merged = Algorithms::linkedListMergeSorted(l1, l2);
        assert(merged != nullptr && "Merged list not empty");
        assert(merged->data == 5 && "Correct value");
        assert(count_nodes(merged) == 1 && "One node");
    }
    
    // Test 4: Both single node
    {
        ListNode* l1 = new ListNode(3);
        ListNode* l2 = new ListNode(5);
        ListNode* merged = Algorithms::linkedListMergeSorted(l1, l2);
        assert(is_sorted(merged) && "Merged list sorted");
        assert(count_nodes(merged) == 2 && "Two nodes");
    }
    
    // Test 5: Different lengths
    {
        ListNode* l1 = new ListNode(1);
        l1->next = new ListNode(3);
        l1->next->next = new ListNode(5);
        ListNode* l2 = new ListNode(2);
        l2->next = new ListNode(4);
        ListNode* merged = Algorithms::linkedListMergeSorted(l1, l2);
        assert(is_sorted(merged) && "Merged list sorted");
        assert(count_nodes(merged) == 5 && "Five nodes");
    }
    
    // Test 6: No overlap
    {
        ListNode* l1 = new ListNode(1);
        l1->next = new ListNode(2);
        ListNode* l2 = new ListNode(5);
        l2->next = new ListNode(6);
        ListNode* merged = Algorithms::linkedListMergeSorted(l1, l2);
        assert(is_sorted(merged) && "Merged list sorted");
        assert(count_nodes(merged) == 4 && "Four nodes");
    }
    
    // Test 7: With duplicates
    {
        ListNode* l1 = new ListNode(1);
        l1->next = new ListNode(3);
        l1->next->next = new ListNode(5);
        ListNode* l2 = new ListNode(1);
        l2->next = new ListNode(3);
        l2->next->next = new ListNode(5);
        ListNode* merged = Algorithms::linkedListMergeSorted(l1, l2);
        assert(is_sorted(merged) && "Merged list sorted");
        assert(count_nodes(merged) == 6 && "Six nodes");
    }
    
    // Test 8: Large lists
    {
        ListNode* l1 = new ListNode(1);
        ListNode* curr1 = l1;
        for (int i = 3; i <= 99; i += 2) {
            curr1->next = new ListNode(i);
            curr1 = curr1->next;
        }
        ListNode* l2 = new ListNode(2);
        ListNode* curr2 = l2;
        for (int i = 4; i <= 100; i += 2) {
            curr2->next = new ListNode(i);
            curr2 = curr2->next;
        }
        ListNode* merged = Algorithms::linkedListMergeSorted(l1, l2);
        assert(is_sorted(merged) && "Large merged list sorted");
        assert(count_nodes(merged) == 100 && "100 nodes");
    }
    
    return 0;
}
