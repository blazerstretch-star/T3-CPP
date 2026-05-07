#ifndef DS_ALGORITHMS_H
#define DS_ALGORITHMS_H

#include <iostream>
#include <limits>

// Node structures
struct TreeNode {
    typedef int T;
    int value;
    TreeNode* left;
    TreeNode* right;
    int bFactor;
    double frequency;
    
    TreeNode() : left(nullptr), right(nullptr), bFactor(0), frequency(0.0) {}
    TreeNode(const int& val) : value(val), left(nullptr), right(nullptr), bFactor(0), frequency(0.0) {}
};

struct ListNode {
    typedef int T;
    int data;
    ListNode* next;
    
    ListNode() : next(nullptr) {}
    ListNode(const int& val) : data(val), next(nullptr) {}
    ListNode(const int& val, ListNode* n) : data(val), next(n) {}
};

namespace Algorithms {

// ============================================================================
// EASY FUNCTIONS (10) - Simple operations, 5-15 lines
// ============================================================================

// 1. max - Return maximum of two values
int max(int a, int b);

// 2. min - Return minimum of two values
int min(int a, int b);

// 3. swap - Swap two values
void swap(int& a, int& b);

// 4. abs - Absolute value
int absolute(int x);

// 5. power - Calculate x^n iteratively
int power(int x, int n);

// 6. factorial - Calculate n! iteratively
int factorial(int n);

// 7. fibonacci - Calculate nth Fibonacci number iteratively
int fibonacci(int n);

// 8. isPrime - Check if number is prime
bool isPrime(int n);

// 9. gcd - Greatest common divisor (Euclidean algorithm)
int gcd(int a, int b);

// 10. reverseArray - Reverse array in-place
void reverseArray(int arr[], int size);

// ============================================================================
// MEDIUM FUNCTIONS (30) - Moderate complexity, 15-40 lines
// ============================================================================

// BST Operations (6)
// 11. bstInsert - Binary search tree insertion
void bstInsert(TreeNode*& root, const int& item);

// 12. bstSearch - Binary search tree search
TreeNode* bstSearch(TreeNode* root, const int& item);

// 13. bstFindMin - Find minimum value in BST
TreeNode* bstFindMin(TreeNode* root);

// 14. bstFindMax - Find maximum value in BST
TreeNode* bstFindMax(TreeNode* root);

// 15. bstHeight - Calculate tree height
int bstHeight(TreeNode* root);

// 16. bstCountNodes - Count total nodes in tree
int bstCountNodes(TreeNode* root);

// Sorting Algorithms (6)
// 17. selectionSort - Selection sort O(n²)
void selectionSort(int arr[], int size);

// 18. insertionSort - Insertion sort O(n²)
void insertionSort(int arr[], int size);

// 19. bubbleSort - Bubble sort O(n²)
void bubbleSort(int arr[], int size);

// 20. mergeArrays - Merge two sorted arrays
void mergeArrays(int arr[], int left, int mid, int right);

// 21. shellSort - Shell sort O(n log n)
void shellSort(int arr[], int size);

// 22. countingSort - Counting sort O(n+k)
void countingSort(int arr[], int size, int maxVal);

// Search Algorithms (4)
// 23. linearSearch - Linear search O(n)
int linearSearch(const int arr[], int size, const int& target);

// 24. binarySearchIterative - Iterative binary search O(log n)
int binarySearchIterative(const int arr[], int size, const int& target);

// 25. jumpSearch - Jump search O(√n)
int jumpSearch(const int arr[], int size, const int& target);

// 26. interpolationSearch - Interpolation search O(log log n)
int interpolationSearch(int arr[], int size, int target);

// AVL Tree Operations (4)
// 27. avlRotateLeft - AVL left rotation
void avlRotateLeft(TreeNode*& k1);

// 28. avlRotateRight - AVL right rotation
void avlRotateRight(TreeNode*& k1);

// 29. avlGetBalance - Calculate balance factor
int avlGetBalance(TreeNode* root);

// 30. avlUpdateHeight - Update height after rotation
void avlUpdateHeight(TreeNode* root);

// Linked List Operations (6)
// 31. linkedListInsertSorted - Insert in sorted order
bool linkedListInsertSorted(ListNode*& head, const int& value);

// 32. linkedListRemove - Remove node by value
bool linkedListRemove(ListNode*& head, const int& value);

// 33. linkedListReverse - Reverse linked list
void linkedListReverse(ListNode*& head);

// 34. linkedListFindMiddle - Find middle node
ListNode* linkedListFindMiddle(ListNode* head);

// 35. linkedListDetectCycle - Detect cycle (Floyd's algorithm)
bool linkedListDetectCycle(ListNode* head);

// 36. linkedListMergeSorted - Merge two sorted lists
ListNode* linkedListMergeSorted(ListNode* l1, ListNode* l2);

// Heap Operations (4)
// 37. heapifyMax - Maintain max heap property
void heapifyMax(int arr[], int n, int root);

// 38. heapifyMin - Maintain min heap property
void heapifyMin(int arr[], int n, int root);

// 39. buildMaxHeap - Build max heap from array
void buildMaxHeap(int arr[], int size);

// 40. buildMinHeap - Build min heap from array
void buildMinHeap(int arr[], int size);

// ============================================================================
// HARD FUNCTIONS (20) - Complex algorithms, 40+ lines
// ============================================================================

// Advanced Sorting (5)
// 41. quickSort - Quick sort O(n log n) average
void quickSort(int arr[], int left, int right);

// 42. quickSortPartition - Partition for quick sort
int quickSortPartition(int arr[], int left, int right);

// 43. mergeSort - Merge sort O(n log n)
void mergeSort(int arr[], int left, int right);

// 44. heapSort - Heap sort O(n log n)
void heapSort(int arr[], int size);

// 45. radixSort - Radix sort O(d*n)
void radixSort(int arr[], int size);

// Advanced Search (2)
// 46. binarySearchRecursive - Recursive binary search
int binarySearchRecursive(const int arr[], int left, int right, const int& target);

// 47. ternarySearch - Ternary search O(log₃ n)
int ternarySearch(const int arr[], int left, int right, const int& target);

// AVL Tree (2)
// 48. avlInsert - AVL insertion with balancing
void avlInsert(TreeNode*& root, const int& item);

// 49. avlDelete - AVL deletion with rebalancing
void avlDelete(TreeNode*& root, const int& item);

// Huffman Coding (3)
// 50. huffmanBuildTree - Build Huffman tree from frequencies
TreeNode* huffmanBuildTree(int values[], double freqs[], int size);

// 51. huffmanGenerateCodes - Generate Huffman codes
void huffmanGenerateCodes(TreeNode* root, int path[], int pathLen, int codes[][100], int& codeCount);

// 52. huffmanDecode - Decode Huffman encoded string
void huffmanDecode(TreeNode* root, const int encoded[], int encLen, int decoded[], int& decLen);

// Hash Table (2)
// 53. hashQuadraticProbe - Quadratic probing for hash table
int hashQuadraticProbe(int key, int tableSize, int attempt);

// 54. hashDoubleHash - Double hashing
int hashDoubleHash(int key, int tableSize, int attempt);

// Priority Queue (2)
// 55. priorityQueueEnqueue - Insert into max heap
bool priorityQueueEnqueue(int arr[], int& size, int capacity, const int& value);

// 56. priorityQueueDequeue - Extract max from heap
bool priorityQueueDequeue(int arr[], int& size, int& value);

// Graph Algorithms (4)
// 57. dfsRecursive - Depth-first search recursive
void dfsRecursive(int graph[][100], int n, int node, bool visited[]);

// 58. bfsIterative - Breadth-first search iterative
void bfsIterative(int graph[][100], int n, int start, bool visited[]);

// 59. dijkstraShortestPath - Dijkstra's algorithm
void dijkstraShortestPath(int graph[][100], int n, int start, int dist[]);

// 60. topologicalSort - Topological sort using DFS
void topologicalSort(int graph[][100], int n, int result[]);

} // namespace Algorithms

#endif
