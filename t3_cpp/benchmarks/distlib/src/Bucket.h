// Copyright (c) 2016-2020 AlertAvert.com. All rights reserved.

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <algorithm>

class Bucket {
private:
  std::string name_;
  std::vector<float> hash_points_;

public:
  Bucket(std::string name, std::vector<float> hash_points) 
    : name_(std::move(name)), hash_points_(std::move(hash_points)) {
    std::sort(hash_points_.begin(), hash_points_.end());
  }

  std::string name() const { return name_; }
  void set_name(std::string name) { name_ = std::move(name); }
  
  std::vector<float> partition_points() const { return hash_points_; }
  
  float partition_point(int i) const {
    if (i < 0 || i >= partitions()) {
      std::ostringstream msg;
      msg << "Requesting partition point #" << i << ", when only "
           << partitions() << " are available";
      throw std::out_of_range(msg.str());
    }
    return hash_points_[i];
  }
  
  int partitions() const { return hash_points_.size(); }
};
