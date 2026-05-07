#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    ListNode* head = new ListNode(1);
    head->next = new ListNode(2);
    Algorithms::linkedListReverse(head);
    assert(head->data == 2 && head->next->data == 1);
    return 0;
}
