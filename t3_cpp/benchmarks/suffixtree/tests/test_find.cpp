#include "../src/suffixtree_functions.h"
#include <cassert>
#include <iostream>
#include <algorithm>
#include <string>

int main() {
	// Test case 1: Find pattern in simple text - verify exact positions
	SuffixTreeContext ctx1;
	ctx1.end_marker = 0x03;
	set_text(ctx1, "abcabc");
	auto result1 = find(ctx1, "abc");
	assert(result1.size() == 2);
	assert(std::find(result1.begin(), result1.end(), 0) != result1.end());
	assert(std::find(result1.begin(), result1.end(), 3) != result1.end());

	// Test case 2: Find pattern not in text
	SuffixTreeContext ctx2;
	ctx2.end_marker = 0x03;
	set_text(ctx2, "hello");
	auto result2 = find(ctx2, "xyz");
	assert(result2.empty());

	// Test case 3: Find overlapping patterns - verify exact positions
	SuffixTreeContext ctx3;
	ctx3.end_marker = 0x03;
	set_text(ctx3, "banana");
	auto result3 = find(ctx3, "ana");
	assert(result3.size() == 2);
	assert(std::find(result3.begin(), result3.end(), 1) != result3.end());
	assert(std::find(result3.begin(), result3.end(), 3) != result3.end());

	// Test case 4: Find single character - verify all 3 positions
	SuffixTreeContext ctx4;
	ctx4.end_marker = 0x03;
	set_text(ctx4, "aaa");
	auto result4 = find(ctx4, "a");
	assert(result4.size() == 3);
	assert(std::find(result4.begin(), result4.end(), 0) != result4.end());
	assert(std::find(result4.begin(), result4.end(), 1) != result4.end());
	assert(std::find(result4.begin(), result4.end(), 2) != result4.end());

	// Test case 5: Find empty pattern - returns non-empty result
	SuffixTreeContext ctx5;
	ctx5.end_marker = 0x03;
	set_text(ctx5, "test");
	auto result5 = find(ctx5, "");
	assert(!result5.empty());

	// Test case 6: Find in empty text
	SuffixTreeContext ctx6;
	ctx6.end_marker = 0x03;
	set_text(ctx6, "");
	auto result6 = find(ctx6, "a");
	assert(result6.empty());

	// Test case 7: Find pattern longer than text
	SuffixTreeContext ctx7;
	ctx7.end_marker = 0x03;
	set_text(ctx7, "ab");
	auto result7 = find(ctx7, "abcdef");
	assert(result7.empty());

	// Test case 8: Find with special characters - verify exact position
	SuffixTreeContext ctx8;
	ctx8.end_marker = 0x03;
	set_text(ctx8, "a b!c@d#");
	auto result8 = find(ctx8, "b!c");
	assert(result8.size() == 1);
	assert(result8[0] == 2);

	// Test case 9: Large text stress test - verify exact position
	SuffixTreeContext ctx9;
	ctx9.end_marker = 0x03;
	std::string large_text(1000, 'a');
	large_text += "xyz";
	large_text += std::string(1000, 'a');
	set_text(ctx9, large_text);
	auto result9 = find(ctx9, "xyz");
	assert(result9.size() == 1);
	assert(result9[0] == 1000);

	// Test case 10: Pattern at start only
	SuffixTreeContext ctx10;
	ctx10.end_marker = 0x03;
	set_text(ctx10, "abcdef");
	auto result10 = find(ctx10, "abc");
	assert(result10.size() == 1);
	assert(result10[0] == 0);

	// Test case 11: Pattern at end only
	SuffixTreeContext ctx11;
	ctx11.end_marker = 0x03;
	set_text(ctx11, "xyzabc");
	auto result11 = find(ctx11, "abc");
	assert(result11.size() == 1);
	assert(result11[0] == 3);

	// Test case 12: No duplicate positions returned
	SuffixTreeContext ctx12;
	ctx12.end_marker = 0x03;
	set_text(ctx12, "abab");
	auto result12 = find(ctx12, "ab");
	assert(result12.size() == 2);
	assert(std::find(result12.begin(), result12.end(), 0) != result12.end());
	assert(std::find(result12.begin(), result12.end(), 2) != result12.end());

	// Test case 13: mississippi - exact positions and cross-validate with get_text
	SuffixTreeContext ctx13;
	ctx13.end_marker = 0x03;
	set_text(ctx13, "mississippi");
	auto result13 = find(ctx13, "issi");
	assert(result13.size() == 2);
	assert(std::find(result13.begin(), result13.end(), 1) != result13.end());
	assert(std::find(result13.begin(), result13.end(), 4) != result13.end());
	{
		std::string text13 = get_text(ctx13);
		for (size_t p : result13)
			assert(text13.substr(p, 4) == "issi");
	}

	// Test case 14: abcabcabc - 3 occurrences, all positions valid
	SuffixTreeContext ctx14;
	ctx14.end_marker = 0x03;
	set_text(ctx14, "abcabcabc");
	auto result14 = find(ctx14, "abc");
	assert(result14.size() == 3);
	assert(std::find(result14.begin(), result14.end(), 0) != result14.end());
	assert(std::find(result14.begin(), result14.end(), 3) != result14.end());
	assert(std::find(result14.begin(), result14.end(), 6) != result14.end());
	{
		std::string text14 = get_text(ctx14);
		for (size_t p : result14)
			assert(text14.substr(p, 3) == "abc");
	}

	std::cout << "All tests passed!" << std::endl;
	return 0;
}
