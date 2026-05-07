#include "ds_algorithms.h"
#include <cmath>
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
#include <string>
#include <memory>
#include <utility>

namespace Algorithms {

// ============================================================================
// EASY FUNCTIONS (10)
// ============================================================================

int max(int a, int b) {
    // FUNCTION_ID: cppprojects_func001 - START
    return (a > b) ? a : b;
    // FUNCTION_ID: cppprojects_func001 - END
}

int min(int a, int b) {
    // FUNCTION_ID: cppprojects_func002 - START
    return (a < b) ? a : b;
    // FUNCTION_ID: cppprojects_func002 - END
}

void swap(int& a, int& b) {
    // FUNCTION_ID: cppprojects_func003 - START
    int temp = a;
    a = b;
    b = temp;
    // FUNCTION_ID: cppprojects_func003 - END
}

int absolute(int x) {
    // FUNCTION_ID: cppprojects_func004 - START
    return (x < 0) ? -x : x;
    // FUNCTION_ID: cppprojects_func004 - END
}

int power(int x, int n) {
    // FUNCTION_ID: cppprojects_func005 - START
    int result = 1;
    for (int i = 0; i < n; i++) {
        result *= x;
    }
    return result;
    // FUNCTION_ID: cppprojects_func005 - END
}
    
int factorial(int n) {
    // FUNCTION_ID: cppprojects_func006 - START
    int result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
    // FUNCTION_ID: cppprojects_func006 - END
}
    

int fibonacci(int n) {
    // FUNCTION_ID: cppprojects_func007 - START
    if (n <= 1) return n;
    int a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        int temp = a + b;
        a = b;
        b = temp;
    }
    return b;
    // FUNCTION_ID: cppprojects_func007 - END
}
    
bool isPrime(int n) {
    // FUNCTION_ID: cppprojects_func008 - START
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
    // FUNCTION_ID: cppprojects_func008 - END
}

int gcd(int a, int b) {
    // FUNCTION_ID: cppprojects_func009 - START
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
    // FUNCTION_ID: cppprojects_func009 - END
}
   

void reverseArray(int arr[], int size) {
    // FUNCTION_ID: cppprojects_func010 - START
    for (int i = 0; i < size / 2; i++) {
   
        swap(arr[i], arr[size - 1 - i]);
    }
    // FUNCTION_ID: cppprojects_func010 - END
}

// ============================================================================
// MEDIUM FUNCTIONS (30)
// ============================================================================
    

void bstInsert(TreeNode*& root, const int& item) {
    // FUNCTION_ID: cppprojects_func011 - START
    if (root == nullptr) {
        root = new TreeNode(item);
    } else if (item < root->value) {
        bstInsert(root->left, item);
    } else if (item > root->value) {
        bstInsert(root->right, item);
    }
     // FUNCTION_ID: cppprojects_func011 - END
}

TreeNode* bstSearch(TreeNode* root, const int& item) {
    // FUNCTION_ID: cppprojects_func012 - START
    if (root == nullptr || root->value == item) {
        return root;
    }
    if (item < root->value) {
        return bstSearch(root->left, item);
   
    }
    return bstSearch(root->right, item);
     // FUNCTION_ID: cppprojects_func012 - END
}

TreeNode* bstFindMin(TreeNode* root) {
    // FUNCTION_ID: cppprojects_func013 - START
    if (root == nullptr) return nullptr;
    while (root->left != nullptr) {
        root = root->left;
    }
    return root;
    // FUNCTION_ID: cppprojects_func013 - END
}

TreeNode* bstFindMax(TreeNode* root) {
    // FUNCTION_ID: cppprojects_func014 - START
    if (root == nullptr) return nullptr;
    while (root->right != nullptr) {
        root = root->right;
    }
    return root;
    // FUNCTION_ID: cppprojects_func014 - END
}

int bstHeight(TreeNode* root) {
    // FUNCTION_ID: cppprojects_func015 - START
    if (root == nullptr) return 0;
    return 1 + max(bstHeight(root->left), bstHeight(root->right));
    // FUNCTION_ID: cppprojects_func015 - END
}

int bstCountNodes(TreeNode* root) {
     // FUNCTION_ID: cppprojects_func016 - START
    if (root == nullptr) return 0;
    return 1 + bstCountNodes(root->left) + bstCountNodes(root->right);
    // FUNCTION_ID: cppprojects_func016 - END
}

void selectionSort(int arr[], int size) {
    // FUNCTION_ID: cppprojects_func017 - START
    for (int i = 0; i < size - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < size; j++) {
            if (arr[j] < arr[minIdx]) minIdx = j;
        }
        swap(arr[i], arr[minIdx]);
    }
    // FUNCTION_ID: cppprojects_func017 - END
}

void insertionSort(int arr[], int size) {
    // FUNCTION_ID: cppprojects_func018 - START
    for (int i = 1; i < size; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
    // FUNCTION_ID: cppprojects_func018 - END
}

void bubbleSort(int arr[], int size) {
    // FUNCTION_ID: cppprojects_func019 - START
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                swap(arr[j], arr[j + 1]);
            }
        }
    }
    // FUNCTION_ID: cppprojects_func019 - END
}

void mergeArrays(int arr[], int left, int mid, int right) {
    // FUNCTION_ID: cppprojects_func020 - START
    int n1 = mid - left + 1;
    int n2 = right - mid;
    int* L = new int[n1];
    int* R = new int[n2];
    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
    delete[] L;
    delete[] R;
    // FUNCTION_ID: cppprojects_func020 - END
}

void shellSort(int arr[], int size) {
    // FUNCTION_ID: cppprojects_func021 - START
    for (int gap = size / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < size; i++) {
            int temp = arr[i];
            int j;
            for (j = i; j >= gap && arr[j - gap] > temp; j -= gap) {
                arr[j] = arr[j - gap];
            }
            arr[j] = temp;
        }
    }    
    // FUNCTION_ID: cppprojects_func021 - END
}
    
void countingSort(int arr[], int size, int maxVal) {
    // FUNCTION_ID: cppprojects_func022 - START
    int* count = new int[maxVal + 1]();
    for (int i = 0; i < size; i++) count[arr[i]]++;
    int idx = 0;
    for (int i = 0; i <= maxVal; i++) {
        while (count[i]-- > 0) arr[idx++] = i;
    }
    delete[] count;
     // FUNCTION_ID: cppprojects_func022 - END
}

int linearSearch(const int arr[], int size, const int& target) {
    // FUNCTION_ID: cppprojects_func023 - START
    for (int i = 0; i < size; i++) {
        if (arr[i] == target) return i;
    }
    return -1;
    // FUNCTION_ID: cppprojects_func023 - END
}

   
int binarySearchIterative(const int arr[], int size, const int& target) {
    // FUNCTION_ID: cppprojects_func024 - START
    int left = 0, right = size - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (arr[mid] == target) return mid;
        if (arr[mid] < target) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
    // FUNCTION_ID: cppprojects_func024 - END
}

int jumpSearch(const int arr[], int size, const int& target) {
    // FUNCTION_ID: cppprojects_func025 - START
    int step = sqrt(size);
    int prev = 0;
    while (arr[min(step, size) - 1] < target) {
        prev = step;
        step += sqrt(size);
        if (prev >= size) return -1;
    }
    for (int i = prev; i < min(step, size); i++) {
        if (arr[i] == target) return i;
    }
    return -1;
     // FUNCTION_ID: cppprojects_func025 - END
}

int interpolationSearch(int arr[], int size, int target) {
    // FUNCTION_ID: cppprojects_func026 - START
    int left = 0, right = size - 1;
    while (left <= right && target >= arr[left] && target <= arr[right]) {
        if (left == right) {
            if (arr[left] == target) return left;
            return -1;
        }
        int pos = left + ((target - arr[left]) * (right - left)) / (arr[right] - arr[left]);
        if (arr[pos] == target) return pos;
        if (arr[pos] < target) left = pos + 1;
        else right = pos - 1;
    }
    return -1;
    // FUNCTION_ID: cppprojects_func026 - END
}

void avlRotateLeft(TreeNode*& k1) {
    // FUNCTION_ID: cppprojects_func027 - START
    TreeNode* k2 = k1->right;
    k1->right = k2->left;
    k2->left = k1;
    k1 = k2;
    // FUNCTION_ID: cppprojects_func027 - END
}

    
void avlRotateRight(TreeNode*& k1) {
    // FUNCTION_ID: cppprojects_func028 - START
    TreeNode* k2 = k1->left;
    k1->left = k2->right;
    k2->right = k1;
    k1 = k2;
    // FUNCTION_ID: cppprojects_func028 - END
}

   
int avlGetBalance(TreeNode* root) {
    // FUNCTION_ID: cppprojects_func029 - START
    if (root == nullptr) return 0;
    return bstHeight(root->left) - bstHeight(root->right);
    // FUNCTION_ID: cppprojects_func029 - END
}

void avlUpdateHeight(TreeNode* root) {
     // FUNCTION_ID: cppprojects_func030 - START
    if (root != nullptr) {
        root->bFactor = avlGetBalance(root);
        avlUpdateHeight(root->left);
        avlUpdateHeight(root->right);
    }
    // FUNCTION_ID: cppprojects_func030 - END
}

bool linkedListInsertSorted(ListNode*& head, const int& value) {
     // FUNCTION_ID: cppprojects_func031 - START
    ListNode* newNode = new ListNode(value);
    if (head == nullptr || head->data >= value) {
        newNode->next = head;
        head = newNode;
        return true;
    }
    ListNode* curr = head;
    while (curr->next != nullptr && curr->next->data < value) {
        curr = curr->next;
    }
    newNode->next = curr->next;
    curr->next = newNode;
    return true;
    // FUNCTION_ID: cppprojects_func031 - END
}

bool linkedListRemove(ListNode*& head, const int& value) {
     // FUNCTION_ID: cppprojects_func032 - START
    if (head == nullptr) return false;
    if (head->data == value) {
        ListNode* temp = head;
        head = head->next;
        delete temp;
        return true;
    }
    ListNode* curr = head;
    while (curr->next != nullptr && curr->next->data != value) {
        curr = curr->next;
    }
    if (curr->next == nullptr) return false;
    ListNode* temp = curr->next;
    curr->next = curr->next->next;
    delete temp;
    return true;
     // FUNCTION_ID: cppprojects_func032 - END
}

void linkedListReverse(ListNode*& head) {
    // FUNCTION_ID: cppprojects_func033 - START
    ListNode* prev = nullptr;
    ListNode* curr = head;
    while (curr != nullptr) {
        ListNode* next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next;
    }
    head = prev;
    // FUNCTION_ID: cppprojects_func033 - END
}

ListNode* linkedListFindMiddle(ListNode* head) {
    // FUNCTION_ID: cppprojects_func034 - START
    if (head == nullptr) return nullptr;
    ListNode* slow = head;
    ListNode* fast = head;
    while (fast->next != nullptr && fast->next->next != nullptr) {
        slow = slow->next;
        fast = fast->next->next;
    }
    return slow;
    // FUNCTION_ID: cppprojects_func034 - END
}

bool linkedListDetectCycle(ListNode* head) {
    // FUNCTION_ID: cppprojects_func035 - START
    if (head == nullptr) return false;
    ListNode* slow = head;
    ListNode* fast = head;
    while (fast != nullptr && fast->next != nullptr) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) return true;
    }
    return false;
    // FUNCTION_ID: cppprojects_func035 - END
}

ListNode* linkedListMergeSorted(ListNode* l1, ListNode* l2) {
    // FUNCTION_ID: cppprojects_func036 - START
    if (l1 == nullptr) return l2;
    if (l2 == nullptr) return l1;
    if (l1->data <= l2->data) {
        l1->next = linkedListMergeSorted(l1->next, l2);
        return l1;
    } else {
        l2->next = linkedListMergeSorted(l1, l2->next);
        return l2;
    }
    // FUNCTION_ID: cppprojects_func036 - END
}

void heapifyMax(int arr[], int n, int root) {
    // FUNCTION_ID: cppprojects_func037 - START
    int largest = root;
    int left = 2 * root + 1;
    int right = 2 * root + 2;
    if (left < n && arr[left] > arr[largest]) largest = left;
    if (right < n && arr[right] > arr[largest]) largest = right;
    if (largest != root) {
        swap(arr[root], arr[largest]);
        heapifyMax(arr, n, largest);
    }
    // FUNCTION_ID: cppprojects_func037 - END
}

void heapifyMin(int arr[], int n, int root) {
    // FUNCTION_ID: cppprojects_func038 - START
    int smallest = root;
    int left = 2 * root + 1;
    int right = 2 * root + 2;
    if (left < n && arr[left] < arr[smallest]) smallest = left;
    if (right < n && arr[right] < arr[smallest]) smallest = right;
    if (smallest != root) {
        swap(arr[root], arr[smallest]);
        heapifyMin(arr, n, smallest);
    }
    // FUNCTION_ID: cppprojects_func038 - END
}

void buildMaxHeap(int arr[], int size) {
    // FUNCTION_ID: cppprojects_func039 - START
    for (int i = size / 2 - 1; i >= 0; i--) {
        heapifyMax(arr, size, i);
    }
    // FUNCTION_ID: cppprojects_func039 - END
}

void buildMinHeap(int arr[], int size) {
    // FUNCTION_ID: cppprojects_func040 - START
    for (int i = size / 2 - 1; i >= 0; i--) {
        heapifyMin(arr, size, i);
    // FUNCTION_ID: cppprojects_func040 - END
    }
}

// ============================================================================
// HARD FUNCTIONS (20)
// ============================================================================

void quickSort(int arr[], int left, int right) {
    // FUNCTION_ID: cppprojects_func041 - START
    if (left < right) {
        int pivot = quickSortPartition(arr, left, right);
        quickSort(arr, left, pivot - 1);
        quickSort(arr, pivot + 1, right);
    }
    // FUNCTION_ID: cppprojects_func041 - END
}

int quickSortPartition(int arr[], int left, int right) {
    // FUNCTION_ID: cppprojects_func042 - START
    int pivot = arr[right];
    int i = left - 1;
    for (int j = left; j < right; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[right]);
    return i + 1;
    // FUNCTION_ID: cppprojects_func042 - END
}

void mergeSort(int arr[], int left, int right) {
    // FUNCTION_ID: cppprojects_func043 - START
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        mergeArrays(arr, left, mid, right);
    }
    // FUNCTION_ID: cppprojects_func043 - END
}

void heapSort(int arr[], int size) {
    // FUNCTION_ID: cppprojects_func044 - START
    buildMaxHeap(arr, size);
    for (int i = size - 1; i > 0; i--) {
        swap(arr[0], arr[i]);
        heapifyMax(arr, i, 0);
    }
    // FUNCTION_ID: cppprojects_func044 - END
}

void radixSort(int arr[], int size) {
    // FUNCTION_ID: cppprojects_func045 - START
    int maxVal = arr[0];
    for (int i = 1; i < size; i++) {
        if (arr[i] > maxVal) maxVal = arr[i];
    }
    for (int exp = 1; maxVal / exp > 0; exp *= 10) {
        int* output = new int[size];
        int count[10] = {0};
        for (int i = 0; i < size; i++) count[(arr[i] / exp) % 10]++;
        for (int i = 1; i < 10; i++) count[i] += count[i - 1];
        for (int i = size - 1; i >= 0; i--) {
            output[count[(arr[i] / exp) % 10] - 1] = arr[i];
            count[(arr[i] / exp) % 10]--;
        }
        for (int i = 0; i < size; i++) arr[i] = output[i];
        delete[] output;
    }
    // FUNCTION_ID: cppprojects_func045 - END
}

int binarySearchRecursive(const int arr[], int left, int right, const int& target) {
    // FUNCTION_ID: cppprojects_func046 - START
    if (left > right) return -1;
    int mid = left + (right - left) / 2;
    if (arr[mid] == target) return mid;
    if (arr[mid] > target) return binarySearchRecursive(arr, left, mid - 1, target);
    return binarySearchRecursive(arr, mid + 1, right, target);
    // FUNCTION_ID: cppprojects_func046 - END
}

int ternarySearch(const int arr[], int left, int right, const int& target) {
    // FUNCTION_ID: cppprojects_func047 - START
    if (left > right) return -1;
    int mid1 = left + (right - left) / 3;
    int mid2 = right - (right - left) / 3;
    if (arr[mid1] == target) return mid1;
    if (arr[mid2] == target) return mid2;
    if (target < arr[mid1]) return ternarySearch(arr, left, mid1 - 1, target);
    if (target > arr[mid2]) return ternarySearch(arr, mid2 + 1, right, target);
    return ternarySearch(arr, mid1 + 1, mid2 - 1, target);
    // FUNCTION_ID: cppprojects_func047 - END
}

void avlInsert(TreeNode*& root, const int& item) {
    // FUNCTION_ID: cppprojects_func048 - START
    if (root == nullptr) {
        root = new TreeNode(item);
        return;
    }
    if (item < root->value) avlInsert(root->left, item);
    else if (item > root->value) avlInsert(root->right, item);
    else return;
    
    root->bFactor = avlGetBalance(root);
    if (root->bFactor > 1) {
        if (item < root->left->value) {
            avlRotateRight(root);
        } else {
            avlRotateLeft(root->left);
            avlRotateRight(root);
        }
    } else if (root->bFactor < -1) {
        if (item > root->right->value) {
            avlRotateLeft(root);
        } else {
            avlRotateRight(root->right);
            avlRotateLeft(root);
        }
    }
    // FUNCTION_ID: cppprojects_func048 - END
}

void avlDelete(TreeNode*& root, const int& item) {
    // FUNCTION_ID: cppprojects_func049 - START
    if (root == nullptr) return;
    if (item < root->value) {
        avlDelete(root->left, item);
    } else if (item > root->value) {
        avlDelete(root->right, item);
    } else {
        if (root->left == nullptr || root->right == nullptr) {
            TreeNode* temp = root->left ? root->left : root->right;
            if (temp == nullptr) {
                temp = root;
                root = nullptr;
            } else {
                *root = *temp;
            }
            delete temp;
   
        } else {
            TreeNode* temp = bstFindMin(root->right);
   
            root->value = temp->value;
            avlDelete(root->right, temp->value);
        }
    }
    if (root == nullptr) return;
    root->bFactor = avlGetBalance(root);
    if (root->bFactor > 1) {
        if (avlGetBalance(root->left) >= 0) avlRotateRight(root);
        else { avlRotateLeft(root->left); avlRotateRight(root); }
    } else if (root->bFactor < -1) {
        if (avlGetBalance(root->right) <= 0) avlRotateLeft(root);
        else { avlRotateRight(root->right); avlRotateLeft(root); }
    }
    // FUNCTION_ID: cppprojects_func049 - END
}

TreeNode* huffmanBuildTree(int values[], double freqs[], int size) {
    // FUNCTION_ID: cppprojects_func050 - START
    TreeNode** nodes = new TreeNode*[size];
    
    for (int i = 0; i < size; i++) {
        nodes[i] = new TreeNode(values[i]);
   
        nodes[i]->frequency = freqs[i];
    }
    while (size > 1) {
        int min1 = 0, min2 = 1;
        if (nodes[min2]->frequency < nodes[min1]->frequency) { int t = min1; min1 = min2; min2 = t; }
        for (int i = 2; i < size; i++) {
            if (nodes[i]->frequency < nodes[min1]->frequency) {
                min2 = min1; min1 = i;
            } else if (nodes[i]->frequency < nodes[min2]->frequency) {
                min2 = i;
            }
        }
        TreeNode* newNode = new TreeNode();
        newNode->left = nodes[min1];
        newNode->right = nodes[min2];
        newNode->frequency = nodes[min1]->frequency + nodes[min2]->frequency;
        int minIdx = min(min1, min2);
        int maxIdx = max(min1, min2);
        nodes[minIdx] = newNode;
        nodes[maxIdx] = nodes[size - 1];
        size--;
    }
    TreeNode* root = nodes[0];
    delete[] nodes;
    return root;
        // FUNCTION_ID: cppprojects_func050 - END
}

void huffmanGenerateCodes(TreeNode* root, int path[], int pathLen, int codes[][100], int& codeCount) {
        // FUNCTION_ID: cppprojects_func051 - START
    if (root == nullptr) return;
    if (root->left == nullptr && root->right == nullptr) {
        for (int i = 0; i < pathLen; i++) {
            codes[codeCount][i] = path[i];
        }
        codes[codeCount][pathLen] = -1;
        codeCount++;
        return;
    }
    if (root->left) {
        path[pathLen] = 0;
        huffmanGenerateCodes(root->left, path, pathLen + 1, codes, codeCount);
    }
    if (root->right) {
        path[pathLen] = 1;
        huffmanGenerateCodes(root->right, path, pathLen + 1, codes, codeCount);
    }
        // FUNCTION_ID: cppprojects_func051 - END
}

void huffmanDecode(TreeNode* root, const int encoded[], int encLen, int decoded[], int& decLen) {
    // FUNCTION_ID: cppprojects_func052 - START
    TreeNode* curr = root;
    decLen = 0;
    for (int i = 0; i < encLen; i++) {
        if (encoded[i] == 0) curr = curr->left;
        else curr = curr->right;
        if (curr->left == nullptr && curr->right == nullptr) {
            decoded[decLen++] = curr->value;
            curr = root;
        }
    }
    // FUNCTION_ID: cppprojects_func052 - END
}

int hashQuadraticProbe(int key, int tableSize, int attempt) {
    // FUNCTION_ID: cppprojects_func053 - START
    return (key % tableSize + attempt * attempt) % tableSize;
    // FUNCTION_ID: cppprojects_func053 - END
}

int hashDoubleHash(int key, int tableSize, int attempt) {
    // FUNCTION_ID: cppprojects_func054 - START
    int hash1 = key % tableSize;
    int hash2 = 1 + (key % (tableSize - 1));
    return (hash1 + attempt * hash2) % tableSize;
    // FUNCTION_ID: cppprojects_func054 - END
}

bool priorityQueueEnqueue(int arr[], int& size, int capacity, const int& value) {
    // FUNCTION_ID: cppprojects_func055 - START
    if (size >= capacity) return false;
    int i = size++;
    arr[i] = value;
    while (i > 0 && arr[(i - 1) / 2] < arr[i]) {
        swap(arr[i], arr[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
    return true;
    // FUNCTION_ID: cppprojects_func055 - END
}

bool priorityQueueDequeue(int arr[], int& size, int& value) {
    // FUNCTION_ID: cppprojects_func056 - START
    if (size <= 0) return false;
    value = arr[0];
    arr[0] = arr[--size];
    heapifyMax(arr, size, 0);
    return true;
    // FUNCTION_ID: cppprojects_func056 - END
}

void dfsRecursive(int graph[][100], int n, int node, bool visited[]) {
    // FUNCTION_ID: cppprojects_func057 - START
    visited[node] = true;
    for (int i = 0; i < n; i++) {
        if (graph[node][i] && !visited[i]) {
            dfsRecursive(graph, n, i, visited);
        }
    }
    // FUNCTION_ID: cppprojects_func057 - END
}

void bfsIterative(int graph[][100], int n, int start, bool visited[]) {
    // FUNCTION_ID: cppprojects_func058 - START
    int queue[100], front = 0, rear = 0;
    visited[start] = true;
    queue[rear++] = start;
    while (front < rear) {
        int node = queue[front++];
        for (int i = 0; i < n; i++) {
            if (graph[node][i] && !visited[i]) {
                visited[i] = true;
                queue[rear++] = i;
            }
        }
    }
    // FUNCTION_ID: cppprojects_func058 - END
}

void dijkstraShortestPath(int graph[][100], int n, int start, int dist[]) {
    // FUNCTION_ID: cppprojects_func059 - START
    bool visited[100] = {false};
    for (int i = 0; i < n; i++) dist[i] = std::numeric_limits<int>::max();
    dist[start] = 0;
    for (int count = 0; count < n - 1; count++) {
        int minDist = std::numeric_limits<int>::max(), u = -1;
        for (int v = 0; v < n; v++) {
            if (!visited[v] && dist[v] < minDist) {
                minDist = dist[v];
                u = v;
            }
        }
        if (u == -1) break;
        visited[u] = true;
        for (int v = 0; v < n; v++) {
            if (!visited[v] && graph[u][v] && dist[u] != std::numeric_limits<int>::max() && dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
            }
        }
    }
    // FUNCTION_ID: cppprojects_func059 - END
}

void topologicalSort(int graph[][100], int n, int result[]) {
    // FUNCTION_ID: cppprojects_func060 - START
    int inDegree[100] = {0};
    
    // Calculate in-degrees
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (graph[i][j]) {
                inDegree[j]++;
            }
        }
    }
    
    // Queue for nodes with in-degree 0
    int queue[100], front = 0, rear = 0;
    for (int i = 0; i < n; i++) {
        if (inDegree[i] == 0) {
            queue[rear++] = i;
        }
    }
    
    // Process nodes in topological order
    int idx = 0;
    while (front < rear) {
        int u = queue[front++];
        result[idx++] = u;
        
        // Reduce in-degree for neighbors
        for (int v = 0; v < n; v++) {
            if (graph[u][v]) {
                inDegree[v]--;
                if (inDegree[v] == 0) {
                    queue[rear++] = v;
                }
            }
        }
    }
    // FUNCTION_ID: cppprojects_func060 - END
}

} // namespace Algorithms
