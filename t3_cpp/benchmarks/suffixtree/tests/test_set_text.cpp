#include "../src/suffixtree_functions.h"
#include <cassert>
#include <iostream>
#include <stdexcept>

int main() {
	// Test case 1: Set simple text - tree is functional
	SuffixTreeContext ctx1;
	ctx1.end_marker = 0x03;
	set_text(ctx1, "test");
	assert(get_text(ctx1) == "test");
	assert(ctx1.root != nullptr);
	assert(contains(ctx1, "test"));
	assert(contains(ctx1, "est"));
	assert(contains(ctx1, "st"));
	assert(contains(ctx1, "t"));

	// Test case 2: Set empty text
	SuffixTreeContext ctx2;
	ctx2.end_marker = 0x03;
	set_text(ctx2, "");
	assert(get_text(ctx2) == "");

	// Test case 3: Set text with end marker should throw logic_error
	SuffixTreeContext ctx3;
	ctx3.end_marker = 0x03;
	bool threw = false;
	try {
		std::string bad_text = "abc";
		bad_text.push_back(0x03);
		set_text(ctx3, bad_text);
	} catch(const std::logic_error &e) {
		threw = true;
	}
	assert(threw);

	// Test case 4: Overwrite existing text - old content gone, new content searchable
	SuffixTreeContext ctx4;
	ctx4.end_marker = 0x03;
	set_text(ctx4, "first");
	set_text(ctx4, "second");
	assert(get_text(ctx4) == "second");
	assert(contains(ctx4, "second"));
	assert(!contains(ctx4, "first")); // old text must be gone

	// Test case 5: Text with end marker only at start should throw
	SuffixTreeContext ctx5;
	ctx5.end_marker = 0x03;
	bool threw5 = false;
	try {
		std::string bad_text;
		bad_text.push_back(0x03);
		bad_text += "abc";
		set_text(ctx5, bad_text);
	} catch(const std::logic_error &e) {
		threw5 = true;
	}
	assert(threw5);

	// Test case 6: Set text rebuilds tree correctly (find works after set)
	SuffixTreeContext ctx6;
	ctx6.end_marker = 0x03;
	set_text(ctx6, "banana");
	auto result = find(ctx6, "ana");
	assert(result.size() == 2);

	// Test case 7: Internal text field has end marker appended
	SuffixTreeContext ctx7;
	ctx7.end_marker = 0x03;
	set_text(ctx7, "hello");
	assert(ctx7.text.back() == 0x03);
	assert(ctx7.text.size() == 6); // "hello" + end_marker

	// Test case 8: Custom end_marker - throws when that marker appears in input
	SuffixTreeContext ctx8;
	ctx8.end_marker = '$';
	bool threw8 = false;
	try {
		set_text(ctx8, "he$llo");
	} catch(const std::logic_error &e) {
		threw8 = true;
	}
	assert(threw8);

	// Test case 9: Custom end_marker - does NOT throw for \x03 (not the marker)
	SuffixTreeContext ctx9;
	ctx9.end_marker = '$';
	bool threw9 = false;
	try {
		set_text(ctx9, "he\x03llo"); // \x03 is not the marker here
	} catch(...) {
		threw9 = true;
	}
	assert(!threw9);
	assert(get_text(ctx9) == "he\x03llo");
	assert(get_text(ctx9).size() == 6);  // "he\x03llo" is 6 chars (\x03 is one char)

	std::cout << "All tests passed!" << std::endl;
	return 0;
}
