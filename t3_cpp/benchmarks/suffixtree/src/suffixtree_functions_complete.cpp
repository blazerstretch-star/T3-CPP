#include "suffixtree_functions.h"
#include <stdexcept>
#include <algorithm>

// Helper function for Node
size_t Node::get_text_end(size_t end_of_text) const {
	// FUNCTION_ID: suffixtree_func015 - START
	if(text_end == 0){
		return end_of_text;
	}
	return text_end;
	// FUNCTION_ID: suffixtree_func015 - END
}

// Function 1: Copy assignment operator (Hard)
SuffixTreeContext copy_assign_context(const SuffixTreeContext &other){
	// FUNCTION_ID: suffixtree_func001 - START
	SuffixTreeContext ctx;
	ctx.end_marker = other.end_marker;
	ctx.text = other.text;
	if(!ctx.text.empty()){
		ctx.text.pop_back();
	}
	set_text(ctx, ctx.text);
	return ctx;
	// FUNCTION_ID: suffixtree_func001 - END
}

// Function 2: Move assignment operator (Hard)
SuffixTreeContext move_assign_context(SuffixTreeContext &other){
	// FUNCTION_ID: suffixtree_func002 - START
	SuffixTreeContext ctx;
	ctx.end_marker = std::move(other.end_marker);
	ctx.text = std::move(other.text);
	ctx.root = std::move(other.root);
	ctx.end_of_text = std::move(other.end_of_text);
	other.text = "";
	other.text.push_back(ctx.end_marker);
	other.root = std::make_shared<Node>();
	other.end_of_text = 0;
	return ctx;
	// FUNCTION_ID: suffixtree_func002 - END
}

// Function 3: Get text (Easy)
std::string get_text(const SuffixTreeContext &ctx){
	// FUNCTION_ID: suffixtree_func003 - START
	if(ctx.end_of_text == 0){
		return "";
	}
	return ctx.text.substr(0, ctx.end_of_text - 1);
	// FUNCTION_ID: suffixtree_func003 - END
}

// Function 4: Set text (Medium)
void set_text(SuffixTreeContext &ctx, const std::string &text_){
	// FUNCTION_ID: suffixtree_func004 - START
	if(text_.find(ctx.end_marker) != std::string::npos){
		throw std::logic_error("text contains end marker");
	}
	ctx.text = text_;
	ctx.text.push_back(ctx.end_marker);
	rebuild(ctx);
	// FUNCTION_ID: suffixtree_func004 - END
}

// Function 5: Find pattern (Medium)
std::vector<size_t> find(const SuffixTreeContext &ctx, const std::string &needle){
	// FUNCTION_ID: suffixtree_func005 - START
	size_t pos = 0;
	const size_t end = needle.size();
	auto stop = traverse_tree(ctx, needle, pos, end);
	if(pos < end){
		return {};
	}

	std::vector<size_t> result;
	std::queue<std::shared_ptr<const Node>> todo;
	todo.push(stop);
	bool cant_be_root = false;
	while(!todo.empty()){
		auto current_node = todo.front();
		todo.pop();
		if(current_node->children.empty() && (cant_be_root || stop->parent.lock())){
			result.push_back(current_node->suffix_start);
		}
		for(const auto &entry : current_node->children){
			todo.push(entry.second);
		}
		cant_be_root = true;
	}
	return result;
	// FUNCTION_ID: suffixtree_func005 - END
}

// Function 6: Ends with suffix (Medium)
bool ends_with(const SuffixTreeContext &ctx, const std::string &suffix){
	// FUNCTION_ID: suffixtree_func006 - START
	auto suffix_ = suffix;
	suffix_.push_back(ctx.end_marker);
	size_t pos = 0;
	const size_t end = suffix_.size();
	auto stop = traverse_tree(ctx, suffix_, pos, end);
	return (pos >= end && stop->children.empty() && stop->parent.lock());
	// FUNCTION_ID: suffixtree_func006 - END
}

// Function 7: Contains substring (Easy)
bool contains(const SuffixTreeContext &ctx, const std::string &needle){
	// FUNCTION_ID: suffixtree_func007 - START
	size_t pos = 0;
	const size_t end = needle.size();
	auto stop = traverse_tree(ctx, needle, pos, end);
	return (pos >= end);
	// FUNCTION_ID: suffixtree_func007 - END
}

// Function 8: Check suffix links (Medium)
void check_suffix_links(const SuffixTreeContext &ctx){
	// FUNCTION_ID: suffixtree_func008 - START
	std::queue<std::shared_ptr<const Node>> todo;
	todo.push(ctx.root);
	while(!todo.empty()){
		auto current_node = todo.front();
		todo.pop();

		if(current_node != ctx.root && !current_node->children.empty()){
			auto linked_node = current_node->suffix_link.lock();
			if(!linked_node){
				throw std::runtime_error("Inner node does not have a suffix link");
			}
			if(linked_node == ctx.root && (current_node->parent.lock() != ctx.root || current_node->text_begin+1 != current_node->text_end)){
				throw std::runtime_error("Wrong suffix link to root");
			}else{
				const auto current_label = get_path_label(ctx, current_node);
				const auto linked_label = get_path_label(ctx, linked_node);
				if(current_label.size() != linked_label.size()+1 || current_label.substr(1) != linked_label){
					throw std::runtime_error("Wrong suffix link");
				}
			}
		}
		for(const auto &entry : current_node->children){
			todo.push(entry.second);
		}
	}
	// FUNCTION_ID: suffixtree_func008 - END
}

// Function 9: Rebuild tree using Ukkonen's algorithm (Hard)
void rebuild(SuffixTreeContext &ctx){
	// FUNCTION_ID: suffixtree_func009 - START
	ctx.end_of_text = 0;
	ctx.root = std::make_shared<Node>();
	size_t next_k = 0;
	std::shared_ptr<Node> link_wanted;
	size_t link_wanted_position = 0;
	for(size_t i=0; i<ctx.text.length(); i++){
		std::shared_ptr<Node> link_follow;
		size_t link_follow_position = 0;
		const size_t k = next_k;
		ctx.end_of_text++;
		next_k = i+1;

		for(size_t j=k; j<=i; j++){
			std::shared_ptr<Node> current_node, next_node;
			size_t current_position, next_position;
			if(link_follow){
				if(auto link = link_follow->suffix_link.lock()){
					current_node = link;
					current_position = link_follow_position;
					link_follow = nullptr;
				}else{
					throw std::runtime_error("A suffix link we wanted to follow does not exist");
				}
			}else{
				current_node = ctx.root;
				current_position = j;
			}

			next_node = current_node;
			next_position = current_position;
			do{
				if(link_wanted && link_wanted_position == next_position){
					link_wanted->suffix_link = next_node;
					link_wanted = nullptr;
				}
				if(next_node->suffix_link.lock()){
					link_follow = next_node;
					link_follow_position = next_position;
				}
				current_node = next_node;
				current_position = next_position;
				next_node = traverse_node(ctx, current_node, ctx.text, next_position, i+1);
			}while(next_node && next_position <= i && current_node != next_node && next_node->get_text_end(ctx.end_of_text) - next_node->text_begin == next_position - current_position);

			if(!next_node){
				// Rule 1
			}else if(next_position > i){
				// Rule 3
				next_k = j;
				break;
			}else{
				// Rule 2
				if(current_node == next_node){
					auto new_node = std::make_shared<Node>();
					new_node->parent = current_node;
					new_node->text_begin = current_position;
					new_node->suffix_start = j;
					current_node->children[ctx.text[current_position]] = new_node;
				}else{
					const size_t offset = next_position - current_position;
					auto mid_node = std::make_shared<Node>();
					mid_node->parent = next_node->parent;
					mid_node->text_begin = next_node->text_begin;
					mid_node->text_end = next_node->text_begin + offset;
					if(mid_node->text_begin == mid_node->text_end && mid_node->text_end != 0){
						throw std::runtime_error("mid_node text_begin == text_end == " + std::to_string(mid_node->text_begin));
					}
					if(auto parent = next_node->parent.lock()){
						parent->children[ctx.text[next_node->text_begin]] = mid_node;
					}else{
						throw std::runtime_error("Node without a parent");
					}

					auto match_leaf = std::make_shared<Node>();
					match_leaf->parent = mid_node;
					match_leaf->text_begin = next_position;
					match_leaf->suffix_start = j;

					next_node->parent = mid_node;
					next_node->text_begin += offset;
					if(next_node->text_begin == next_node->text_end && next_node->text_end != 0){
						throw std::runtime_error("next_node text_begin == text_end == " + std::to_string(next_node->text_begin));
					}

					mid_node->children[ctx.text[next_node->text_begin]] = next_node;
					mid_node->children[ctx.text[match_leaf->text_begin]] = match_leaf;

					if(link_wanted){
						link_wanted->suffix_link = mid_node;
						link_wanted = nullptr;
					}
					if(mid_node->parent.lock() == ctx.root && mid_node->get_text_end(ctx.end_of_text) == mid_node->text_begin+1){
						mid_node->suffix_link = ctx.root;
					}else{
						link_wanted = mid_node;
						link_wanted_position = next_position;
					}
				}
			}
		}
	}
	relabel_text_end(ctx);
	// FUNCTION_ID: suffixtree_func009 - END
}

// Function 10: Relabel text end (Medium)
void relabel_text_end(SuffixTreeContext &ctx){
	// FUNCTION_ID: suffixtree_func010 - START
	std::queue<std::shared_ptr<Node>> todo;
	todo.push(ctx.root);
	while(!todo.empty()){
		auto next_node = todo.front();
		todo.pop();
		if(next_node->text_end == 0){
			next_node->text_end = ctx.end_of_text;
		}
		for(const auto &entry : next_node->children){
			todo.push(entry.second);
		}
	}
	// FUNCTION_ID: suffixtree_func010 - END
}

// Function 11: Traverse node (Medium)
std::shared_ptr<Node> traverse_node(const SuffixTreeContext &ctx, std::shared_ptr<Node> node, const std::string &find_text, size_t &position, size_t end){
	// FUNCTION_ID: suffixtree_func011 - START
	if(node->children.empty() && node != ctx.root){
		return nullptr;
	}

	const auto find = node->children.find(find_text[position]);
	if(find == node->children.end()){
		return node;
	}
	const auto next_step = find->second;

	size_t offset = 0;
	while(next_step->text_begin+offset < next_step->get_text_end(ctx.end_of_text) && position+offset < end && ctx.text[next_step->text_begin+offset] == find_text[position+offset]){
		offset++;
	}

	position += offset;
	return next_step;
	// FUNCTION_ID: suffixtree_func011 - END
}

// Function 12: Traverse tree (Medium)
std::shared_ptr<const Node> traverse_tree(const SuffixTreeContext &ctx, const std::string &find_text, size_t &position, size_t end){
	// FUNCTION_ID: suffixtree_func012 - START
	std::shared_ptr<Node> current_node;
	size_t current_position;
	std::shared_ptr<Node> next_node = ctx.root;
	do{
		current_node = next_node;
		current_position = position;
		next_node = traverse_node(ctx, current_node, find_text, position, end);
	}while(next_node && position < end && current_node != next_node && next_node->text_end - next_node->text_begin == position - current_position);

	if(position >= end && next_node && position - current_position <= next_node->text_end - next_node->text_begin){
		return next_node;
	}else{
		return current_node;
	}
	// FUNCTION_ID: suffixtree_func012 - END
}

// Function 13: Get edge label (Easy)
std::string get_edge_label(const SuffixTreeContext &ctx, std::shared_ptr<const Node> node){
	// FUNCTION_ID: suffixtree_func013 - START
	std::string result;
	const size_t text_end = node->get_text_end(ctx.end_of_text);
	result.reserve(text_end - node->text_begin);
	for(size_t i=node->text_begin; i<text_end; i++){
		result.push_back(ctx.text[i]);
	}
	return result;
	// FUNCTION_ID: suffixtree_func013 - END
}

// Function 14: Get path label (Medium)
std::string get_path_label(const SuffixTreeContext &ctx, std::shared_ptr<const Node> node){
	// FUNCTION_ID: suffixtree_func014 - START
	std::string result;
	auto current_node = node;
	while(current_node && current_node != ctx.root){
		result = get_edge_label(ctx, current_node) + result;
		current_node = current_node->parent.lock();
	}
	return result;
	// FUNCTION_ID: suffixtree_func014 - END
}

// Function 15: Node get text end (Easy)
size_t node_get_text_end(const Node &node, size_t end_of_text){
	// FUNCTION_ID: suffixtree_func015 - START
	if(node.text_end == 0){
		return end_of_text;
	}
	return node.text_end;
	// FUNCTION_ID: suffixtree_func015 - END
}
