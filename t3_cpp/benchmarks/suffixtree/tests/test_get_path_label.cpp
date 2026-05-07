#include "../src/suffixtree_functions.h"
#include <cassert>
#include <iostream>

int main() {
	// Test case 1: Get path label from root
	SuffixTreeContext ctx1;
	ctx1.end_marker = 0x03;
	set_text(ctx1, "abc");
	std::string label1 = get_path_label(ctx1, ctx1.root);
	assert(label1.empty()); // Root has empty path
	
	// Test case 2: Get path label from child - label is non-empty and starts correctly
	SuffixTreeContext ctx2;
	ctx2.end_marker = 0x03;
	set_text(ctx2, "banana");
	if(!ctx2.root->children.empty()) {
		auto child = ctx2.root->children.begin()->second;
		std::string label = get_path_label(ctx2, child);
		assert(!label.empty());
		// first char of path label must match the edge key
		char key = ctx2.root->children.begin()->first;
		assert(label[0] == key);
	}
	
	// Test case 3: Manual tree with known path
	SuffixTreeContext ctx3;
	ctx3.end_marker = 0x03;
	ctx3.text = "abc";
	ctx3.text.push_back(0x03);
	ctx3.end_of_text = 4;
	ctx3.root = std::make_shared<Node>();
	
	auto child1 = std::make_shared<Node>();
	child1->parent = ctx3.root;
	child1->text_begin = 0;
	child1->text_end = 1;
	
	auto child2 = std::make_shared<Node>();
	child2->parent = child1;
	child2->text_begin = 1;
	child2->text_end = 2;
	
	std::string path = get_path_label(ctx3, child2);
	assert(path == "ab");
	
	// Test case 4: Single character path
	SuffixTreeContext ctx4;
	ctx4.end_marker = 0x03;
	ctx4.text = "x";
	ctx4.text.push_back(0x03);
	ctx4.end_of_text = 2;
	ctx4.root = std::make_shared<Node>();
	
	auto single_child = std::make_shared<Node>();
	single_child->parent = ctx4.root;
	single_child->text_begin = 0;
	single_child->text_end = 1;
	
	std::string single_path = get_path_label(ctx4, single_child);
	assert(single_path == "x");
	
	// Test case 5: Deep path (3 levels)
	SuffixTreeContext ctx5;
	ctx5.end_marker = 0x03;
	ctx5.text = "abcdef";
	ctx5.text.push_back(0x03);
	ctx5.end_of_text = 7;
	ctx5.root = std::make_shared<Node>();
	
	auto level1 = std::make_shared<Node>();
	level1->parent = ctx5.root;
	level1->text_begin = 0;
	level1->text_end = 2; // "ab"
	
	auto level2 = std::make_shared<Node>();
	level2->parent = level1;
	level2->text_begin = 2;
	level2->text_end = 4; // "cd"
	
	auto level3 = std::make_shared<Node>();
	level3->parent = level2;
	level3->text_begin = 4;
	level3->text_end = 6; // "ef"
	
	std::string deep_path = get_path_label(ctx5, level3);
	assert(deep_path == "abcdef");
	
	// Test case 6: Path with special characters
	SuffixTreeContext ctx6;
	ctx6.end_marker = 0x03;
	ctx6.text = "a@b#c";
	ctx6.text.push_back(0x03);
	ctx6.end_of_text = 6;
	ctx6.root = std::make_shared<Node>();
	
	auto special_child = std::make_shared<Node>();
	special_child->parent = ctx6.root;
	special_child->text_begin = 0;
	special_child->text_end = 3; // "a@b"
	
	std::string special_path = get_path_label(ctx6, special_child);
	assert(special_path == "a@b");

	// Test case 7: Leaf node path label includes end marker byte
	// get_edge_label on a leaf uses get_text_end(end_of_text), not raw text_end=0
	SuffixTreeContext ctx7;
	ctx7.end_marker = 0x03;
	set_text(ctx7, "ab");
	if (ctx7.root->children.count('b')) {
		auto leaf = ctx7.root->children.at('b');
		std::string lbl = get_path_label(ctx7, leaf);
		assert(lbl.size() == 2);
		assert(lbl[0] == 'b');
		assert((unsigned char)lbl[1] == 0x03);
	}

	std::cout << "All tests passed!" << std::endl;
	return 0;
}
