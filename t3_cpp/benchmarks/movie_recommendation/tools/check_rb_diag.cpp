#include "../src/movie_functions.h"
#include <iostream>
#include <vector>
#include <algorithm>
using namespace MovieRecommendation;

bool verify_root_is_black(RBTNode<int>* root) {
    return root == nullptr || root->colour == Colour::Black;
}

bool verify_no_red_red_violation(RBTNode<int>* node) {
    if (!node) return true;
    if (node->colour == Colour::Red) {
        if ((node->leftChild && node->leftChild->colour == Colour::Red) ||
            (node->rightChild && node->rightChild->colour == Colour::Red)) {
            return false;
        }
    }
    return verify_no_red_red_violation(node->leftChild) && verify_no_red_red_violation(node->rightChild);
}

int compute_black_height(RBTNode<int>* node, bool& valid) {
    if (!node) return 0;
    int leftHeight = compute_black_height(node->leftChild, valid);
    int rightHeight = compute_black_height(node->rightChild, valid);
    if (leftHeight != rightHeight) {
        valid = false;
        return -1;
    }
    return leftHeight + (node->colour == Colour::Black ? 1 : 0);
}

bool verify_black_height_property(RBTNode<int>* root) {
    bool valid = true;
    compute_black_height(root, valid);
    return valid;
}

bool verify_parent_consistency(RBTNode<int>* node, RBTNode<int>* expectedParent) {
    if (!node) return true;
    if (node->parent != expectedParent) return false;
    return verify_parent_consistency(node->leftChild, node) && verify_parent_consistency(node->rightChild, node);
}

void collect_data(RBTNode<int>* node, std::vector<int>& data) {
    if (!node) return;
    data.push_back(node->data);
    collect_data(node->leftChild, data);
    collect_data(node->rightChild, data);
}

void print_node(RBTNode<int>* node) {
    if (!node) return;
    std::cout << "node=" << node->data << " col=" << (node->colour==Colour::Red?"R":"B")
              << " parent=" << (node->parent?std::to_string(node->parent->data):std::string("null"))
              << " left=" << (node->leftChild?std::to_string(node->leftChild->data):std::string("null"))
              << " right=" << (node->rightChild?std::to_string(node->rightChild->data):std::string("null"))
              << std::endl;
    print_node(node->leftChild);
    print_node(node->rightChild);
}

int main(){
    RBTNode<int>* root = new RBTNode<int>(50, Colour::Black);
    RBTNode<int>* left = new RBTNode<int>(30, Colour::Red);
    RBTNode<int>* right = new RBTNode<int>(70, Colour::Red);
    RBTNode<int>* leftLeft = new RBTNode<int>(20, Colour::Red);
    RBTNode<int>* leftRight = new RBTNode<int>(40, Colour::Black);
    RBTNode<int>* rightLeft = new RBTNode<int>(60, Colour::Black);
    RBTNode<int>* rightRight = new RBTNode<int>(80, Colour::Red);

    root->leftChild = left;
    root->rightChild = right;
    left->leftChild = leftLeft;
    left->rightChild = leftRight;
    right->leftChild = rightLeft;
    right->rightChild = rightRight;

    left->parent = root;
    right->parent = root;
    leftLeft->parent = left;
    leftRight->parent = left;
    rightLeft->parent = right;
    rightRight->parent = right;

    std::vector<int> before;
    collect_data(root, before);
    std::sort(before.begin(), before.end());
    std::cout << "BEFORE TREE:\n";
    print_node(root);

    rb_insert_fix(leftLeft, root);

    std::vector<int> after;
    collect_data(root, after);
    std::sort(after.begin(), after.end());
    std::cout << "AFTER TREE:\n";
    print_node(root);

    std::cout << "verify_root_is_black: " << verify_root_is_black(root) << std::endl;
    std::cout << "verify_no_red_red_violation: " << verify_no_red_red_violation(root) << std::endl;
    std::cout << "verify_black_height_property: " << verify_black_height_property(root) << std::endl;
    std::cout << "verify_parent_consistency: " << verify_parent_consistency(root, nullptr) << std::endl;
    std::cout << "data_preserved: " << (before==after) << std::endl;

    return 0;
}
