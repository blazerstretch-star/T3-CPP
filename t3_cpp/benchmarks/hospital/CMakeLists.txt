cmake_minimum_required(VERSION 3.10)
project(HospitalBenchmark)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(src)

# Source files
set(SOURCES
    src/hospital_functions.cpp
)

# Create individual test executables for each function
set(TEST_FUNCTIONS
    heapifyUp
    heapifyDown
    findMaxId
    validateAge
    insertDoctor
    searchBySpecialization
    addEmergencyPatient
    getNextEmergencyPatient
    dijkstraShortestPath
    isDoctorAvailable
    updateDoctorAvailability
    getDoctorsBySpecialization
    calculateDistance
    isEmergencyQueueEmpty
    processPatientWorkflow
    dispatchAmbulance
    manageEmergencyOPD
    bookAppointment
    getOptimalRoute
    managePatientLifecycle
)

# Create test executables
foreach(FUNCTION ${TEST_FUNCTIONS})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_${FUNCTION}.cpp")
        add_executable(test_${FUNCTION} tests/test_${FUNCTION}.cpp ${SOURCES})
        add_test(NAME test_${FUNCTION} COMMAND test_${FUNCTION})
    endif()
endforeach()

# Strict test executables
set(STRICT_TEST_FUNCTIONS
    heapifyUp
    heapifyDown
    insertDoctor
    dijkstraShortestPath
    emergencyQueue
)

foreach(FUNCTION ${STRICT_TEST_FUNCTIONS})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_${FUNCTION}_strict.cpp")
        add_executable(test_${FUNCTION}_strict tests/test_${FUNCTION}_strict.cpp ${SOURCES})
        add_test(NAME test_${FUNCTION}_strict COMMAND test_${FUNCTION}_strict)
    endif()
endforeach()

# Enable testing
enable_testing()