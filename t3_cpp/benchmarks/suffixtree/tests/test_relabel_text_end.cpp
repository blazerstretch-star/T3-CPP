#include "../src/suffixtree_functions.h"
#include <cassert>
#include <iostream>

int main() {
	// Test case 1: Relabel after rebuild
	SuffixTreeContext ctx1;
	ctx1.end_marker = 0x03;
	ctx1.text = "test";
	ctx1.text.push_back(0x03);
	ctx1.end_of_text = 0;
	ctx1.root = std::make_shared<Node>();
	rebuild(ctx1);
	assert(contains(ctx1, "test"));
	
	// Test case 2: Relabel simple tree - leaf gets end_of_text, root also gets relabeled
	SuffixTreeContext ctx2;
	ctx2.end_marker = 0x03;
	ctx2.text = "ab";
	ctx2.text.push_back(0x03);
	ctx2.end_of_text = 3;
	ctx2.root = std::make_shared<Node>();
	ctx2.root->text_end = 0; // root starts at 0
	auto child = std::make_shared<Node>();
	child->text_begin = 0;
	child->text_end = 0;
	ctx2.root->children['a'] = child;
	relabel_text_end(ctx2);
	assert(child->text_end == 3);
	assert(ctx2.root->text_end == 3); // root with text_end=0 also gets relabeled
	
	// Test case 3: Relabel with multiple nodes
	SuffixTreeContext ctx3;
	ctx3.end_marker = 0x03;
	ctx3.text = "abc";
	ctx3.text.push_back(0x03);
	ctx3.end_of_text = 4;
	ctx3.root = std::make_shared<Node>();
	auto child1 = std::make_shared<Node>();
	child1->text_end = 0;
	auto child2 = std::make_shared<Node>();
	child2->text_end = 0;
	ctx3.root->children['a'] = child1;
	ctx3.root->children['b'] = child2;
	relabel_text_end(ctx3);
	assert(child1->text_end == 4);
	assert(child2->text_end == 4);
	
	// Test case 4: Relabel with nested nodes
	SuffixTreeContext ctx4;
	ctx4.end_marker = 0x03;
	ctx4.text = "xyz";
	ctx4.text.push_back(0x03);
	ctx4.end_of_text = 4;
	ctx4.root = std::make_shared<Node>();
	auto parent = std::make_shared<Node>();
	parent->text_end = 2; // Internal node with explicit end
	auto leaf = std::make_shared<Node>();
	leaf->text_end = 0; // Leaf node
	ctx4.root->children['x'] = parent;
	parent->children['y'] = leaf;
	relabel_text_end(ctx4);
	assert(parent->text_end == 2); // Should not change
	assert(leaf->text_end == 4); // Should be relabeled
	
	// Test case 5: Empty tree
	SuffixTreeContext ctx5;
	ctx5.end_marker = 0x03;
	ctx5.text = "";
	ctx5.text.push_back(0x03);
	ctx5.end_of_text = 1;
	ctx5.root = std::make_shared<Node>();
	relabel_text_end(ctx5); // Should not crash
	assert(ctx5.root != nullptr);
	
	// Test case 6: Large tree stress test
	SuffixTreeContext ctx6;
	ctx6.end_marker = 0x03;
	std::string large_text(1000, 'a');
	ctx6.text = large_text;
	ctx6.text.push_back(0x03);
	rebuild(ctx6);
	assert(contains(ctx6, "aaa"));
	
	std::cout << "All tests passed!" << std::endl;
	return 0;
}
