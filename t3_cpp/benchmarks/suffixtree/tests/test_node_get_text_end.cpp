#include "../src/suffixtree_functions.h"
#include <cassert>
#include <iostream>

int main() {
	// Test case 1: Node with text_end = 0 (uses end_of_text)
	Node node1;
	node1.text_begin = 0;
	node1.text_end = 0;
	size_t result1 = node_get_text_end(node1, 10);
	assert(result1 == 10);
	
	// Test case 2: Node with explicit text_end
	Node node2;
	node2.text_begin = 0;
	node2.text_end = 5;
	size_t result2 = node_get_text_end(node2, 10);
	assert(result2 == 5);
	
	// Test case 3: Node with text_end = 0 and different end_of_text
	Node node3;
	node3.text_end = 0;
	size_t result3 = node_get_text_end(node3, 100);
	assert(result3 == 100);
	
	// Test case 4: Node with text_end = 1
	Node node4;
	node4.text_end = 1;
	size_t result4 = node_get_text_end(node4, 50);
	assert(result4 == 1);
	
	// Test case 5: Boundary - text_end equals end_of_text
	Node node5;
	node5.text_end = 100;
	size_t result5 = node_get_text_end(node5, 100);
	assert(result5 == 100);
	
	// Test case 6: Large end_of_text value
	Node node6;
	node6.text_end = 0;
	size_t result6 = node_get_text_end(node6, 1000000);
	assert(result6 == 1000000);
	
	// Test case 7: text_end larger than end_of_text (internal node)
	Node node7;
	node7.text_end = 50;
	size_t result7 = node_get_text_end(node7, 30);
	assert(result7 == 50); // Uses explicit text_end
	
	// Test case 8: Zero end_of_text with zero text_end
	Node node8;
	node8.text_end = 0;
	size_t result8 = node_get_text_end(node8, 0);
	assert(result8 == 0);
	
	std::cout << "All tests passed!" << std::endl;
	return 0;
}
