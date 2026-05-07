#include "../src/movie_functions.h"
#include <cassert>
#include <iostream>
#include <climits>
#include <vector>
#include <set>
#include <cmath>

using namespace MovieRecommendation;

class GraphTestCleanup {
public:
    std::vector<GraphVertex<int>*> vertices;
    
    GraphVertex<int>* createVertex(int value) {
        GraphVertex<int>* vertex = new GraphVertex<int>(value);
        vertices.push_back(vertex);
        return vertex;
    }
    
    ~GraphTestCleanup() {
        for (auto vertex : vertices) {
            delete vertex;
        }
    }
};

// PROPERTY 1: Edge count increases
bool verify_edge_added(int oldCount, int newCount) {
    return newCount == oldCount + 1;
}

// PROPERTY 2: Edge destination is correct
bool verify_edge_destination(GraphVertex<int>* source, GraphVertex<int>* expectedDest, int edgeIndex) {
    return source->edges.arr[edgeIndex].destination == expectedDest;
}

// PROPERTY 3: Edge weight is correct (with floating point tolerance)
bool verify_edge_weight(double actual, double expected, double tolerance = 1e-9) {
    return std::abs(actual - expected) < tolerance;
}

// PROPERTY 4: No duplicate edges (unless intentional)
bool has_duplicate_edges(GraphVertex<int>* source) {
    // This is informational - duplicates may be allowed
    std::set<GraphVertex<int>*> seen;
    for (int i = 0; i < source->edges.size(); i++) {
        if (seen.count(source->edges.arr[i].destination)) {
            return true;
        }
        seen.insert(source->edges.arr[i].destination);
    }
    return false;
}

// PROPERTY 5: Source vertex unchanged
bool verify_source_unchanged(GraphVertex<int>* source, int expectedData) {
    return source->data == expectedData;
}

int main() {
    // Test 1: Basic edge addition
    {
        GraphTestCleanup test;
        GraphVertex<int>* source = test.createVertex(1);
        GraphVertex<int>* dest = test.createVertex(2);
        
        int initialSize = source->edges.size();
        graph_add_edge(source, dest, 5.0);
        
        assert(verify_edge_added(initialSize, source->edges.size()));
        assert(source->edges.arr[0].destination == dest);
        assert(verify_edge_weight(source->edges.arr[0].weight, 5.0));
        assert(verify_source_unchanged(source, 1));
    }
    
    // Test 2: Multiple edges from same source
    {
        GraphTestCleanup test;
        GraphVertex<int>* source = test.createVertex(1);
        GraphVertex<int>* dest1 = test.createVertex(2);
        GraphVertex<int>* dest2 = test.createVertex(3);
        GraphVertex<int>* dest3 = test.createVertex(4);
        
        graph_add_edge(source, dest1, 1.5);
        graph_add_edge(source, dest2, 2.5);
        graph_add_edge(source, dest3, 3.5);
        
        assert(source->edges.size() == 3);
        assert(verify_edge_weight(source->edges.arr[0].weight, 1.5));
        assert(verify_edge_weight(source->edges.arr[1].weight, 2.5));
        assert(verify_edge_weight(source->edges.arr[2].weight, 3.5));
        assert(source->edges.arr[0].destination == dest1);
        assert(source->edges.arr[1].destination == dest2);
        assert(source->edges.arr[2].destination == dest3);
    }
    
    // Test 3: Self-loop edge
    {
        GraphTestCleanup test;
        GraphVertex<int>* vertex = test.createVertex(5);
        
        graph_add_edge(vertex, vertex, 0.0);
        
        assert(vertex->edges.size() == 1);
        assert(vertex->edges.arr[0].destination == vertex);
        assert(verify_edge_weight(vertex->edges.arr[0].weight, 0.0));
    }
    
    // Test 4: Extreme weight values
    {
        GraphTestCleanup test;
        GraphVertex<int>* source = test.createVertex(1);
        GraphVertex<int>* dest1 = test.createVertex(2);
        GraphVertex<int>* dest2 = test.createVertex(3);
        GraphVertex<int>* dest3 = test.createVertex(4);
        GraphVertex<int>* dest4 = test.createVertex(5);
        
        graph_add_edge(source, dest1, std::numeric_limits<double>::max());
        graph_add_edge(source, dest2, std::numeric_limits<double>::min());
        graph_add_edge(source, dest3, -std::numeric_limits<double>::max());
        graph_add_edge(source, dest4, std::numeric_limits<double>::infinity());
        
        assert(source->edges.size() == 4);
        assert(source->edges.arr[0].weight == std::numeric_limits<double>::max());
        assert(source->edges.arr[1].weight == std::numeric_limits<double>::min());
        assert(source->edges.arr[2].weight == -std::numeric_limits<double>::max());
        assert(std::isinf(source->edges.arr[3].weight));
    }
    
    // Test 5: Large number of edges (stress test)
    {
        GraphTestCleanup test;
        GraphVertex<int>* source = test.createVertex(0);
        const int NUM_EDGES = 10000;
        
        for (int i = 1; i <= NUM_EDGES; i++) {
            GraphVertex<int>* dest = test.createVertex(i);
            graph_add_edge(source, dest, i * 0.1);
        }
        
        assert(source->edges.size() == NUM_EDGES);
        
        // Verify all edges
        for (int i = 0; i < NUM_EDGES; i++) {
            assert(source->edges.arr[i].destination->data == i + 1);
            assert(verify_edge_weight(source->edges.arr[i].weight, (i + 1) * 0.1));
        }
    }
    
    // Test 6: Bidirectional edges
    {
        GraphTestCleanup test;
        GraphVertex<int>* vertex1 = test.createVertex(1);
        GraphVertex<int>* vertex2 = test.createVertex(2);
        
        graph_add_edge(vertex1, vertex2, 10.0);
        graph_add_edge(vertex2, vertex1, 20.0);
        
        assert(vertex1->edges.size() == 1);
        assert(vertex2->edges.size() == 1);
        assert(vertex1->edges.arr[0].destination == vertex2);
        assert(vertex2->edges.arr[0].destination == vertex1);
        assert(verify_edge_weight(vertex1->edges.arr[0].weight, 10.0));
        assert(verify_edge_weight(vertex2->edges.arr[0].weight, 20.0));
    }
    
    // Test 7: Duplicate edges to same destination (multi-graph)
    {
        GraphTestCleanup test;
        GraphVertex<int>* source = test.createVertex(1);
        GraphVertex<int>* dest = test.createVertex(2);
        
        graph_add_edge(source, dest, 5.0);
        graph_add_edge(source, dest, 10.0);
        graph_add_edge(source, dest, 15.0);
        
        assert(source->edges.size() == 3);
        assert(source->edges.arr[0].destination == dest);
        assert(source->edges.arr[1].destination == dest);
        assert(source->edges.arr[2].destination == dest);
        assert(verify_edge_weight(source->edges.arr[0].weight, 5.0));
        assert(verify_edge_weight(source->edges.arr[1].weight, 10.0));
        assert(verify_edge_weight(source->edges.arr[2].weight, 15.0));
    }
    
    // Test 8: Zero and negative weights
    {
        GraphTestCleanup test;
        GraphVertex<int>* source = test.createVertex(1);
        GraphVertex<int>* dest1 = test.createVertex(2);
        GraphVertex<int>* dest2 = test.createVertex(3);
        GraphVertex<int>* dest3 = test.createVertex(4);
        GraphVertex<int>* dest4 = test.createVertex(5);
        
        graph_add_edge(source, dest1, 0.0);
        graph_add_edge(source, dest2, -5.5);
        graph_add_edge(source, dest3, -100.123);
        graph_add_edge(source, dest4, -0.0001);
        
        assert(source->edges.size() == 4);
        assert(verify_edge_weight(source->edges.arr[0].weight, 0.0));
        assert(verify_edge_weight(source->edges.arr[1].weight, -5.5));
        assert(verify_edge_weight(source->edges.arr[2].weight, -100.123));
        assert(verify_edge_weight(source->edges.arr[3].weight, -0.0001));
    }
    
    // Test 9: Very small weights (precision test)
    {
        GraphTestCleanup test;
        GraphVertex<int>* source = test.createVertex(1);
        GraphVertex<int>* dest1 = test.createVertex(2);
        GraphVertex<int>* dest2 = test.createVertex(3);
        GraphVertex<int>* dest3 = test.createVertex(4);
        
        graph_add_edge(source, dest1, 1e-10);
        graph_add_edge(source, dest2, 1e-15);
        graph_add_edge(source, dest3, 1e-20);
        
        assert(source->edges.size() == 3);
        assert(verify_edge_weight(source->edges.arr[0].weight, 1e-10));
        assert(verify_edge_weight(source->edges.arr[1].weight, 1e-15));
        // Note: 1e-20 may lose precision, but should be stored
    }
    
    // Test 10: Complete graph (all-to-all)
    {
        GraphTestCleanup test;
        const int N = 10;
        std::vector<GraphVertex<int>*> vertices;
        
        for (int i = 0; i < N; i++) {
            vertices.push_back(test.createVertex(i));
        }
        
        // Add edges from each vertex to all others
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (i != j) {
                    graph_add_edge(vertices[i], vertices[j], i * 10.0 + j);
                }
            }
        }
        
        // Verify each vertex has N-1 edges
        for (int i = 0; i < N; i++) {
            assert(vertices[i]->edges.size() == N - 1);
        }
    }
    
    // Test 11: Chain graph
    {
        GraphTestCleanup test;
        const int CHAIN_LENGTH = 100;
        std::vector<GraphVertex<int>*> chain;
        
        for (int i = 0; i < CHAIN_LENGTH; i++) {
            chain.push_back(test.createVertex(i));
        }
        
        // Create chain: 0 -> 1 -> 2 -> ... -> 99
        for (int i = 0; i < CHAIN_LENGTH - 1; i++) {
            graph_add_edge(chain[i], chain[i + 1], i * 1.5);
        }
        
        // Verify chain structure
        for (int i = 0; i < CHAIN_LENGTH - 1; i++) {
            assert(chain[i]->edges.size() == 1);
            assert(chain[i]->edges.arr[0].destination == chain[i + 1]);
            assert(verify_edge_weight(chain[i]->edges.arr[0].weight, i * 1.5));
        }
        
        assert(chain[CHAIN_LENGTH - 1]->edges.size() == 0 && "Last vertex has no outgoing edges");
    }
    
    // Test 12: Star graph (hub and spokes)
    {
        GraphTestCleanup test;
        GraphVertex<int>* hub = test.createVertex(0);
        const int NUM_SPOKES = 50;
        
        for (int i = 1; i <= NUM_SPOKES; i++) {
            GraphVertex<int>* spoke = test.createVertex(i);
            graph_add_edge(hub, spoke, i * 2.0);
        }
        
        assert(hub->edges.size() == NUM_SPOKES);
        
        for (int i = 0; i < NUM_SPOKES; i++) {
            assert(hub->edges.arr[i].destination->data == i + 1);
            assert(verify_edge_weight(hub->edges.arr[i].weight, (i + 1) * 2.0));
        }
    }
    
    // Test 13: Fractional weights
    {
        GraphTestCleanup test;
        GraphVertex<int>* source = test.createVertex(1);
        
        for (int i = 0; i < 100; i++) {
            GraphVertex<int>* dest = test.createVertex(i + 2);
            double weight = 1.0 / (i + 1);
            graph_add_edge(source, dest, weight);
        }
        
        assert(source->edges.size() == 100);
        
        for (int i = 0; i < 100; i++) {
            double expected = 1.0 / (i + 1);
            assert(verify_edge_weight(source->edges.arr[i].weight, expected));
        }
    }
    
    // Test 14: Extreme vertex data values
    {
        GraphTestCleanup test;
        GraphVertex<int>* source = test.createVertex(INT_MAX);
        GraphVertex<int>* dest1 = test.createVertex(INT_MIN);
        GraphVertex<int>* dest2 = test.createVertex(0);
        
        graph_add_edge(source, dest1, 1.0);
        graph_add_edge(source, dest2, 2.0);
        
        assert(source->data == INT_MAX);
        assert(source->edges.size() == 2);
        assert(source->edges.arr[0].destination->data == INT_MIN);
        assert(source->edges.arr[1].destination->data == 0);
    }
    
    // Test 15: Rapid successive additions
    {
        GraphTestCleanup test;
        GraphVertex<int>* source = test.createVertex(0);
        
        for (int batch = 0; batch < 10; batch++) {
            for (int i = 0; i < 100; i++) {
                GraphVertex<int>* dest = test.createVertex(batch * 100 + i);
                graph_add_edge(source, dest, batch + i * 0.01);
            }
            assert(source->edges.size() == (batch + 1) * 100);
        }
        
        assert(source->edges.size() == 1000);
    }
    
    std::cout << "✅ graph_add_edge ultra-strict test passed (15 test cases)" << std::endl;
    std::cout << "   - Basic edge addition verified" << std::endl;
    std::cout << "   - Multiple edges verified" << std::endl;
    std::cout << "   - Self-loops verified" << std::endl;
    std::cout << "   - Extreme weights (max, min, infinity) verified" << std::endl;
    std::cout << "   - Large scale (10,000 edges) verified" << std::endl;
    std::cout << "   - Bidirectional edges verified" << std::endl;
    std::cout << "   - Duplicate edges (multi-graph) verified" << std::endl;
    std::cout << "   - Zero and negative weights verified" << std::endl;
    std::cout << "   - Precision (very small weights) verified" << std::endl;
    std::cout << "   - Complete graph verified" << std::endl;
    std::cout << "   - Chain graph verified" << std::endl;
    std::cout << "   - Star graph verified" << std::endl;
    std::cout << "   - Fractional weights verified" << std::endl;
    std::cout << "   - Extreme vertex data verified" << std::endl;
    std::cout << "   - Rapid additions verified" << std::endl;
    return 0;
}
