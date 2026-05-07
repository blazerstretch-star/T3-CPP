#include "../src/suffixtree_functions.h"
#include <cassert>
#include <iostream>

int main() {
	// Test case 1: Traverse tree with full match - position must reach end, node != root
	SuffixTreeContext ctx1;
	ctx1.end_marker = 0x03;
	set_text(ctx1, "hello");
	size_t pos1 = 0;
	auto result1 = traverse_tree(ctx1, "hello", pos1, 5);
	assert(result1 != nullptr);
	assert(pos1 == 5);
	assert(result1 != ctx1.root); // full match lands on a non-root node

	// Test case 2: Traverse tree with partial match - position must reach end of query
	SuffixTreeContext ctx2;
	ctx2.end_marker = 0x03;
	set_text(ctx2, "hello");
	size_t pos2 = 0;
	auto result2 = traverse_tree(ctx2, "hel", pos2, 3);
	assert(result2 != nullptr);
	assert(pos2 == 3);

	// Test case 3: Traverse tree with no match - position must stay at 0, returns root
	SuffixTreeContext ctx3;
	ctx3.end_marker = 0x03;
	set_text(ctx3, "abc");
	size_t pos3 = 0;
	auto result3 = traverse_tree(ctx3, "xyz", pos3, 3);
	assert(result3 != nullptr);
	assert(pos3 == 0);
	assert(result3 == ctx3.root); // no match stays at root

	// Test case 4: Traverse empty string - position stays 0, returns root
	SuffixTreeContext ctx4;
	ctx4.end_marker = 0x03;
	set_text(ctx4, "test");
	size_t pos4 = 0;
	auto result4 = traverse_tree(ctx4, "", pos4, 0);
	assert(result4 != nullptr);
	assert(pos4 == 0);

	// Test case 5: Traverse prefix that exists - position must equal prefix length
	SuffixTreeContext ctx5;
	ctx5.end_marker = 0x03;
	set_text(ctx5, "abcdef");
	size_t pos5 = 0;
	auto result5 = traverse_tree(ctx5, "abcdef", pos5, 6);
	assert(result5 != nullptr);
	assert(pos5 == 6);

	// Test case 6: Traverse string that partially matches then diverges
	SuffixTreeContext ctx6;
	ctx6.end_marker = 0x03;
	set_text(ctx6, "abcdef");
	size_t pos6 = 0;
	auto result6 = traverse_tree(ctx6, "abcxyz", pos6, 6);
	assert(result6 != nullptr);
	assert(pos6 == 3); // "abc" matched, then mismatch at 'x'

	// Test case 7: Traverse single character match
	SuffixTreeContext ctx7;
	ctx7.end_marker = 0x03;
	set_text(ctx7, "abc");
	size_t pos7 = 0;
	auto result7 = traverse_tree(ctx7, "a", pos7, 1);
	assert(result7 != nullptr);
	assert(pos7 == 1);

	// Test case 8: Traverse single character no match
	SuffixTreeContext ctx8;
	ctx8.end_marker = 0x03;
	set_text(ctx8, "abc");
	size_t pos8 = 0;
	auto result8 = traverse_tree(ctx8, "z", pos8, 1);
	assert(result8 != nullptr);
	assert(pos8 == 0);

	std::cout << "All tests passed!" << std::endl;
	return 0;
}
