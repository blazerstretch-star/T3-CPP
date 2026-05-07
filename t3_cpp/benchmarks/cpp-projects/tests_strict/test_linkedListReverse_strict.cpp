#include "../src/ds_algorithms.h"
#include <cassert>
#include <vector>

int main() {
    auto count_nodes = [](ListNode* head) {
        int count = 0;
        while (head) { count++; head = head->next; }
        return count;
    };
    
    auto to_vector = [](ListNode* head) {
        std::vector<int> result;
        while (head) {
            result.push_back(head->data);
            head = head->next;
        }
        return result;
    };
    
    // Test 1: Reverse empty list
    {
        ListNode* head = nullptr;
        Algorithms::linkedListReverse(head);
        assert(head == nullptr && "Empty list stays empty");
    }
    
    // Test 2: Reverse single node
    {
        ListNode* head = new ListNode(5);
        Algorithms::linkedListReverse(head);
        assert(head->data == 5 && "Single node unchanged");
        assert(count_nodes(head) == 1 && "One node");
    }
    
    // Test 3: Reverse two nodes
    {
        ListNode* head = new ListNode(1);
        head->next = new ListNode(2);
        Algorithms::linkedListReverse(head);
        auto vec = to_vector(head);
        assert(vec[0] == 2 && vec[1] == 1 && "Two nodes reversed");
    }
    
    // Test 4: Reverse three nodes
    {
        ListNode* head = new ListNode(1);
        head->next = new ListNode(2);
        head->next->next = new ListNode(3);
        Algorithms::linkedListReverse(head);
        auto vec = to_vector(head);
        assert(vec[0] == 3 && vec[1] == 2 && vec[2] == 1 && "Three nodes reversed");
    }
    
    // Test 5: Reverse five nodes
    {
        ListNode* head = new ListNode(1);
        head->next = new ListNode(2);
        head->next->next = new ListNode(3);
        head->next->next->next = new ListNode(4);
        head->next->next->next->next = new ListNode(5);
        Algorithms::linkedListReverse(head);
        auto vec = to_vector(head);
        assert(vec[0] == 5 && vec[4] == 1 && "Five nodes reversed");
        assert(count_nodes(head) == 5 && "All nodes preserved");
    }
    
    // Test 6: Double reverse (involution)
    {
        ListNode* head = new ListNode(1);
        head->next = new ListNode(2);
        head->next->next = new ListNode(3);
        auto original = to_vector(head);
        Algorithms::linkedListReverse(head);
        Algorithms::linkedListReverse(head);
        auto after = to_vector(head);
        assert(original == after && "Double reverse returns original");
    }
    
    // Test 7: Reverse with duplicates
    {
        ListNode* head = new ListNode(1);
        head->next = new ListNode(2);
        head->next->next = new ListNode(2);
        head->next->next->next = new ListNode(1);
        Algorithms::linkedListReverse(head);
        auto vec = to_vector(head);
        assert(vec[0] == 1 && vec[3] == 1 && "Duplicates preserved");
    }
    
    // Test 8: Large list
    {
        ListNode* head = new ListNode(1);
        ListNode* curr = head;
        for (int i = 2; i <= 100; i++) {
            curr->next = new ListNode(i);
            curr = curr->next;
        }
        Algorithms::linkedListReverse(head);
        assert(head->data == 100 && "First is 100");
        assert(count_nodes(head) == 100 && "All nodes preserved");
    }
    
    return 0;
}
