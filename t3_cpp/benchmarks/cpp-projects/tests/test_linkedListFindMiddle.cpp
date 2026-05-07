#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    ListNode* head = new ListNode(1);
    head->next = new ListNode(2);
    head->next->next = new ListNode(3);
    assert(Algorithms::linkedListFindMiddle(head)->data == 2);
    return 0;
}
