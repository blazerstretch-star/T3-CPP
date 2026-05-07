#include "../src/suffixtree_functions.h"
#include <cassert>
#include <iostream>

int main() {
	// Test case 1: Traverse node with matching edge - position advances
	SuffixTreeContext ctx1;
	ctx1.end_marker = 0x03;
	set_text(ctx1, "abc");
	size_t pos1 = 0;
	auto result1 = traverse_node(ctx1, ctx1.root, "abc", pos1, 3);
	assert(result1 != nullptr);
	assert(pos1 > 0);

	// Test case 2: Traverse node with no matching edge - returns root, position stays 0
	SuffixTreeContext ctx2;
	ctx2.end_marker = 0x03;
	set_text(ctx2, "abc");
	size_t pos2 = 0;
	auto result2 = traverse_node(ctx2, ctx2.root, "xyz", pos2, 3);
	assert(result2 == ctx2.root);
	assert(pos2 == 0);

	// Test case 3: Traverse to leaf then try to traverse further - leaf returns nullptr
	SuffixTreeContext ctx3;
	ctx3.end_marker = 0x03;
	set_text(ctx3, "a");
	// "a" has a single suffix "a\x03", traverse to the leaf
	size_t pos3 = 0;
	auto leaf = traverse_node(ctx3, ctx3.root, "a", pos3, 1);
	assert(leaf != nullptr);
	assert(leaf != ctx3.root);
	// Now traverse from the leaf - it has no children, must return nullptr
	size_t pos3b = 0;
	auto result3 = traverse_node(ctx3, leaf, "x", pos3b, 1);
	assert(result3 == nullptr);

	// Test case 4: Partial match on edge - position advances only to end of query
	SuffixTreeContext ctx4;
	ctx4.end_marker = 0x03;
	set_text(ctx4, "abcdef");
	size_t pos4 = 0;
	auto result4 = traverse_node(ctx4, ctx4.root, "ab", pos4, 2);
	assert(result4 != nullptr);
	assert(pos4 == 2);

	// Test case 5: Single character match - position advances by 1
	SuffixTreeContext ctx5;
	ctx5.end_marker = 0x03;
	set_text(ctx5, "xyz");
	size_t pos5 = 0;
	auto result5 = traverse_node(ctx5, ctx5.root, "x", pos5, 1);
	assert(result5 != nullptr);
	assert(pos5 == 1);

	// Test case 6: Mismatch after first character - position stops at mismatch
	SuffixTreeContext ctx6;
	ctx6.end_marker = 0x03;
	set_text(ctx6, "abcdef");
	size_t pos6 = 0;
	// "az" - 'a' matches start of edge, 'z' mismatches
	auto result6 = traverse_node(ctx6, ctx6.root, "az", pos6, 2);
	assert(result6 != nullptr);
	assert(pos6 == 1); // Only 'a' matched

	std::cout << "All tests passed!" << std::endl;
	return 0;
}
