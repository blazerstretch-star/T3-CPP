#include "../src/suffixtree_functions.h"
#include <cassert>
#include <iostream>

int main() {
	// Test 1: end_of_text < text.length()
	// Directly set text longer than end_of_text declares.
	// Correct impl uses end_of_text-1; naive impl uses text.length()-1.
	{
		SuffixTreeContext ctx;
		ctx.end_marker = 0x03;
		ctx.text = "longstring";
		ctx.text.push_back(0x03);   // text = "longstring\x03", length = 11
		ctx.end_of_text = 3;        // authoritative: only "lo" + marker are valid
		// Correct: get_text() == "lo"  (end_of_text-1 = 2 chars)
		// Naive:   get_text() == "longstrin" (text.length()-1 = 10 chars)
		std::string result = get_text(ctx);
		assert(result.size() == 2);
		assert(result == "lo");
	}

	// Test 2: end_of_text > 1 but text was manually shortened
	// Simulate a buffer-reuse scenario: text holds a short string but
	// end_of_text still reflects the old longer build.
	{
		SuffixTreeContext ctx;
		ctx.end_marker = 0x03;
		ctx.text = "hi";
		ctx.text.push_back(0x03);   // text = "hi\x03", length = 3
		ctx.end_of_text = 3;        // consistent here — baseline passes
		std::string result = get_text(ctx);
		assert(result == "hi");
		assert(result.size() == 2);
	}

	// Test 3: end_of_text == 1 means empty logical text (only marker)
	// text may physically hold more bytes.
	{
		SuffixTreeContext ctx;
		ctx.end_marker = 0x03;
		ctx.text = "garbage";
		ctx.text.push_back(0x03);   // text = "garbage\x03", length = 8
		ctx.end_of_text = 1;        // only the marker is "valid"
		// Correct: get_text() == ""
		// Naive:   get_text() == "garbage" (strips last of 8 chars)
		std::string result = get_text(ctx);
		assert(result.empty());
	}

	// Test 4: result must contain no end_marker byte
	// Naive impl strips text[length-1]; if end_of_text < length the marker
	// is at text[end_of_text-1], not at text[length-1], so it leaks through.
	{
		SuffixTreeContext ctx;
		ctx.end_marker = 0x03;
		ctx.text = "abc";
		ctx.text.push_back(0x03);   // marker at index 3
		ctx.text += "extra";        // text = "abc\x03extra", length = 9
		ctx.end_of_text = 4;        // logical end: "abc\x03"
		// Correct: get_text() == "abc", no marker
		// Naive:   get_text() == "abc\x03extr" — contains the marker
		std::string result = get_text(ctx);
		assert(result.find('\x03') == std::string::npos);
		assert(result == "abc");
	}

	// Test 5: size must equal end_of_text - 1 exactly
	{
		SuffixTreeContext ctx;
		ctx.end_marker = 0x03;
		ctx.text = "hello world padding padding";
		ctx.text.push_back(0x03);
		ctx.end_of_text = 6;        // logical text is "hello"
		std::string result = get_text(ctx);
		assert(result.size() == ctx.end_of_text - 1);
		assert(result == "hello");
	}

	std::cout << "All tests passed!" << std::endl;
	return 0;
}
