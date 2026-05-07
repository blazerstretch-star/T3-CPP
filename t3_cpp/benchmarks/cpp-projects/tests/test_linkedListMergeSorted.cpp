#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    ListNode* l1 = new ListNode(1);
    l1->next = new ListNode(3);
    ListNode* l2 = new ListNode(2);
    ListNode* merged = Algorithms::linkedListMergeSorted(l1, l2);
    assert(merged->data == 1 && merged->next->data == 2);
    return 0;
}
