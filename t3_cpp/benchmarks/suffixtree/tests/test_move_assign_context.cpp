#include "../src/suffixtree_functions.h"
#include <cassert>
#include <iostream>

int main() {
	// Test case 1: Move assign - destination has correct text
	SuffixTreeContext ctx1;
	ctx1.end_marker = 0x03;
	set_text(ctx1, "test");

	SuffixTreeContext ctx2 = move_assign_context(ctx1);
	assert(get_text(ctx2) == "test");
	assert(ctx2.root != nullptr);

	// Test case 2: Move assign - source is emptied
	SuffixTreeContext ctx3;
	ctx3.end_marker = 0x03;
	set_text(ctx3, "hello");

	SuffixTreeContext ctx4 = move_assign_context(ctx3);
	assert(get_text(ctx3) == ""); // source must be empty after move

	// Test case 3: Move assign - source root is null or empty after move
	SuffixTreeContext ctx5;
	ctx5.end_marker = 0x03;
	set_text(ctx5, "world");

	SuffixTreeContext ctx6 = move_assign_context(ctx5);
	assert(ctx5.root == nullptr || ctx5.root->children.empty());

	// Test case 4: Moved-to context is fully functional
	SuffixTreeContext ctx7;
	ctx7.end_marker = 0x03;
	set_text(ctx7, "banana");

	SuffixTreeContext ctx8 = move_assign_context(ctx7);
	assert(contains(ctx8, "ana"));
	assert(contains(ctx8, "ban"));
	assert(!contains(ctx8, "xyz"));

	// Test case 5: Move empty context
	SuffixTreeContext ctx9;
	ctx9.end_marker = 0x03;
	set_text(ctx9, "");

	SuffixTreeContext ctx10 = move_assign_context(ctx9);
	assert(get_text(ctx10) == "");
	assert(get_text(ctx9) == "");

	// Test case 6: Custom end_marker preserved in destination, source keeps its marker
	SuffixTreeContext ctx11;
	ctx11.end_marker = '$';
	set_text(ctx11, "abc");
	SuffixTreeContext ctx12 = move_assign_context(ctx11);
	assert(ctx12.end_marker == '$');
	assert(get_text(ctx12) == "abc");
	// source marker is also '$' (not reset to 0x03)
	assert(ctx11.end_marker == '$');
	// destination is functional with its marker
	bool threw = false;
	try { set_text(ctx12, "he$llo"); } catch (const std::logic_error&) { threw = true; }
	assert(threw);

	std::cout << "All tests passed!" << std::endl;
	return 0;
}
