#include "../src/route_planning_functions.h"
#include <cassert>
#include <iostream>

using namespace RoutePlanning;

int main() {
    // Test 1: All road types
    Graph mixed;
    for(int i = 0; i < 8; i++) {
        mixed.nodes.emplace_back(i * 1.0f, 0.0f, i);
    }
    
    // Create roads of different types (only using existing enum values)
    Road::Type types[] = {Road::Type::Primary, Road::Type::Secondary, Road::Type::Tertiary,
                         Road::Type::Residential, Road::Type::Unclassified, Road::Type::Footway,
                         Road::Type::Motorway, Road::Type::Trunk};
    
    for(int i = 0; i < 8; i++) {
        Road road;
        road.type = types[i];
        road.way = i;
        mixed.roads.push_back(road);
        
        Way way;
        way.nodes = {i};
        mixed.ways.push_back(way);
    }
    
    // Test closest node with different preferences
    Node* closest = find_closest_node(mixed, 0.1f, 0.0f);
    assert(closest->index == 0); // Should prefer primary road
    
    closest = find_closest_node(mixed, 1.1f, 0.0f);
    assert(closest->index == 1); // Should prefer secondary over footway
    
    // Test 2: Mixed road network
    Graph network;
    for(int i = 0; i < 12; i++) {
        network.nodes.emplace_back((i % 4) * 2.0f, (i / 4) * 2.0f, i);
    }
    
    // Primary roads (main arteries)
    Road primary1, primary2;
    primary1.type = Road::Type::Primary; primary1.way = 0;
    primary2.type = Road::Type::Primary; primary2.way = 1;
    network.roads.insert(network.roads.end(), {primary1, primary2});
    
    Way primary_way1, primary_way2;
    primary_way1.nodes = {0, 1, 2, 3};
    primary_way2.nodes = {0, 4, 8};
    network.ways.insert(network.ways.end(), {primary_way1, primary_way2});
    
    // Residential roads (local streets)
    for(int i = 2; i < 6; i++) {
        Road res;
        res.type = Road::Type::Residential;
        res.way = i;
        network.roads.push_back(res);
        
        Way res_way;
        res_way.nodes = {i*2, i*2+1};
        network.ways.push_back(res_way);
    }
    
    // Unclassified roads (should be deprioritized)
    Road unclassified;
    unclassified.type = Road::Type::Unclassified;
    unclassified.way = 6;
    network.roads.push_back(unclassified);
    
    Way unclassified_way;
    unclassified_way.nodes = {10, 11};
    network.ways.push_back(unclassified_way);
    
    closest = find_closest_node(network, 5.9f, 3.9f);
    assert(closest->index != 11); // Should not pick unclassified road node
    
    // Test 3: Footway vs road preference
    Graph pedestrian;
    pedestrian.nodes.emplace_back(0.0f, 0.0f, 0);
    pedestrian.nodes.emplace_back(0.1f, 0.0f, 1);
    
    Road road_road, foot_road;
    road_road.type = Road::Type::Secondary; road_road.way = 0;
    foot_road.type = Road::Type::Footway; foot_road.way = 1;
    pedestrian.roads.insert(pedestrian.roads.end(), {road_road, foot_road});
    
    Way road_way, foot_way;
    road_way.nodes = {0}; foot_way.nodes = {1};
    pedestrian.ways.insert(pedestrian.ways.end(), {road_way, foot_way});
    
    closest = find_closest_node(pedestrian, 0.05f, 0.0f);
    assert(closest->index == 0); // Should prefer road over footway
    
    // Test 4: Trunk road handling
    Graph trunk_test;
    trunk_test.nodes.emplace_back(10.0f, 10.0f, 0);
    trunk_test.nodes.emplace_back(10.1f, 10.0f, 1);
    
    Road trunk_road;
    trunk_road.type = Road::Type::Trunk;
    trunk_road.way = 0;
    trunk_test.roads.push_back(trunk_road);
    
    Way trunk_way;
    trunk_way.nodes = {1};
    trunk_test.ways.push_back(trunk_way);
    
    closest = find_closest_node(trunk_test, 10.05f, 10.0f);
    assert(closest->index == 0); // Should fall back to any available node
    
    // Test 5: Road hierarchy preference
    Graph hierarchy;
    for(int i = 0; i < 4; i++) {
        hierarchy.nodes.emplace_back(i * 0.1f, 0.0f, i);
    }
    
    Road::Type hierarchy_types[] = {Road::Type::Tertiary, Road::Type::Primary, 
                                   Road::Type::Secondary, Road::Type::Residential};
    
    for(int i = 0; i < 4; i++) {
        Road road;
        road.type = hierarchy_types[i];
        road.way = i;
        hierarchy.roads.push_back(road);
        
        Way way;
        way.nodes = {i};
        hierarchy.ways.push_back(way);
    }
    
    closest = find_closest_node(hierarchy, 0.15f, 0.0f);
    assert(closest->index == 1); // Should prefer primary road
    
    std::cout << "test_road_types passed!" << std::endl;
    return 0;
}