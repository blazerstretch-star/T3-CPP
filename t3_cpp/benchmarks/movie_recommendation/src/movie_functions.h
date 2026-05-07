#pragma once
#include <iostream>
#include <vector>
#include <array>

using namespace std;

namespace MovieRecommendation {

enum class Colour : bool { Red, Black };

// Forward declarations
template <class T> class GraphVertex;

template <class T> class RBTNode {
public:
    T data;
    RBTNode<T>* leftChild;
    RBTNode<T>* rightChild;
    RBTNode<T>* parent;
    Colour colour;
    RBTNode<T>(T value, Colour c = Colour::Red) : data(value), leftChild(NULL), rightChild(NULL), parent(NULL), colour(c) {}
};

template <class T> class AVLNode {
public:
    T data;
    AVLNode<T>* leftChild;
    AVLNode<T>* rightChild;
    int height;
    AVLNode(T value) : data(value), leftChild(NULL), rightChild(NULL), height(0) {}
};

template <class T> class DLLNode {
public:
    T data;
    DLLNode<T>* next;
    DLLNode<T>* prev;
    DLLNode(T value) : data(value), next(NULL), prev(NULL) {}
};

template <class T> class Vector {
public:
    int capacity;
    int numElements;
    T* arr;
    
    Vector(int Size = 10) {
        capacity = Size;
        numElements = 0;
        arr = new T[capacity];
    }
    
    bool empty() { return numElements == 0; }
    bool full() { return numElements == capacity; }
    int size() { return numElements; }
    
    void push_back(const T& key) {
        vector_dynamic_expansion(*this);
        arr[numElements++] = key;
    }
    
    void DynamicExpansion() { vector_dynamic_expansion(*this); }
    void DynamicReduction() { vector_dynamic_reduction(*this); }
};

template <class T> class GraphEdge {
public:
    GraphVertex<T>* destination;
    double weight;
    GraphEdge(GraphVertex<T>* d = NULL, const double& w = 1) : destination(d), weight(w) {}
};

template <class T> class GraphVertex {
public:
    T data;
    Vector<GraphEdge<T>> edges;
    
    GraphVertex(const T& value) : data(value) {}
    
    void addEdge(GraphVertex<T>* destination, double weight) {
        edges.push_back(GraphEdge<T>(destination, weight));
    }
};

// Function declarations
template <class T> void rb_left_rotate(RBTNode<T>* x, RBTNode<T>*& root);
template <class T> void rb_right_rotate(RBTNode<T>* x, RBTNode<T>*& root);
template <class T> void rb_insert_fix(RBTNode<T>* current, RBTNode<T>*& root);
template <class T> Colour rb_get_colour(RBTNode<T>* node);

template <class T> AVLNode<T>* avl_rotate_left(AVLNode<T>* x, AVLNode<T>*& root);
template <class T> AVLNode<T>* avl_rotate_right(AVLNode<T>* x, AVLNode<T>*& root);
template <class T> AVLNode<T>* avl_rebalance(AVLNode<T>* current);
template <class T> int avl_height(AVLNode<T>* node);

template <class T> void merge_sort(T* input, int first, int last);
template <class T> void merge_arrays(T* input, int leftFirst, int leftLast, int rightFirst, int rightLast);

template <class T> void vector_dynamic_expansion(Vector<T>& vec);
template <class T> void vector_dynamic_reduction(Vector<T>& vec);

template <class T> void dll_push_front(DLLNode<T>*& start, DLLNode<T>*& last, T value, int& numElements);
template <class T> void dll_push_back(DLLNode<T>*& start, DLLNode<T>*& last, T value, int& numElements);
template <class T> void dll_pop_front(DLLNode<T>*& start, DLLNode<T>*& last, int& numElements);

template <class T> void graph_add_edge(GraphVertex<T>* source, GraphVertex<T>* destination, double weight);

}