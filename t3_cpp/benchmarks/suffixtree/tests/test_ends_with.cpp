#include "../src/suffixtree_functions.h"
#include <cassert>
#include <iostream>

int main() {
	// Test case 1: Text ends with suffix
	SuffixTreeContext ctx1;
	ctx1.end_marker = 0x03;
	set_text(ctx1, "hello");
	assert(ends_with(ctx1, "llo"));
	assert(ends_with(ctx1, "o"));
	assert(ends_with(ctx1, "hello"));

	// Test case 2: Text does not end with suffix
	SuffixTreeContext ctx2;
	ctx2.end_marker = 0x03;
	set_text(ctx2, "world");
	assert(!ends_with(ctx2, "wor"));
	assert(!ends_with(ctx2, "hello"));

	// Test case 3: Empty suffix
	SuffixTreeContext ctx3;
	ctx3.end_marker = 0x03;
	set_text(ctx3, "test");
	assert(ends_with(ctx3, ""));

	// Test case 4: Suffix longer than text
	SuffixTreeContext ctx4;
	ctx4.end_marker = 0x03;
	set_text(ctx4, "ab");
	assert(!ends_with(ctx4, "abc"));

	// Test case 5: Substring that appears in middle but not at end
	SuffixTreeContext ctx5;
	ctx5.end_marker = 0x03;
	set_text(ctx5, "abcdef");
	assert(!ends_with(ctx5, "abc")); // "abc" is in text but not at end
	assert(ends_with(ctx5, "def"));
	assert(ends_with(ctx5, "cdef"));

	// Test case 6: Repeated pattern - only true if at end
	SuffixTreeContext ctx6;
	ctx6.end_marker = 0x03;
	set_text(ctx6, "ababab");
	assert(ends_with(ctx6, "ab"));
	assert(ends_with(ctx6, "abab"));
	assert(!ends_with(ctx6, "ba")); // "ba" appears but not at end

	// Test case 7: Single character text
	SuffixTreeContext ctx7;
	ctx7.end_marker = 0x03;
	set_text(ctx7, "x");
	assert(ends_with(ctx7, "x"));
	assert(!ends_with(ctx7, "y"));

	// Test case 8: Empty text
	SuffixTreeContext ctx8;
	ctx8.end_marker = 0x03;
	set_text(ctx8, "");
	assert(ends_with(ctx8, ""));
	assert(!ends_with(ctx8, "a"));

	// Test case 9: Special characters at end
	SuffixTreeContext ctx9;
	ctx9.end_marker = 0x03;
	set_text(ctx9, "hello world!");
	assert(ends_with(ctx9, "world!"));
	assert(ends_with(ctx9, "!"));
	assert(!ends_with(ctx9, "world"));

	// Test case 10: banana - "ana" is in text but not at end
	SuffixTreeContext ctx10;
	ctx10.end_marker = 0x03;
	set_text(ctx10, "banana");
	assert(ends_with(ctx10, "ana"));
	assert(ends_with(ctx10, "banana"));
	assert(!ends_with(ctx10, "ban"));
	assert(!ends_with(ctx10, "nan"));

	std::cout << "All tests passed!" << std::endl;
	return 0;
}
