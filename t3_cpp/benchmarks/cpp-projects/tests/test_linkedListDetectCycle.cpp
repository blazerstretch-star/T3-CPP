#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    ListNode* head = new ListNode(1);
    head->next = new ListNode(2);
    assert(Algorithms::linkedListDetectCycle(head) == false);
    return 0;
}
