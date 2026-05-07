#include "../src/suffixtree_functions.h"
#include <cassert>
#include <iostream>

int main() {
	// Test case 1: Manual node - exact label content
	SuffixTreeContext ctx1;
	ctx1.end_marker = 0x03;
	ctx1.text = "test";
	ctx1.text.push_back(0x03);
	ctx1.end_of_text = 5;

	auto node1 = std::make_shared<Node>();
	node1->text_begin = 0;
	node1->text_end = 2;
	assert(get_edge_label(ctx1, node1) == "te");

	// Test case 2: Single character edge
	auto node2 = std::make_shared<Node>();
	node2->text_begin = 1;
	node2->text_end = 2;
	assert(get_edge_label(ctx1, node2) == "e");

	// Test case 3: Full text edge (excluding end marker)
	auto node3 = std::make_shared<Node>();
	node3->text_begin = 0;
	node3->text_end = 4;
	assert(get_edge_label(ctx1, node3) == "test");

	// Test case 4: Leaf node (text_end = 0 means use end_of_text)
	auto node4 = std::make_shared<Node>();
	node4->text_begin = 2;
	node4->text_end = 0; // leaf
	std::string label4 = get_edge_label(ctx1, node4);
	assert(label4 == "st\x03"); // from index 2 to end_of_text=5

	// Test case 5: Edge in banana tree - verify 'b' edge starts with 'b'
	SuffixTreeContext ctx5;
	ctx5.end_marker = 0x03;
	set_text(ctx5, "banana");
	// Find the child reachable via 'b'
	auto b_child = ctx5.root->children.count('b') ? ctx5.root->children.at('b') : nullptr;
	if (b_child) {
		std::string label = get_edge_label(ctx5, b_child);
		assert(!label.empty());
		assert(label[0] == 'b');
	}

	// Test case 6: Edge in "abc" tree - 'a' child label starts with 'a'
	SuffixTreeContext ctx6;
	ctx6.end_marker = 0x03;
	set_text(ctx6, "abc");
	auto a_child = ctx6.root->children.count('a') ? ctx6.root->children.at('a') : nullptr;
	if (a_child) {
		std::string label = get_edge_label(ctx6, a_child);
		assert(label[0] == 'a');
	}

	// Test case 7: Edge label length matches text_end - text_begin
	SuffixTreeContext ctx7;
	ctx7.end_marker = 0x03;
	ctx7.text = "abcdef";
	ctx7.text.push_back(0x03);
	ctx7.end_of_text = 7;
	auto node7 = std::make_shared<Node>();
	node7->text_begin = 1;
	node7->text_end = 4;
	std::string label7 = get_edge_label(ctx7, node7);
	assert(label7.size() == 3);
	assert(label7 == "bcd");

	std::cout << "All tests passed!" << std::endl;
	return 0;
}
