cmake_minimum_required(VERSION 3.10)
project(DSAlgorithms)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library
add_library(ds_algorithms src/cpp-projects_functions.cpp)
target_include_directories(ds_algorithms PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Enable testing
enable_testing()

# Test executables
set(TEST_NAMES
    absolute avlDelete avlGetBalance avlInsert avlRotateLeft avlRotateRight avlUpdateHeight
    bfsIterative binarySearchIterative binarySearchRecursive
    bstCountNodes bstFindMax bstFindMin bstHeight bstInsert bstSearch
    bubbleSort buildMaxHeap buildMinHeap countingSort
    dfsRecursive dijkstraShortestPath
    factorial fibonacci gcd
    hashDoubleHash hashQuadraticProbe heapifyMax heapifyMin heapSort
    huffmanBuildTree huffmanDecode huffmanGenerateCodes
    insertionSort interpolationSearch isPrime jumpSearch
    linearSearch linkedListDetectCycle linkedListFindMiddle linkedListInsertSorted linkedListMergeSorted linkedListRemove linkedListReverse
    max mergeArrays mergeSort min
    power priorityQueueDequeue priorityQueueEnqueue
    quickSort quickSortPartition radixSort reverseArray
    selectionSort shellSort swap ternarySearch topologicalSort
)

foreach(TEST_NAME ${TEST_NAMES})
    add_executable(test_${TEST_NAME} tests/test_${TEST_NAME}.cpp)
    target_link_libraries(test_${TEST_NAME} ds_algorithms)
    add_test(NAME ${TEST_NAME} COMMAND test_${TEST_NAME})
endforeach()

# Strict test executables
set(STRICT_TEST_NAMES
    absolute avlDelete avlGetBalance avlInsert avlRotateLeft avlRotateRight avlUpdateHeight
    bfsIterative binarySearchIterative binarySearchRecursive
    bstCountNodes bstFindMax bstFindMin bstHeight bstInsert bstSearch
    bubbleSort buildMaxHeap buildMinHeap countingSort
    dfsRecursive dijkstraShortestPath
    factorial fibonacci gcd
    hashDoubleHash hashQuadraticProbe heapifyMax heapifyMin heapSort
    huffmanBuildTree huffmanDecode huffmanGenerateCodes
    insertionSort interpolationSearch isPrime jumpSearch
    linearSearch linkedListDetectCycle linkedListFindMiddle linkedListInsertSorted linkedListMergeSorted linkedListRemove linkedListReverse
    max mergeArrays mergeSort min
    power priorityQueueDequeue priorityQueueEnqueue
    quickSort quickSortPartition radixSort reverseArray
    selectionSort shellSort swap ternarySearch topologicalSort
)

foreach(TEST_NAME ${STRICT_TEST_NAMES})
    add_executable(test_${TEST_NAME}_strict tests_strict/test_${TEST_NAME}_strict.cpp)
    target_link_libraries(test_${TEST_NAME}_strict ds_algorithms)
    add_test(NAME ${TEST_NAME}_strict COMMAND test_${TEST_NAME}_strict)
endforeach()
