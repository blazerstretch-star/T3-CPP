cmake_minimum_required(VERSION 3.10)
project(cg3lib_benchmark)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CGAL REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src)

# Build libraries from source files
add_library(aabb_tree_lib src/aabb_tree_functions.cpp)
target_link_libraries(aabb_tree_lib CGAL::CGAL)

add_library(convex_hull_lib src/convex_hull.cpp)
target_link_libraries(convex_hull_lib CGAL::CGAL)

add_library(geometry_utils_lib src/geometry_utils.cpp)
target_link_libraries(geometry_utils_lib CGAL::CGAL)

add_library(mesh_processing_lib src/mesh_processing.cpp)
target_link_libraries(mesh_processing_lib CGAL::CGAL)

add_library(booleans2d_lib src/booleans2d.cpp)
target_link_libraries(booleans2d_lib CGAL::CGAL)

enable_testing()

# AABB tree tests
add_executable(test_numberIntersectedPrimitives_segment tests/test_numberIntersectedPrimitives_segment.cpp)
target_link_libraries(test_numberIntersectedPrimitives_segment aabb_tree_lib CGAL::CGAL)
add_test(NAME test_numberIntersectedPrimitives_segment COMMAND test_numberIntersectedPrimitives_segment)

add_executable(test_numberIntersectedPrimitives_bbox tests/test_numberIntersectedPrimitives_bbox.cpp)
target_link_libraries(test_numberIntersectedPrimitives_bbox aabb_tree_lib CGAL::CGAL)
add_test(NAME test_numberIntersectedPrimitives_bbox COMMAND test_numberIntersectedPrimitives_bbox)

add_executable(test_squaredDistance tests/test_squaredDistance.cpp)
target_link_libraries(test_squaredDistance aabb_tree_lib CGAL::CGAL)
add_test(NAME test_squaredDistance COMMAND test_squaredDistance)

add_executable(test_nearestPoint tests/test_nearestPoint.cpp)
target_link_libraries(test_nearestPoint aabb_tree_lib CGAL::CGAL)
add_test(NAME test_nearestPoint COMMAND test_nearestPoint)

add_executable(test_isInside tests/test_isInside.cpp)
target_link_libraries(test_isInside aabb_tree_lib CGAL::CGAL)
add_test(NAME test_isInside COMMAND test_isInside)

add_executable(test_isInsidePseudoRandom tests/test_isInsidePseudoRandom.cpp)
target_link_libraries(test_isInsidePseudoRandom aabb_tree_lib CGAL::CGAL)
add_test(NAME test_isInsidePseudoRandom COMMAND test_isInsidePseudoRandom)

add_executable(test_containedTriangles tests/test_containedTriangles.cpp)
target_link_libraries(test_containedTriangles aabb_tree_lib CGAL::CGAL)
add_test(NAME test_containedTriangles COMMAND test_containedTriangles)

# Convex hull tests
add_executable(test_convexHull2D tests/test_convexHull2D.cpp)
target_link_libraries(test_convexHull2D convex_hull_lib CGAL::CGAL)
add_test(NAME test_convexHull2D COMMAND test_convexHull2D)

add_executable(test_minimumBoundingBox2D tests/test_minimumBoundingBox2D.cpp)
target_link_libraries(test_minimumBoundingBox2D convex_hull_lib CGAL::CGAL)
add_test(NAME test_minimumBoundingBox2D COMMAND test_minimumBoundingBox2D)

add_executable(test_delaunayTriangulation2D tests/test_delaunayTriangulation2D.cpp)
target_link_libraries(test_delaunayTriangulation2D convex_hull_lib CGAL::CGAL)
add_test(NAME test_delaunayTriangulation2D COMMAND test_delaunayTriangulation2D)

add_executable(test_voronoiDiagram2D tests/test_voronoiDiagram2D.cpp)
target_link_libraries(test_voronoiDiagram2D convex_hull_lib CGAL::CGAL)
add_test(NAME test_voronoiDiagram2D COMMAND test_voronoiDiagram2D)

add_executable(test_delaunayTriangulation3D tests/test_delaunayTriangulation3D.cpp)
target_link_libraries(test_delaunayTriangulation3D convex_hull_lib CGAL::CGAL)
add_test(NAME test_delaunayTriangulation3D COMMAND test_delaunayTriangulation3D)

add_executable(test_extractTetrahedra tests/test_extractTetrahedra.cpp)
target_link_libraries(test_extractTetrahedra convex_hull_lib CGAL::CGAL)
add_test(NAME test_extractTetrahedra COMMAND test_extractTetrahedra)

add_executable(test_computeAlphaShape2D tests/test_computeAlphaShape2D.cpp)
target_link_libraries(test_computeAlphaShape2D convex_hull_lib CGAL::CGAL)
add_test(NAME test_computeAlphaShape2D COMMAND test_computeAlphaShape2D)

# Geometry utils tests
add_executable(test_pointDistance tests/test_pointDistance.cpp)
target_link_libraries(test_pointDistance geometry_utils_lib CGAL::CGAL)
add_test(NAME test_pointDistance COMMAND test_pointDistance)

add_executable(test_pointMidpoint tests/test_pointMidpoint.cpp)
target_link_libraries(test_pointMidpoint geometry_utils_lib CGAL::CGAL)
add_test(NAME test_pointMidpoint COMMAND test_pointMidpoint)

add_executable(test_vectorNormalize tests/test_vectorNormalize.cpp)
target_link_libraries(test_vectorNormalize geometry_utils_lib CGAL::CGAL)
add_test(NAME test_vectorNormalize COMMAND test_vectorNormalize)

add_executable(test_vectorLength tests/test_vectorLength.cpp)
target_link_libraries(test_vectorLength geometry_utils_lib CGAL::CGAL)
add_test(NAME test_vectorLength COMMAND test_vectorLength)

add_executable(test_boundingBox tests/test_boundingBox.cpp)
target_link_libraries(test_boundingBox geometry_utils_lib CGAL::CGAL)
add_test(NAME test_boundingBox COMMAND test_boundingBox)

# Mesh processing tests
add_executable(test_holeFilling tests/test_holeFilling.cpp)
target_link_libraries(test_holeFilling mesh_processing_lib CGAL::CGAL)
add_test(NAME test_holeFilling COMMAND test_holeFilling)

# Boolean operations tests
add_executable(test_difference2D tests/test_difference2D.cpp)
target_link_libraries(test_difference2D booleans2d_lib CGAL::CGAL)
add_test(NAME test_difference2D COMMAND test_difference2D)

add_executable(test_intersection2D tests/test_intersection2D.cpp)
target_link_libraries(test_intersection2D booleans2d_lib CGAL::CGAL)
add_test(NAME test_intersection2D COMMAND test_intersection2D)

add_executable(test_union2D tests/test_union2D.cpp)
target_link_libraries(test_union2D booleans2d_lib CGAL::CGAL)
add_test(NAME test_union2D COMMAND test_union2D)

add_executable(test_doIntersect2D tests/test_doIntersect2D.cpp)
target_link_libraries(test_doIntersect2D booleans2d_lib CGAL::CGAL)
add_test(NAME test_doIntersect2D COMMAND test_doIntersect2D)

add_executable(test_segmentIntersections tests/test_segmentIntersections.cpp)
target_link_libraries(test_segmentIntersections booleans2d_lib CGAL::CGAL)
add_test(NAME test_segmentIntersections COMMAND test_segmentIntersections)

add_executable(test_computeArrangement tests/test_computeArrangement.cpp)
target_link_libraries(test_computeArrangement booleans2d_lib CGAL::CGAL)
add_test(NAME test_computeArrangement COMMAND test_computeArrangement)

add_executable(test_extractArrangementFaces tests/test_extractArrangementFaces.cpp)
target_link_libraries(test_extractArrangementFaces booleans2d_lib CGAL::CGAL)
add_test(NAME test_extractArrangementFaces COMMAND test_extractArrangementFaces)
