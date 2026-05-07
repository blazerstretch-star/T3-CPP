#include "../src/suffixtree_functions.h"
#include <cassert>
#include <iostream>

int main() {
	// Test case 1: Copy assign simple context - verify content
	SuffixTreeContext ctx1;
	ctx1.end_marker = 0x03;
	set_text(ctx1, "abc");

	SuffixTreeContext ctx2 = copy_assign_context(ctx1);
	assert(ctx2.end_marker == ctx1.end_marker);
	assert(get_text(ctx2) == "abc");
	assert(ctx2.root != nullptr);

	// Test case 2: Copy is independent - modifying copy does not affect original
	SuffixTreeContext ctx3;
	ctx3.end_marker = 0x03;
	set_text(ctx3, "hello");

	SuffixTreeContext ctx4 = copy_assign_context(ctx3);
	set_text(ctx4, "world");
	assert(get_text(ctx3) == "hello"); // original unchanged
	assert(get_text(ctx4) == "world");

	// Test case 3: Copy has independent root pointer
	SuffixTreeContext ctx5;
	ctx5.end_marker = 0x03;
	set_text(ctx5, "banana");

	SuffixTreeContext ctx6 = copy_assign_context(ctx5);
	assert(ctx5.root != ctx6.root); // different root objects

	// Test case 4: Copy assign with pattern search works correctly
	SuffixTreeContext ctx7;
	ctx7.end_marker = 0x03;
	set_text(ctx7, "banana");

	SuffixTreeContext ctx8 = copy_assign_context(ctx7);
	auto result = find(ctx8, "ana");
	assert(result.size() == 2);

	// Test case 5: Empty context copy
	SuffixTreeContext ctx9;
	ctx9.end_marker = 0x03;
	set_text(ctx9, "");

	SuffixTreeContext ctx10 = copy_assign_context(ctx9);
	assert(get_text(ctx10) == "");

	// Test case 6: Copy preserves custom end_marker
	SuffixTreeContext ctx11;
	ctx11.end_marker = '$';
	set_text(ctx11, "test");
	SuffixTreeContext ctx12 = copy_assign_context(ctx11);
	assert(ctx12.end_marker == '$');
	assert(get_text(ctx12) == "test");
	// copy with custom marker must be functional: set_text uses the copied marker
	bool threw = false;
	try { set_text(ctx12, "he$llo"); } catch (const std::logic_error&) { threw = true; }
	assert(threw); // '$' is the marker, must reject
	bool ok = true;
	try { set_text(ctx12, "world"); } catch (...) { ok = false; }
	assert(ok);
	assert(get_text(ctx12) == "world");

	std::cout << "All tests passed!" << std::endl;
	return 0;
}
