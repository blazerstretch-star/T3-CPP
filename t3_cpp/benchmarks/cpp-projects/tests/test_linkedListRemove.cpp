#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    ListNode* head = new ListNode(1);
    head->next = new ListNode(2);
    Algorithms::linkedListRemove(head, 1);
    assert(head->data == 2);
    return 0;
}
