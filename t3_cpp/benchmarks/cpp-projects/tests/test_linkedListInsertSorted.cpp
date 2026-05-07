#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    ListNode* head = nullptr;
    Algorithms::linkedListInsertSorted(head, 5);
    Algorithms::linkedListInsertSorted(head, 3);
    assert(head->data == 3 && head->next->data == 5);
    return 0;
}
