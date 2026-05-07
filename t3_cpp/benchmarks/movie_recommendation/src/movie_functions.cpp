#include "movie_functions.h"
#include <algorithm>
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


namespace MovieRecommendation {

template <class T>
void rb_left_rotate(RBTNode<T>* x, RBTNode<T>*& root) {
    // FUNCTION_ID: movie_func001 - START
    if (x == NULL || x->rightChild == NULL) return;
    RBTNode<T>* y = x->rightChild;
    x->rightChild = y->leftChild;
    if (y->leftChild != NULL) y->leftChild->parent = x;
    y->parent = x->parent;
    if (x->parent == NULL) root = y;
    else if (x == x->parent->leftChild) x->parent->leftChild = y;
    else x->parent->rightChild = y;
    y->leftChild = x;
    x->parent = y;
    // FUNCTION_ID: movie_func001 - END
}

template <class T>
void rb_right_rotate(RBTNode<T>* x, RBTNode<T>*& root) {
    // FUNCTION_ID: movie_func002 - START
    if (x == NULL || x->leftChild == NULL) return;
    RBTNode<T>* y = x->leftChild;
    x->leftChild = y->rightChild;
    if (y->rightChild != NULL) y->rightChild->parent = x;
    y->parent = x->parent;
    if (x->parent == NULL) root = y;
    else if (x == x->parent->leftChild) x->parent->leftChild = y;
    else x->parent->rightChild = y;
    y->rightChild = x;
    x->parent = y;
    // FUNCTION_ID: movie_func002 - END
}

template <class T>
void rb_insert_fix(RBTNode<T>* current, RBTNode<T>*& root) {
    // FUNCTION_ID: movie_func003 - START
    if (current == NULL) return;
    // Debug: basic pointers (help reproduce segfaults in CI)
    // Note: kept lightweight to avoid altering control flow
    #ifndef NDEBUG
    std::cerr << "[rb_insert_fix] enter: current=" << current << " root=" << &root << std::endl;
    if (current) std::cerr << "[rb_insert_fix] current->parent=" << current->parent << std::endl;
    #endif
    
    RBTNode<T>* uncle;
    while (current != root && current->parent != NULL && current->parent->colour == Colour::Red) {
        RBTNode<T>* parent = current->parent;
        RBTNode<T>* grand = parent ? parent->parent : NULL;
        if (grand == NULL) break;

        if (parent == grand->leftChild) {
            uncle = grand->rightChild;
            if (uncle != NULL && uncle->colour == Colour::Red) {
                parent->colour = Colour::Black;
                uncle->colour = Colour::Black;
                grand->colour = Colour::Red;
                current = grand;
            } else {
                if (current == parent->rightChild) {
                    current = parent;
                    rb_left_rotate(current, root);
                    parent = current->parent;
                    grand = parent ? parent->parent : NULL;
                }
                if (parent) parent->colour = Colour::Black;
                if (grand) grand->colour = Colour::Red;
                if (grand) rb_right_rotate(grand, root);
            }
        } else {
            uncle = grand->leftChild;
            if (uncle != NULL && uncle->colour == Colour::Red) {
                parent->colour = Colour::Black;
                uncle->colour = Colour::Black;
                grand->colour = Colour::Red;
                current = grand;
            } else {
                if (current == parent->leftChild) {
                    current = parent;
                    rb_right_rotate(current, root);
                    parent = current->parent;
                    grand = parent ? parent->parent : NULL;
                }
                if (parent) parent->colour = Colour::Black;
                if (grand) grand->colour = Colour::Red;
                if (grand) rb_left_rotate(grand, root);
            }
        }
    }
    if (root != NULL) root->colour = Colour::Black;
    // FUNCTION_ID: movie_func003 - END
}

template <class T>
Colour rb_get_colour(RBTNode<T>* node) {
    // FUNCTION_ID: movie_func004 - START
    if (node == NULL) return Colour::Black;
    else return node->colour;
    // FUNCTION_ID: movie_func004 - END
}

template <class T>
AVLNode<T>* avl_rotate_left(AVLNode<T>* x, AVLNode<T>*& root) {
    // FUNCTION_ID: movie_func005 - START
    if (x == NULL || x->rightChild == NULL) return x;
    AVLNode<T>* y = x->rightChild;
    x->rightChild = y->leftChild;
    y->leftChild = x;
    if (x == root) root = y;
    x->height = std::max(avl_height(x->leftChild), avl_height(x->rightChild)) + 1;
    y->height = std::max(avl_height(y->leftChild), avl_height(y->rightChild)) + 1;
    return y;
    // FUNCTION_ID: movie_func005 - END
}

template <class T>
AVLNode<T>* avl_rotate_right(AVLNode<T>* x, AVLNode<T>*& root) {
    // FUNCTION_ID: movie_func006 - START
    if (x == NULL || x->leftChild == NULL) return x;
    AVLNode<T>* y = x->leftChild;
    x->leftChild = y->rightChild;
    y->rightChild = x;
    if (x == root) root = y;
    x->height = std::max(avl_height(x->leftChild), avl_height(x->rightChild)) + 1;
    y->height = std::max(avl_height(y->leftChild), avl_height(y->rightChild)) + 1;
    return y;
    // FUNCTION_ID: movie_func006 - END
}

template <class T>
AVLNode<T>* avl_rebalance(AVLNode<T>* current) {
    // FUNCTION_ID: movie_func007 - START
    int balanceFactor = avl_height(current->leftChild) - avl_height(current->rightChild);
    if (balanceFactor == -2) {
        AVLNode<T>* y = current->rightChild;
        if (avl_height(y->leftChild) > avl_height(y->rightChild))
            current->rightChild = avl_rotate_right(y, current);
        current = avl_rotate_left(current, current);
    } else if (balanceFactor == 2) {
        AVLNode<T>* y = current->leftChild;
        if (avl_height(y->rightChild) > avl_height(y->leftChild))
            current->leftChild = avl_rotate_left(y, current);
        current = avl_rotate_right(current, current);
    }
    current->height = std::max(avl_height(current->leftChild), avl_height(current->rightChild)) + 1;
    return current;
    // FUNCTION_ID: movie_func007 - END
}

template <class T>
int avl_height(AVLNode<T>* node) {
    // FUNCTION_ID: movie_func008 - START
    if (node == NULL) return -1;
    else return node->height;
    // FUNCTION_ID: movie_func008 - END
}

template <class T>
void merge_sort(T* input, int first, int last) {
    // FUNCTION_ID: movie_func009 - START
    if (first < last) {
        int middle = (first + last) / 2;
        merge_sort(input, first, middle);
        merge_sort(input, middle + 1, last);
        merge_arrays(input, first, middle, middle + 1, last);
    }
    // FUNCTION_ID: movie_func009 - END
}

template <class T>
void merge_arrays(T* input, int leftFirst, int leftLast, int rightFirst, int rightLast) {
    // FUNCTION_ID: movie_func010 - START
    int tempFirst = leftFirst;
    int index = 0;
    Vector<T> tempArray(rightLast - leftFirst + 1);
    while (leftFirst <= leftLast && rightFirst <= rightLast) {
        if (input[leftFirst] > input[rightFirst])
            tempArray.arr[index++] = input[leftFirst++];
        else
            tempArray.arr[index++] = input[rightFirst++];
    }
    while (leftFirst <= leftLast) tempArray.arr[index++] = input[leftFirst++];
    while (rightFirst <= rightLast) tempArray.arr[index++] = input[rightFirst++];
    for (index = tempFirst; index <= rightLast; index++) input[index] = tempArray.arr[index - tempFirst];
    // FUNCTION_ID: movie_func010 - END
}

template <class T>
void vector_dynamic_expansion(Vector<T>& vec) {
    // FUNCTION_ID: movie_func011 - START
    if (vec.full()) {
        T* tempArray = new T[vec.capacity];
        int tempSize = vec.capacity;
        for (int i = 0; i < tempSize; i++)
            tempArray[i] = vec.arr[i];
        delete[] vec.arr;
        vec.capacity *= 1.5;
        vec.capacity += 2;
        vec.arr = new T[vec.capacity];
        for (int i = 0; i < tempSize; i++)
            vec.arr[i] = tempArray[i];
        delete[] tempArray;
    }
    // FUNCTION_ID: movie_func011 - END
}

template <class T>
void vector_dynamic_reduction(Vector<T>& vec) {
    // FUNCTION_ID: movie_func012 - START
    double percentSize = (double)vec.numElements / (double)vec.capacity;
    if (percentSize < 0.5) {
        vec.capacity /= 2;
        vec.capacity += 1;
        T* tempArray = new T[vec.capacity];
        for (int i = 0; i < vec.capacity; i++)
            tempArray[i] = vec.arr[i];
        delete[] vec.arr;
        vec.arr = new T[vec.capacity];
        for (int i = 0; i < vec.capacity; i++)
            vec.arr[i] = tempArray[i];
        delete[] tempArray;
    }
    // FUNCTION_ID: movie_func012 - END
}

template <class T>
void dll_push_front(DLLNode<T>*& start, DLLNode<T>*& last, T value, int& numElements) {
    // FUNCTION_ID: movie_func013 - START
    DLLNode<T>* newNode = new DLLNode<T>(value);
    numElements++;
    if (start == NULL) {
        start = newNode;
        last = newNode;
    } else {
        newNode->next = start;
        start->prev = newNode;
        start = newNode;
    }
    // FUNCTION_ID: movie_func013 - END
}

template <class T>
void dll_push_back(DLLNode<T>*& start, DLLNode<T>*& last, T value, int& numElements) {
    // FUNCTION_ID: movie_func014 - START
    DLLNode<T>* newNode = new DLLNode<T>(value);
    numElements++;
    if (start == NULL) {
        start = newNode;
        last = newNode;
    } else {
        last->next = newNode;
        newNode->prev = last;
        last = newNode;
    }
    // FUNCTION_ID: movie_func014 - END
}

template <class T>
void dll_pop_front(DLLNode<T>*& start, DLLNode<T>*& last, int& numElements) {
    // FUNCTION_ID: movie_func015 - START
    if (start != NULL) {
        DLLNode<T>* temp = start;
        numElements--;
        if (start == last) {
            delete temp;
            start = NULL;
            last = NULL;
        } else {
            start = start->next;
            start->prev = NULL;
            temp->next = NULL;
            delete temp;
        }
    }
    // FUNCTION_ID: movie_func015 - END
}

template <class T>
void graph_add_edge(GraphVertex<T>* source, GraphVertex<T>* destination, double weight) {
    // FUNCTION_ID: movie_func016 - START
    source->addEdge(destination, weight);
    // FUNCTION_ID: movie_func016 - END
}

// Explicit template instantiations
template void rb_left_rotate<int>(RBTNode<int>* x, RBTNode<int>*& root);
template void rb_right_rotate<int>(RBTNode<int>* x, RBTNode<int>*& root);
template void rb_insert_fix<int>(RBTNode<int>* current, RBTNode<int>*& root);
template Colour rb_get_colour<int>(RBTNode<int>* node);
template AVLNode<int>* avl_rotate_left<int>(AVLNode<int>* x, AVLNode<int>*& root);
template AVLNode<int>* avl_rotate_right<int>(AVLNode<int>* x, AVLNode<int>*& root);
template AVLNode<int>* avl_rebalance<int>(AVLNode<int>* current);
template int avl_height<int>(AVLNode<int>* node);
template void merge_sort<int>(int* input, int first, int last);
template void merge_arrays<int>(int* input, int leftFirst, int leftLast, int rightFirst, int rightLast);
template void vector_dynamic_expansion<int>(Vector<int>& vec);
template void vector_dynamic_reduction<int>(Vector<int>& vec);
template void dll_push_front<int>(DLLNode<int>*& start, DLLNode<int>*& last, int value, int& numElements);
template void dll_push_back<int>(DLLNode<int>*& start, DLLNode<int>*& last, int value, int& numElements);
template void dll_pop_front<int>(DLLNode<int>*& start, DLLNode<int>*& last, int& numElements);
template void graph_add_edge<int>(GraphVertex<int>* source, GraphVertex<int>* destination, double weight);

}