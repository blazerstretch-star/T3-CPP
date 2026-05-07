#include "../src/suffixtree_functions.h"
#include <cassert>
#include <iostream>
#include <stdexcept>

int main() {
	// Test 1: Basic round-trip - exact content and length
	SuffixTreeContext ctx1;
	ctx1.end_marker = 0x03;
	set_text(ctx1, "hello");
	std::string r1 = get_text(ctx1);
	assert(r1 == "hello");
	assert(r1.size() == 5); // not 6 (no end marker appended)

	// Test 2: End marker must not appear in result
	assert(r1.find('\x03') == std::string::npos);

	// Test 3: Empty string round-trip
	SuffixTreeContext ctx3;
	ctx3.end_marker = 0x03;
	set_text(ctx3, "");
	std::string r3 = get_text(ctx3);
	assert(r3 == "");
	assert(r3.size() == 0); // not 1 (not just the marker)

	// Test 4: Special characters and spaces preserved exactly
	SuffixTreeContext ctx4;
	ctx4.end_marker = 0x03;
	set_text(ctx4, "a b c");
	assert(get_text(ctx4) == "a b c");
	assert(get_text(ctx4).size() == 5);

	// Test 5: Longer text round-trip
	SuffixTreeContext ctx5;
	ctx5.end_marker = 0x03;
	set_text(ctx5, "abracadabra");
	assert(get_text(ctx5) == "abracadabra");
	assert(get_text(ctx5).size() == 11);

	// Test 6: Non-default end marker - get_text strips the right marker
	SuffixTreeContext ctx6;
	ctx6.end_marker = '$';
	set_text(ctx6, "hello");
	std::string r6 = get_text(ctx6);
	assert(r6 == "hello");
	assert(r6.find('$') == std::string::npos);

	// Test 7: Non-default end marker with 0x01
	SuffixTreeContext ctx7;
	ctx7.end_marker = 0x01;
	set_text(ctx7, "world");
	std::string r7 = get_text(ctx7);
	assert(r7 == "world");
	assert(r7.find('\x01') == std::string::npos);

	// Test 8: Idempotency - calling get_text twice returns identical result
	SuffixTreeContext ctx8;
	ctx8.end_marker = 0x03;
	set_text(ctx8, "banana");
	assert(get_text(ctx8) == get_text(ctx8));
	assert(get_text(ctx8) == "banana");

	// Test 9: After overwrite, get_text reflects new text, not old
	SuffixTreeContext ctx9;
	ctx9.end_marker = 0x03;
	set_text(ctx9, "first");
	set_text(ctx9, "second");
	assert(get_text(ctx9) == "second");
	assert(get_text(ctx9).find("first") == std::string::npos);

	// Test 10: After overwrite with empty, get_text returns empty
	SuffixTreeContext ctx10;
	ctx10.end_marker = 0x03;
	set_text(ctx10, "nonempty");
	set_text(ctx10, "");
	assert(get_text(ctx10) == "");
	assert(get_text(ctx10).size() == 0);

	// Test 11: String whose last real char is visually close to a control char
	// Ensures no off-by-one over-stripping
	SuffixTreeContext ctx11;
	ctx11.end_marker = 0x03;
	set_text(ctx11, "ab\x02"); // last char is 0x02, end_marker is 0x03
	std::string r11 = get_text(ctx11);
	assert(r11 == "ab\x02");   // 0x02 must NOT be stripped
	assert(r11.size() == 3);

	// Test 12: Internal state not mutated - ctx.text still has marker after get_text
	SuffixTreeContext ctx12;
	ctx12.end_marker = 0x03;
	set_text(ctx12, "test");
	get_text(ctx12); // call once
	assert(ctx12.text.back() == 0x03); // internal text unchanged
	assert(get_text(ctx12) == "test"); // second call still correct

	// Tip 8/9: set_text must throw logic_error when input contains the end marker
	// (whether it's the only char or embedded mid-string)
	{
		SuffixTreeContext ctx;
		ctx.end_marker = 0x03;
		bool threw = false;
		try { set_text(ctx, std::string(1, '\x03')); } catch (const std::logic_error&) { threw = true; }
		assert(threw); // input IS the end marker - must reject
	}
	{
		SuffixTreeContext ctx;
		ctx.end_marker = 0x03;
		bool threw = false;
		try { set_text(ctx, "hello\x03world"); } catch (const std::logic_error&) { threw = true; }
		assert(threw); // embedded marker mid-string - must reject
	}

	// Tip 10: shrink text - get_text must use actual stored length, not stale text.size()
	{
		SuffixTreeContext ctx;
		ctx.end_marker = 0x03;
		set_text(ctx, "abc");
		set_text(ctx, "ab"); // shorter replacement
		std::string r = get_text(ctx);
		assert(r == "ab");
		assert(r.size() == 2); // not 3 ("abc") or 3 ("ab\x03c")
	}

	// Tip 11: repeated set_text mutations - no stale state
	{
		SuffixTreeContext ctx;
		ctx.end_marker = 0x03;
		set_text(ctx, "first");
		const char* texts[] = {"second", "third", "", "fourth"};
		for (auto s : texts) {
			set_text(ctx, s);
			std::string r = get_text(ctx);
			assert(r == s);
			assert(r.size() == std::string(s).size());
		}
	}

	// Tip 12: get_text and find() must agree on content and positions
	{
		SuffixTreeContext ctx;
		ctx.end_marker = 0x03;
		set_text(ctx, "banana");
		std::string recovered = get_text(ctx);
		auto positions = find(ctx, "ana");
		assert(positions.size() == 2);
		for (size_t pos : positions) {
			assert(recovered.substr(pos, 3) == "ana");
		}
	}

	// Tip 15: null byte in input - must be preserved, not truncated
	{
		SuffixTreeContext ctx;
		ctx.end_marker = 0x03;
		std::string s("ab\0cd", 5);
		set_text(ctx, s);
		std::string r = get_text(ctx);
		assert(r == s);
		assert(r.size() == 5);
	}

	// Tip 16: check_suffix_links must not throw after get_text is called
	{
		SuffixTreeContext ctx;
		ctx.end_marker = 0x03;
		set_text(ctx, "mississippi");
		get_text(ctx);
		check_suffix_links(ctx); // must not throw
	}

	// Tip 17: pathological input - 10000 identical chars
	{
		SuffixTreeContext ctx;
		ctx.end_marker = 0x03;
		std::string worst(10000, 'a');
		set_text(ctx, worst);
		std::string r = get_text(ctx);
		assert(r == worst);
		assert(r.size() == 10000);
	}

	std::cout << "All tests passed!" << std::endl;
	return 0;
}
