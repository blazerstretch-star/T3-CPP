#include "../src/suffixtree_functions.h"
#include <cassert>
#include <iostream>
#include <algorithm>

int main() {
	// Test case 1: Rebuild simple tree and verify structure
	SuffixTreeContext ctx1;
	ctx1.end_marker = 0x03;
	ctx1.text = "abc";
	ctx1.text.push_back(0x03);
	rebuild(ctx1);
	assert(ctx1.root != nullptr);
	assert(contains(ctx1, "abc"));
	assert(contains(ctx1, "bc"));
	assert(contains(ctx1, "c"));
	assert(!contains(ctx1, "xyz"));

	// Test case 2: Rebuild tree with repetitions - verify exact positions
	SuffixTreeContext ctx2;
	ctx2.end_marker = 0x03;
	ctx2.text = "banana";
	ctx2.text.push_back(0x03);
	rebuild(ctx2);
	assert(contains(ctx2, "ana"));
	auto result = find(ctx2, "ana");
	assert(result.size() == 2);
	assert(std::find(result.begin(), result.end(), 1) != result.end());
	assert(std::find(result.begin(), result.end(), 3) != result.end());

	// Test case 3: Rebuild empty tree
	SuffixTreeContext ctx3;
	ctx3.end_marker = 0x03;
	ctx3.text = "";
	ctx3.text.push_back(0x03);
	rebuild(ctx3);
	assert(ctx3.root != nullptr);

	// Test case 4: Rebuild tree with all same characters - verify all positions
	SuffixTreeContext ctx4;
	ctx4.end_marker = 0x03;
	ctx4.text = "aaaa";
	ctx4.text.push_back(0x03);
	rebuild(ctx4);
	auto result4 = find(ctx4, "a");
	assert(result4.size() == 4);
	assert(std::find(result4.begin(), result4.end(), 0) != result4.end());
	assert(std::find(result4.begin(), result4.end(), 1) != result4.end());
	assert(std::find(result4.begin(), result4.end(), 2) != result4.end());
	assert(std::find(result4.begin(), result4.end(), 3) != result4.end());

	// Test case 5: Suffix links are valid after rebuild
	SuffixTreeContext ctx5;
	ctx5.end_marker = 0x03;
	set_text(ctx5, "abcabcabc");
	check_suffix_links(ctx5); // Must not throw

	// Test case 6: Rebuild produces correct suffix count
	// "abcabc" has 6 suffixes: abcabc, bcabc, cabc, abc, bc, c
	SuffixTreeContext ctx6;
	ctx6.end_marker = 0x03;
	ctx6.text = "abcabc";
	ctx6.text.push_back(0x03);
	rebuild(ctx6);
	assert(contains(ctx6, "abcabc"));
	assert(contains(ctx6, "bcabc"));
	assert(contains(ctx6, "cabc"));
	assert(contains(ctx6, "abc"));
	assert(contains(ctx6, "bc"));
	assert(contains(ctx6, "c"));
	assert(!contains(ctx6, "abcabcd"));

	// Test case 7: end_of_text is set correctly after rebuild
	SuffixTreeContext ctx7;
	ctx7.end_marker = 0x03;
	ctx7.text = "hello";
	ctx7.text.push_back(0x03);
	rebuild(ctx7);
	assert(ctx7.end_of_text == ctx7.text.size());

	std::cout << "All tests passed!" << std::endl;
	return 0;
}
