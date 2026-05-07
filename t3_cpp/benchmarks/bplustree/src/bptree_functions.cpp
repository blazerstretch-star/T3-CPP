#include "bptree.h"
#include <algorithm>
#include <queue>
// Standard C++ Library (pre-included for agent evaluation)
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <queue>
#include <stack>
#include <deque>
#include <list>
#include <algorithm>
#include <cmath>
#include <string>
#include <memory>
#include <utility>


namespace bptree {

Node::ptr::ptr() {}
Node::ptr::~ptr() {}

Node::Node() {
    this->isLeaf = false;
    this->ptr2next = NULL;
}

Node::~Node() {
    if (isLeaf) {
        for (size_t i = 0; i < ptr2TreeOrData.dataPtr.size(); i++) {
            if (ptr2TreeOrData.dataPtr[i] != NULL) {
                fclose(ptr2TreeOrData.dataPtr[i]);
                ptr2TreeOrData.dataPtr[i] = NULL;
            }
        }
        ptr2TreeOrData.dataPtr.~vector<FILE*>();
    } else {
        ptr2TreeOrData.ptr2Tree.~vector<Node*>();
    }
}

bool search_key(Node* root, int key) {
    // FUNCTION_ID: bptree_func001 - START
    if (root == NULL) return false;
    Node* cursor = root;
    while (cursor->isLeaf == false) {
        int idx = std::upper_bound(cursor->keys.begin(), cursor->keys.end(), key) - cursor->keys.begin();
        cursor = cursor->ptr2TreeOrData.ptr2Tree[idx];
    }
    int idx = std::lower_bound(cursor->keys.begin(), cursor->keys.end(), key) - cursor->keys.begin();
    if (idx == cursor->keys.size() || cursor->keys[idx] != key) return false;
    return true;
    // FUNCTION_ID: bptree_func001 - END
}

Node* find_leaf_node(Node* root, int key) {
    // FUNCTION_ID: bptree_func002 - START
    if (root == NULL) return NULL;
    Node* cursor = root;
    while (cursor->isLeaf == false) {
        int idx = std::upper_bound(cursor->keys.begin(), cursor->keys.end(), key) - cursor->keys.begin();
        cursor = cursor->ptr2TreeOrData.ptr2Tree[idx];
    }
    return cursor;
    // FUNCTION_ID: bptree_func002 - END
}

int find_insert_position(const std::vector<int>& keys, int key) {
    // FUNCTION_ID: bptree_func003 - START
    return std::upper_bound(keys.begin(), keys.end(), key) - keys.begin();
    // FUNCTION_ID: bptree_func003 - END
}

void insert_into_sorted_leaf(Node* leaf, int key, FILE* filePtr) {
    // FUNCTION_ID: bptree_func005 - START
    if (leaf == NULL) return;
    int i = std::upper_bound(leaf->keys.begin(), leaf->keys.end(), key) - leaf->keys.begin();
    leaf->keys.push_back(key);
    leaf->ptr2TreeOrData.dataPtr.push_back(filePtr);
    if (i != leaf->keys.size() - 1) {
        for (int j = leaf->keys.size() - 1; j > i; j--) {
            leaf->keys[j] = leaf->keys[j - 1];
            leaf->ptr2TreeOrData.dataPtr[j] = leaf->ptr2TreeOrData.dataPtr[j - 1];
        }
        leaf->keys[i] = key;
        leaf->ptr2TreeOrData.dataPtr[i] = filePtr;
    }
    // FUNCTION_ID: bptree_func005 - END
}

Node* find_first_leaf(Node* root) {
    // FUNCTION_ID: bptree_func004 - START
    if (root == NULL) return NULL;
    Node* cursor = root;
    if (cursor->isLeaf) return cursor;
    for (int i = 0; i < cursor->ptr2TreeOrData.ptr2Tree.size(); i++)
        if (cursor->ptr2TreeOrData.ptr2Tree[i] != NULL)
            return find_first_leaf(cursor->ptr2TreeOrData.ptr2Tree[i]);
    return NULL;
    // FUNCTION_ID: bptree_func004 - END
}

Node* find_parent(Node* root, Node* child) {
    // FUNCTION_ID: bptree_func006 - START
    if (root == NULL || child == NULL) return NULL;
    if (root->isLeaf) return NULL;
    if (root->ptr2TreeOrData.ptr2Tree.empty()) return NULL;
    if (root->ptr2TreeOrData.ptr2Tree[0] == NULL || root->ptr2TreeOrData.ptr2Tree[0]->isLeaf) return NULL;
    for (int i = 0; i < root->ptr2TreeOrData.ptr2Tree.size(); i++) {
        if (root->ptr2TreeOrData.ptr2Tree[i] == child) {
            return root;
        }
    }
    for (int i = 0; i < root->ptr2TreeOrData.ptr2Tree.size(); i++) {
        Node* tmpCursor = root->ptr2TreeOrData.ptr2Tree[i];
        if (tmpCursor != NULL && !tmpCursor->isLeaf) {
            Node* result = find_parent(tmpCursor, child);
            if (result != NULL) return result;
        }
    }
    return NULL;
    // FUNCTION_ID: bptree_func006 - END
}

int count_keys_in_level(Node* root, int level) {
    // FUNCTION_ID: bptree_func007 - START
    if (root == NULL) return 0;
    std::queue<Node*> q;
    q.push(root);
    int currentLevel = 0;
    int keyCount = 0;
    while (!q.empty()) {
        int sz = q.size();
        if (currentLevel == level) {
            for (int i = 0; i < sz; i++) {
                Node* u = q.front();
                q.pop();
                keyCount += u->keys.size();
            }
            return keyCount;
        }
        for (int i = 0; i < sz; i++) {
            Node* u = q.front();
            q.pop();
            if (u->isLeaf != true) {
                for (Node* v : u->ptr2TreeOrData.ptr2Tree) {
                    q.push(v);
                }
            }
        }
        currentLevel++;
    }
    return 0;
    // FUNCTION_ID: bptree_func007 - END
}

} // namespace bptree
