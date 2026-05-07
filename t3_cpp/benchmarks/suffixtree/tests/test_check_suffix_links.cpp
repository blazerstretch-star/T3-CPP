#include "../src/suffixtree_functions.h"
#include <cassert>
#include <iostream>
#include <stdexcept>

int main() {
	// Test case 1: Check suffix links in simple tree
	SuffixTreeContext ctx1;
	ctx1.end_marker = 0x03;
	set_text(ctx1, "abc");
	bool passed1 = true;
	try {
		check_suffix_links(ctx1);
	} catch(const std::runtime_error &e) {
		passed1 = false;
	}
	assert(passed1);
	
	// Test case 2: Check suffix links in tree with repetitions
	SuffixTreeContext ctx2;
	ctx2.end_marker = 0x03;
	set_text(ctx2, "banana");
	bool passed2 = true;
	try {
		check_suffix_links(ctx2);
	} catch(const std::runtime_error &e) {
		passed2 = false;
	}
	assert(passed2);
	
	// Test case 3: Check suffix links in tree with all same characters
	SuffixTreeContext ctx3;
	ctx3.end_marker = 0x03;
	set_text(ctx3, "aaaa");
	bool passed3 = true;
	try {
		check_suffix_links(ctx3);
	} catch(const std::runtime_error &e) {
		passed3 = false;
	}
	assert(passed3);
	
	// Test case 4: Check suffix links in empty tree
	SuffixTreeContext ctx4;
	ctx4.end_marker = 0x03;
	set_text(ctx4, "");
	bool passed4 = true;
	try {
		check_suffix_links(ctx4);
	} catch(const std::runtime_error &e) {
		passed4 = false;
	}
	assert(passed4);
	
	// Test case 5: Verify tree structure is valid
	SuffixTreeContext ctx5;
	ctx5.end_marker = 0x03;
	set_text(ctx5, "abcabcabc");
	assert(ctx5.root != nullptr);
	assert(contains(ctx5, "abc"));
	check_suffix_links(ctx5);
	
	std::cout << "All tests passed!" << std::endl;
	return 0;
}
