#include "ds_algorithms.h"
#include <cmath>

namespace Algorithms {

// ============================================================================
// EASY FUNCTIONS (10)
// ============================================================================

template <typename T>
    // FUNCTION_ID: cppprojects_func001 - START
T max(T a, T b) {
    return (a > b) ? a : b;
    // FUNCTION_ID: cppprojects_func001 - END
}

template <typename T>
    // FUNCTION_ID: cppprojects_func002 - START
T min(T a, T b) {
    return (a < b) ? a : b;
    // FUNCTION_ID: cppprojects_func002 - END
}

template <typename T>
    // FUNCTION_ID: cppprojects_func003 - START
void swap(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
    // FUNCTION_ID: cppprojects_func003 - END
}

    // FUNCTION_ID: cppprojects_func004 - START
int absolute(int x) {
    return (x < 0) ? -x : x;
    // FUNCTION_ID: cppprojects_func004 - END
}

    // FUNCTION_ID: cppprojects_func005 - START
int power(int x, int n) {
    int result = 1;
    for (int i = 0; i < n; i++) {
        result *= x;
    }
    return result;
    // FUNCTION_ID: cppprojects_func005 - END
}

    // FUNCTION_ID: cppprojects_func006 - START
int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    // FUNCTION_ID: cppprojects_func006 - END
    return result;
}
    // FUNCTION_ID: cppprojects_func007 - START

int fibonacci(int n) {
    if (n <= 1) return n;
    int a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        int temp = a + b;
        a = b;
        b = temp;
    }
    // FUNCTION_ID: cppprojects_func007 - END
    return b;
}
    // FUNCTION_ID: cppprojects_func008 - START

bool isPrime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
    // FUNCTION_ID: cppprojects_func008 - END
}

    // FUNCTION_ID: cppprojects_func009 - START
int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    // FUNCTION_ID: cppprojects_func009 - END
    return a;
}
    // FUNCTION_ID: cppprojects_func010 - START

template <typename T>
void reverseArray(T arr[], int size) {
    for (int i = 0; i < size / 2; i++) {
    // FUNCTION_ID: cppprojects_func010 - END
        swap(arr[i], arr[size - 1 - i]);
    }
}

// ============================================================================
// MEDIUM FUNCTIONS (30)
// ============================================================================
   

template <typename T>
void bstInsert(TreeNode<T>*& root, const T& item) {
     // FUNCTION_ID: cppprojects_func011 - START
    if (root == nullptr) {
        root = new TreeNode<T>(item);
    } else if (item < root->value) {
        bstInsert(root->left, item);
    } else if (item > root->value) {
    
        bstInsert(root->right, item);
    }
    // FUNCTION_ID: cppprojects_func011 - END
}

template <typename T>
TreeNode<T>* bstSearch(TreeNode<T>* root, const T& item) {
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

template <typename T>
TreeNode<T>* bstFindMin(TreeNode<T>* root) {
    // FUNCTION_ID: cppprojects_func013 - START
    if (root == nullptr) return nullptr;
    while (root->left != nullptr) {
    
        root = root->left;
    }
    return root;
    // FUNCTION_ID: cppprojects_func013 - END
}

template <typename T>
TreeNode<T>* bstFindMax(TreeNode<T>* root) {
     // FUNCTION_ID: cppprojects_func014 - START
    if (root == nullptr) return nullptr;
    
    while (root->right != nullptr) {
        root = root->right;
 
    }
    return root;
    // FUNCTION_ID: cppprojects_func014 - END
}

   
template <typename T>
int bstHeight(TreeNode<T>* root) {
       // FUNCTION_ID: cppprojects_func015 - START
    
    if (root == nullptr) return 0;
    return 1 + max(bstHeight(root->left), bstHeight(root->right));
 // FUNCTION_ID: cppprojects_func015 - END
}
    

template <typename T>
   
int bstCountNodes(TreeNode<T>* root) {
    // FUNCTION_ID: cppprojects_func016 - START
    if (root == nullptr) return 0;
    return 1 + bstCountNodes(root->left) + bstCountNodes(root->right);
     // FUNCTION_ID: cppprojects_func016 - END
}

template <typename T>
void selectionSort(T arr[], int size) {
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

template <typename T>
void insertionSort(T arr[], int size) {
    // FUNCTION_ID: cppprojects_func018 - START
    for (int i = 1; i < size; i++) {
    
        T key = arr[i];
        int j = i - 1;
   
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
    // FUNCTION_ID: cppprojects_func018 - END
}
    

template <typename T>
    
void bubbleSort(T arr[], int size) {
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

template <typename T>
void mergeArrays(T arr[], int left, int mid, int right) {
      // FUNCTION_ID: cppprojects_func020 - START
    int n1 = mid - left + 1;
    int n2 = right - mid;
    T* L = new T[n1];
    T* R = new T[n2];
  
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

template <typename T>
    // FUNCTION_ID: cppprojects_func021 - START
void shellSort(T arr[], int size) {
    for (int gap = size / 2; gap > 0; gap /= 2) {
    
        for (int i = gap; i < size; i++) {
            T temp = arr[i];
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

template <typename T>
int linearSearch(const T arr[], int size, const T& target) {
    // FUNCTION_ID: cppprojects_func023 - START
    for (int i = 0; i < size; i++) {
        if (arr[i] == target) return i;
    }
    return -1;
    // FUNCTION_ID: cppprojects_func023 - END
}

    
template <typename T>
int binarySearchIterative(const T arr[], int size, const T& target) {
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

template <typename T>
int jumpSearch(const T arr[], int size, const T& target) {
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
}

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

template <typename T>
void avlRotateLeft(TreeNode<T>*& k1) {
    // FUNCTION_ID: cppprojects_func027 - START
    TreeNode<T>* k2 = k1->right;
    k1->right = k2->left;
    k2->left = k1;
    k1 = k2;
    // FUNCTION_ID: cppprojects_func027 - END
}

template <typename T>
void avlRotateRight(TreeNode<T>*& k1) {
    // FUNCTION_ID: cppprojects_func028 - START
    TreeNode<T>* k2 = k1->left;
    k1->left = k2->right;
    k2->right = k1;
    k1 = k2;
    // FUNCTION_ID: cppprojects_func028 - END
}

template <typename T>
int avlGetBalance(TreeNode<T>* root) {
    // FUNCTION_ID: cppprojects_func029 - START
    if (root == nullptr) return 0;
    return bstHeight(root->left) - bstHeight(root->right);
    // FUNCTION_ID: cppprojects_func029 - END
}

template <typename T>
void avlUpdateHeight(TreeNode<T>* root) {
    // FUNCTION_ID: cppprojects_func030 - START
    if (root != nullptr) {
        root->bFactor = avlGetBalance(root);
        avlUpdateHeight(root->left);
        avlUpdateHeight(root->right);
    }
    // FUNCTION_ID: cppprojects_func030 - END
}

template <typename T>
bool linkedListInsertSorted(ListNode<T>*& head, const T& value) {
    // FUNCTION_ID: cppprojects_func031 - START
    ListNode<T>* newNode = new ListNode<T>(value);
    if (head == nullptr || head->data >= value) {
        newNode->next = head;
        head = newNode;
        return true;
    }
    ListNode<T>* curr = head;
    while (curr->next != nullptr && curr->next->data < value) {
        curr = curr->next;
    }
    newNode->next = curr->next;
    curr->next = newNode;
    return true;
    // FUNCTION_ID: cppprojects_func031 - END
}

template <typename T>
bool linkedListRemove(ListNode<T>*& head, const T& value) {
    // FUNCTION_ID: cppprojects_func032 - START
    if (head == nullptr) return false;
    if (head->data == value) {
        ListNode<T>* temp = head;
        head = head->next;
        delete temp;
        return true;
    }
    ListNode<T>* curr = head;
    while (curr->next != nullptr && curr->next->data != value) {
        curr = curr->next;
    }
    if (curr->next == nullptr) return false;
    ListNode<T>* temp = curr->next;
    curr->next = curr->next->next;
    delete temp;
    return true;
    // FUNCTION_ID: cppprojects_func032 - END
}

template <typename T>
void linkedListReverse(ListNode<T>*& head) {
    // FUNCTION_ID: cppprojects_func033 - START
    ListNode<T>* prev = nullptr;
    ListNode<T>* curr = head;
    while (curr != nullptr) {
        ListNode<T>* next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next;
    }
    head = prev;
}
    // FUNCTION_ID: cppprojects_func033 - END

template <typename T>
    // FUNCTION_ID: cppprojects_func034 - START
ListNode<T>* linkedListFindMiddle(ListNode<T>* head) {
    if (head == nullptr) return nullptr;
    ListNode<T>* slow = head;
    ListNode<T>* fast = head;
    while (fast->next != nullptr && fast->next->next != nullptr) {
        slow = slow->next;
        fast = fast->next->next;
    }
    return slow;
}
    // FUNCTION_ID: cppprojects_func034 - END

template <typename T>
    // FUNCTION_ID: cppprojects_func035 - START
bool linkedListDetectCycle(ListNode<T>* head) {
    if (head == nullptr) return false;
    ListNode<T>* slow = head;
    ListNode<T>* fast = head;
    while (fast != nullptr && fast->next != nullptr) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) return true;
    }
    return false;
    // FUNCTION_ID: cppprojects_func035 - END
}

    // FUNCTION_ID: cppprojects_func036 - START
template <typename T>
ListNode<T>* linkedListMergeSorted(ListNode<T>* l1, ListNode<T>* l2) {
    if (l1 == nullptr) return l2;
    if (l2 == nullptr) return l1;
    if (l1->data <= l2->data) {
        l1->next = linkedListMergeSorted(l1->next, l2);
        return l1;
    } else {
        l2->next = linkedListMergeSorted(l1, l2->next);
        return l2;
    // FUNCTION_ID: cppprojects_func036 - END
    }
}

    // FUNCTION_ID: cppprojects_func037 - START
template <typename T>
void heapifyMax(T arr[], int n, int root) {
    int largest = root;
    int left = 2 * root + 1;
    int right = 2 * root + 2;
    if (left < n && arr[left] > arr[largest]) largest = left;
    if (right < n && arr[right] > arr[largest]) largest = right;
    if (largest != root) {
        swap(arr[root], arr[largest]);
        heapifyMax(arr, n, largest);
    }
}
    // FUNCTION_ID: cppprojects_func037 - END

    // FUNCTION_ID: cppprojects_func038 - START
template <typename T>
void heapifyMin(T arr[], int n, int root) {
    int smallest = root;
    int left = 2 * root + 1;
    int right = 2 * root + 2;
    if (left < n && arr[left] < arr[smallest]) smallest = left;
    if (right < n && arr[right] < arr[smallest]) smallest = right;
    if (smallest != root) {
        swap(arr[root], arr[smallest]);
        heapifyMin(arr, n, smallest);
    }
}
    // FUNCTION_ID: cppprojects_func038 - END

    // FUNCTION_ID: cppprojects_func039 - START
template <typename T>
void buildMaxHeap(T arr[], int size) {
    for (int i = size / 2 - 1; i >= 0; i--) {
        heapifyMax(arr, size, i);
    }
}
    // FUNCTION_ID: cppprojects_func039 - END

    // FUNCTION_ID: cppprojects_func040 - START
template <typename T>
void buildMinHeap(T arr[], int size) {
    for (int i = size / 2 - 1; i >= 0; i--) {
        heapifyMin(arr, size, i);
    }
}
    // FUNCTION_ID: cppprojects_func040 - END

// ============================================================================
// HARD FUNCTIONS (20)
// ============================================================================

template <typename T>
    // FUNCTION_ID: cppprojects_func041 - START
void quickSort(T arr[], int left, int right) {
    if (left < right) {
        int pivot = quickSortPartition(arr, left, right);
        quickSort(arr, left, pivot - 1);
        quickSort(arr, pivot + 1, right);
    }
    // FUNCTION_ID: cppprojects_func041 - END
}

    // FUNCTION_ID: cppprojects_func042 - START
template <typename T>
int quickSortPartition(T arr[], int left, int right) {
    T pivot = arr[right];
    int i = left - 1;
    for (int j = left; j < right; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[right]);
    return i + 1;
}
    // FUNCTION_ID: cppprojects_func042 - END

    // FUNCTION_ID: cppprojects_func043 - START
template <typename T>
void mergeSort(T arr[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        mergeArrays(arr, left, mid, right);
    }
}
    // FUNCTION_ID: cppprojects_func043 - END

    // FUNCTION_ID: cppprojects_func044 - START
template <typename T>
void heapSort(T arr[], int size) {
    buildMaxHeap(arr, size);
    for (int i = size - 1; i > 0; i--) {
        swap(arr[0], arr[i]);
        heapifyMax(arr, i, 0);
    }
}
    // FUNCTION_ID: cppprojects_func044 - END

    // FUNCTION_ID: cppprojects_func045 - START
void radixSort(int arr[], int size) {
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
}
    // FUNCTION_ID: cppprojects_func045 - END

    // FUNCTION_ID: cppprojects_func046 - START
template <typename T>
int binarySearchRecursive(const T arr[], int left, int right, const T& target) {
    if (left > right) return -1;
    int mid = left + (right - left) / 2;
    if (arr[mid] == target) return mid;
    if (arr[mid] > target) return binarySearchRecursive(arr, left, mid - 1, target);
    return binarySearchRecursive(arr, mid + 1, right, target);
}
    // FUNCTION_ID: cppprojects_func046 - END

    // FUNCTION_ID: cppprojects_func047 - START
template <typename T>
int ternarySearch(const T arr[], int left, int right, const T& target) {
    if (left > right) return -1;
    int mid1 = left + (right - left) / 3;
    int mid2 = right - (right - left) / 3;
    if (arr[mid1] == target) return mid1;
    if (arr[mid2] == target) return mid2;
    if (target < arr[mid1]) return ternarySearch(arr, left, mid1 - 1, target);
    if (target > arr[mid2]) return ternarySearch(arr, mid2 + 1, right, target);
    return ternarySearch(arr, mid1 + 1, mid2 - 1, target);
}
    // FUNCTION_ID: cppprojects_func047 - END

    // FUNCTION_ID: cppprojects_func048 - START
template <typename T>
void avlInsert(TreeNode<T>*& root, const T& item) {
    if (root == nullptr) {
        root = new TreeNode<T>(item);
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
}
    // FUNCTION_ID: cppprojects_func048 - END

    // FUNCTION_ID: cppprojects_func049 - START
template <typename T>
void avlDelete(TreeNode<T>*& root, const T& item) {
    if (root == nullptr) return;
    if (item < root->value) {
        avlDelete(root->left, item);
    } else if (item > root->value) {
        avlDelete(root->right, item);
    } else {
        if (root->left == nullptr || root->right == nullptr) {
            TreeNode<T>* temp = root->left ? root->left : root->right;
            if (temp == nullptr) {
                temp = root;
                root = nullptr;
            } else {
                *root = *temp;
            }
            delete temp;
        } else {
            TreeNode<T>* temp = bstFindMin(root->right);
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
}
    // FUNCTION_ID: cppprojects_func049 - END

    // FUNCTION_ID: cppprojects_func050 - START
template <typename T>
TreeNode<T>* huffmanBuildTree(T values[], double freqs[], int size) {
    TreeNode<T>** nodes = new TreeNode<T>*[size];
    for (int i = 0; i < size; i++) {
        nodes[i] = new TreeNode<T>(values[i]);
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
        TreeNode<T>* newNode = new TreeNode<T>();
        newNode->left = nodes[min1];
        newNode->right = nodes[min2];
        newNode->frequency = nodes[min1]->frequency + nodes[min2]->frequency;
        int minIdx = min(min1, min2);
        int maxIdx = max(min1, min2);
        nodes[minIdx] = newNode;
        nodes[maxIdx] = nodes[size - 1];
        size--;
    }
    TreeNode<T>* root = nodes[0];
    delete[] nodes;
    return root;
}
    // FUNCTION_ID: cppprojects_func050 - END

    // FUNCTION_ID: cppprojects_func051 - START
template <typename T>
void huffmanGenerateCodes(TreeNode<T>* root, int path[], int pathLen, int codes[][100], int& codeCount) {
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
}
    // FUNCTION_ID: cppprojects_func051 - END

    // FUNCTION_ID: cppprojects_func052 - START
template <typename T>
void huffmanDecode(TreeNode<T>* root, const int encoded[], int encLen, T decoded[], int& decLen) {
    TreeNode<T>* curr = root;
    decLen = 0;
    for (int i = 0; i < encLen; i++) {
        if (encoded[i] == 0) curr = curr->left;
        else curr = curr->right;
        if (curr->left == nullptr && curr->right == nullptr) {
            decoded[decLen++] = curr->value;
            curr = root;
        }
    }
}
    // FUNCTION_ID: cppprojects_func052 - END

    // FUNCTION_ID: cppprojects_func053 - START
int hashQuadraticProbe(int key, int tableSize, int attempt) {
    return (key % tableSize + attempt * attempt) % tableSize;
}
    // FUNCTION_ID: cppprojects_func053 - END

    // FUNCTION_ID: cppprojects_func054 - START
int hashDoubleHash(int key, int tableSize, int attempt) {
    int hash1 = key % tableSize;
    int hash2 = 1 + (key % (tableSize - 1));
    return (hash1 + attempt * hash2) % tableSize;
}
    // FUNCTION_ID: cppprojects_func054 - END

    // FUNCTION_ID: cppprojects_func055 - START
template <typename T>
bool priorityQueueEnqueue(T arr[], int& size, int capacity, const T& value) {
    if (size >= capacity) return false;
    int i = size++;
    arr[i] = value;
    while (i > 0 && arr[(i - 1) / 2] < arr[i]) {
        swap(arr[i], arr[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
    return true;
}
    // FUNCTION_ID: cppprojects_func055 - END

    // FUNCTION_ID: cppprojects_func056 - START
template <typename T>
bool priorityQueueDequeue(T arr[], int& size, T& value) {
    if (size <= 0) return false;
    value = arr[0];
    arr[0] = arr[--size];
    heapifyMax(arr, size, 0);
    return true;
}
    // FUNCTION_ID: cppprojects_func056 - END

    // FUNCTION_ID: cppprojects_func057 - START
void dfsRecursive(int graph[][100], int n, int node, bool visited[]) {
    visited[node] = true;
    for (int i = 0; i < n; i++) {
        if (graph[node][i] && !visited[i]) {
            dfsRecursive(graph, n, i, visited);
        }
    }
}
    // FUNCTION_ID: cppprojects_func057 - END

    // FUNCTION_ID: cppprojects_func058 - START
void bfsIterative(int graph[][100], int n, int start, bool visited[]) {
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
}
    // FUNCTION_ID: cppprojects_func058 - END

    // FUNCTION_ID: cppprojects_func059 - START
void dijkstraShortestPath(int graph[][100], int n, int start, int dist[]) {
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
}
    // FUNCTION_ID: cppprojects_func059 - END

    // FUNCTION_ID: cppprojects_func060 - START
void topologicalSort(int graph[][100], int n, int result[]) {
    bool visited[100] = {false};
    int stack[100], top = -1;
    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            int tempStack[100], tempTop = -1;
            tempStack[++tempTop] = i;
            while (tempTop >= 0) {
                int v = tempStack[tempTop--];
                if (!visited[v]) {
                    visited[v] = true;
                    stack[++top] = v;
                    for (int j = 0; j < n; j++) {
                        if (graph[v][j] && !visited[j]) {
                            tempStack[++tempTop] = j;
                        }
                    }
                }
            }
        }
    }
    for (int i = 0; i < n; i++) {
        result[i] = stack[n - 1 - i];
    }
}
    // FUNCTION_ID: cppprojects_func060 - END

// Explicit template instantiations for int
template int max<int>(int, int);
template int min<int>(int, int);
template void swap<int>(int&, int&);
template void reverseArray<int>(int[], int);
template void bstInsert<int>(TreeNode<int>*&, const int&);
template TreeNode<int>* bstSearch<int>(TreeNode<int>*, const int&);
template TreeNode<int>* bstFindMin<int>(TreeNode<int>*);
template TreeNode<int>* bstFindMax<int>(TreeNode<int>*);
template int bstHeight<int>(TreeNode<int>*);
template int bstCountNodes<int>(TreeNode<int>*);
template void selectionSort<int>(int[], int);
template void insertionSort<int>(int[], int);
template void bubbleSort<int>(int[], int);
template void mergeArrays<int>(int[], int, int, int);
template void shellSort<int>(int[], int);
template int linearSearch<int>(const int[], int, const int&);
template int binarySearchIterative<int>(const int[], int, const int&);
template int jumpSearch<int>(const int[], int, const int&);
template void avlRotateLeft<int>(TreeNode<int>*&);
template void avlRotateRight<int>(TreeNode<int>*&);
template int avlGetBalance<int>(TreeNode<int>*);
template void avlUpdateHeight<int>(TreeNode<int>*);
template bool linkedListInsertSorted<int>(ListNode<int>*&, const int&);
template bool linkedListRemove<int>(ListNode<int>*&, const int&);
template void linkedListReverse<int>(ListNode<int>*&);
template ListNode<int>* linkedListFindMiddle<int>(ListNode<int>*);
template bool linkedListDetectCycle<int>(ListNode<int>*);
template ListNode<int>* linkedListMergeSorted<int>(ListNode<int>*, ListNode<int>*);
template void heapifyMax<int>(int[], int, int);
template void heapifyMin<int>(int[], int, int);
template void buildMaxHeap<int>(int[], int);
template void buildMinHeap<int>(int[], int);
template void quickSort<int>(int[], int, int);
template int quickSortPartition<int>(int[], int, int);
template void mergeSort<int>(int[], int, int);
template void heapSort<int>(int[], int);
template int binarySearchRecursive<int>(const int[], int, int, const int&);
template int ternarySearch<int>(const int[], int, int, const int&);
template void avlInsert<int>(TreeNode<int>*&, const int&);
template void avlDelete<int>(TreeNode<int>*&, const int&);
template TreeNode<int>* huffmanBuildTree<int>(int[], double[], int);
template void huffmanGenerateCodes<int>(TreeNode<int>*, int[], int, int[][100], int&);
template void huffmanDecode<int>(TreeNode<int>*, const int[], int, int[], int&);
template bool priorityQueueEnqueue<int>(int[], int&, int, const int&);
template bool priorityQueueDequeue<int>(int[], int&, int&);

// Explicit template instantiations for char (Huffman)
template TreeNode<char>* huffmanBuildTree<char>(char[], double[], int);
template void huffmanGenerateCodes<char>(TreeNode<char>*, int[], int, int[][100], int&);
template void huffmanDecode<char>(TreeNode<char>*, const int[], int, char[], int&);

} // namespace Algorithms
