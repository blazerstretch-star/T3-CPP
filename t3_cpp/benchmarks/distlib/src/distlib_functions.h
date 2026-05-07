#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <utility>
#include "Bucket.h"

using BucketPtr = std::shared_ptr<Bucket>;

namespace distlib {

// Consistent Hash functions
float consistent_hash(const std::string &msg);

// Bucket functions
std::pair<int, float> bucket_partition_point(const std::vector<float>& hash_points, float x);
void bucket_add_partition_point(std::vector<float>& hash_points, float point);
void bucket_remove_partition_point(std::vector<float>& hash_points, unsigned int i);
bool bucket_less_than(const std::string& lhs_name, const std::vector<float>& lhs_points,
                      const std::string& rhs_name, const std::vector<float>& rhs_points);
bool bucket_equals(const std::string& lhs_name, const std::vector<float>& lhs_points,
                   const std::string& rhs_name, const std::vector<float>& rhs_points);

// View functions
BucketPtr view_find_bucket(const std::map<float, BucketPtr>& partition_to_bucket, float hash);
std::unique_ptr<std::map<float, BucketPtr>> make_balanced_view_map(
    int num_buckets, int partitions_per_bucket, std::vector<BucketPtr>& buckets_out);

}
