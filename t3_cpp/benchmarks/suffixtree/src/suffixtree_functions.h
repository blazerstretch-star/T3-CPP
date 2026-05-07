#ifndef SUFFIXTREE_FUNCTIONS_H
#define SUFFIXTREE_FUNCTIONS_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>

// Node structure
struct Node {
	std::unordered_map<char, std::shared_ptr<Node>> children;
	std::weak_ptr<Node> parent;
	std::weak_ptr<Node> suffix_link;
	size_t text_begin = 0;
	size_t text_end = 0;
	size_t suffix_start = 0;
	
	size_t get_text_end(size_t end_of_text) const;
};

// SuffixTree context structure
struct SuffixTreeContext {
	char end_marker;
	std::string text;
	std::shared_ptr<Node> root;
	size_t end_of_text;
};

// Function declarations (15 functions)
// Easy functions (4)
std::string get_text(const SuffixTreeContext &ctx);
std::string get_edge_label(const SuffixTreeContext &ctx, std::shared_ptr<const Node> node);
size_t node_get_text_end(const Node &node, size_t end_of_text);
bool contains(const SuffixTreeContext &ctx, const std::string &needle);

// Medium functions (8)
std::vector<size_t> find(const SuffixTreeContext &ctx, const std::string &needle);
bool ends_with(const SuffixTreeContext &ctx, const std::string &suffix);
std::shared_ptr<const Node> traverse_tree(const SuffixTreeContext &ctx, const std::string &find_text, size_t &position, size_t end);
std::shared_ptr<Node> traverse_node(const SuffixTreeContext &ctx, std::shared_ptr<Node> node, const std::string &find_text, size_t &position, size_t end);
std::string get_path_label(const SuffixTreeContext &ctx, std::shared_ptr<const Node> node);
void check_suffix_links(const SuffixTreeContext &ctx);
void relabel_text_end(SuffixTreeContext &ctx);
void set_text(SuffixTreeContext &ctx, const std::string &text_);

// Hard functions (3)
void rebuild(SuffixTreeContext &ctx);
SuffixTreeContext copy_assign_context(const SuffixTreeContext &other);
SuffixTreeContext move_assign_context(SuffixTreeContext &other);

#endif
