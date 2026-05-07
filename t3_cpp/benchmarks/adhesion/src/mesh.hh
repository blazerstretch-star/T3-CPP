#pragma once
#include <vector>

template <typename Point, typename Info>
struct Mesh
{
  std::vector<Point>    vertices;
  std::vector<unsigned> data;
  std::vector<unsigned> sizes;
  std::vector<Info>     info;

  size_t size() const { return sizes.size(); }
  
  void push_back(
    std::vector<unsigned> const &vertices,
    Info const &i)
  {
    data.insert(data.end(), vertices.begin(), vertices.end());
    sizes.push_back(vertices.size());
    info.push_back(i);
  }
};
