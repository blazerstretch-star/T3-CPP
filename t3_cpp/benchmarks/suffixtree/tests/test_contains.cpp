#include "../src/suffixtree_functions.h"
#include <cassert>
#include <iostream>
#include <string>

int main() {
	// Test case 1: Text contains substring
	SuffixTreeContext ctx1;
	ctx1.end_marker = 0x03;
	set_text(ctx1, "hello world");
	assert(contains(ctx1, "hello"));
	assert(contains(ctx1, "world"));
	assert(contains(ctx1, "o w"));
	
	// Test case 2: Text does not contain substring
	SuffixTreeContext ctx2;
	ctx2.end_marker = 0x03;
	set_text(ctx2, "test");
	assert(!contains(ctx2, "xyz"));
	assert(!contains(ctx2, "testing"));
	
	// Test case 3: Empty substring
	SuffixTreeContext ctx3;
	ctx3.end_marker = 0x03;
	set_text(ctx3, "abc");
	assert(contains(ctx3, ""));
	
	// Test case 4: Single character
	SuffixTreeContext ctx4;
	ctx4.end_marker = 0x03;
	set_text(ctx4, "abcdef");
	assert(contains(ctx4, "c"));
	assert(!contains(ctx4, "x"));
	
	// Test case 5: Contains entire text
	SuffixTreeContext ctx5;
	ctx5.end_marker = 0x03;
	set_text(ctx5, "test");
	assert(contains(ctx5, "test"));
	
	// Test case 6: Empty text
	SuffixTreeContext ctx6;
	ctx6.end_marker = 0x03;
	set_text(ctx6, "");
	assert(contains(ctx6, ""));
	assert(!contains(ctx6, "a"));
	
	// Test case 7: Pattern longer than text
	SuffixTreeContext ctx7;
	ctx7.end_marker = 0x03;
	set_text(ctx7, "ab");
	assert(!contains(ctx7, "abcdef"));
	
	// Test case 8: Special characters
	SuffixTreeContext ctx8;
	ctx8.end_marker = 0x03;
	set_text(ctx8, "a@b#c$d");
	assert(contains(ctx8, "@b#"));
	assert(contains(ctx8, "#c$"));
	assert(!contains(ctx8, "@#"));
	
	// Test case 9: Repeated characters
	SuffixTreeContext ctx9;
	ctx9.end_marker = 0x03;
	set_text(ctx9, "aaaaaaa");
	assert(contains(ctx9, "aaa"));
	assert(contains(ctx9, "aaaaaaa"));
	assert(!contains(ctx9, "aaaaaaaa"));
	
	// Test case 10: Large text stress test
	SuffixTreeContext ctx10;
	ctx10.end_marker = 0x03;
	std::string large_text(5000, 'x');
	large_text += "needle";
	large_text += std::string(5000, 'x');
	set_text(ctx10, large_text);
	assert(contains(ctx10, "needle"));
	assert(contains(ctx10, "xxx"));
	assert(!contains(ctx10, "haystack"));
	
	std::cout << "All tests passed!" << std::endl;
	return 0;
}
