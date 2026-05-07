#include "distlib_functions.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <openssl/md5.h>
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
#include <cmath>
#include <string>
#include <memory>
#include <utility>


namespace distlib {

// Constants from original
inline const unsigned long kModulo = 65535;
inline const unsigned long kBase = 13;
constexpr float kEpsilon = 1e-6;

// Helper function from utils (simplified)
inline size_t basic_hash(const char* value, size_t len, unsigned char** hash_value) {
    *hash_value = new unsigned char[MD5_DIGEST_LENGTH];
    MD5((unsigned char*)value, len, *hash_value);
    return MD5_DIGEST_LENGTH;
}

float consistent_hash(const std::string &msg) {
  // FUNCTION_ID: distlib_func001 - START
  unsigned char* digest;
  unsigned long sum = 0;

  basic_hash(msg.c_str(), msg.length(), &digest);
  for (int i = 0; i < MD5_DIGEST_LENGTH - 1; i += 2) {
    sum += kBase * (unsigned long) (digest[i] + digest[i+1] * 16);
  }
  delete[](digest);

  return float(sum % kModulo) / kModulo;
  // FUNCTION_ID: distlib_func001 - END
}

std::pair<int, float> bucket_partition_point(const std::vector<float>& hash_points, float x) {
  // FUNCTION_ID: distlib_func002 - START
  auto pos = std::upper_bound(hash_points.begin(), hash_points.end(), x);
  if (pos == hash_points.end()) {
    return std::make_pair(0, hash_points[0]);
  }
  return std::make_pair(std::distance(hash_points.cbegin(), pos), *pos);
  // FUNCTION_ID: distlib_func002 - END
}

void bucket_add_partition_point(std::vector<float>& hash_points, float point) {
  // FUNCTION_ID: distlib_func003 - START
  auto pos = hash_points.begin();
  for (auto x: hash_points) {
    if (x > point) {
      break;
    }
    pos++;
  }
  if (pos != hash_points.end()) {
    hash_points.insert(pos, point);
  } else {
    hash_points.push_back(point);
  }
  // FUNCTION_ID: distlib_func003 - END
}

void bucket_remove_partition_point(std::vector<float>& hash_points, unsigned int i) {
  // FUNCTION_ID: distlib_func004 - START
  if (i < hash_points.size()) {
    hash_points.erase(hash_points.cbegin() + i);
  }
  // FUNCTION_ID: distlib_func004 - END
}

bool bucket_less_than(const std::string& lhs_name, const std::vector<float>& lhs_points, const std::string& rhs_name, const std::vector<float>& rhs_points) {
  // FUNCTION_ID: distlib_func005 - START
  if (lhs_name != rhs_name) {
    return lhs_name < rhs_name;
  }
  int count = std::min(lhs_points.size(), rhs_points.size());
  for (int i = 0; i < count; ++i) {
    if (std::abs(lhs_points[i] - rhs_points[i]) > kEpsilon) {
      return lhs_points[i] < rhs_points[i];
    }
  }
  return false;
  // FUNCTION_ID: distlib_func005 - END
}

bool bucket_equals(const std::string& lhs_name, const std::vector<float>& lhs_points, const std::string& rhs_name, const std::vector<float>& rhs_points) {
  // FUNCTION_ID: distlib_func006 - START
  return !bucket_less_than(lhs_name, lhs_points, rhs_name, rhs_points) && 
         !bucket_less_than(rhs_name, rhs_points, lhs_name, lhs_points);
  // FUNCTION_ID: distlib_func006 - END
}

BucketPtr view_find_bucket(const std::map<float, BucketPtr>& partition_to_bucket, float hash) {
  // FUNCTION_ID: distlib_func007 - START
  if (hash < 0.0f || hash > 1.10000001f) {
    throw std::invalid_argument(
        "Hash should always be in the [0, 1] interval, was: " + std::to_string(hash));
  }

  if (partition_to_bucket.empty()) {
    throw std::invalid_argument("No buckets in this View");
  }
  auto pos = partition_to_bucket.upper_bound(hash);

  if (pos == partition_to_bucket.end()) {
    return partition_to_bucket.begin()->second;
  } else {
    return pos->second;
  }
  // FUNCTION_ID: distlib_func007 - END
}

std::unique_ptr<std::map<float, BucketPtr>> make_balanced_view_map(int num_buckets, int partitions_per_bucket, std::vector<BucketPtr>& buckets_out) {
  // FUNCTION_ID: distlib_func008 - START
  if (num_buckets <= 0 || partitions_per_bucket <= 0) {
    throw std::invalid_argument("num_buckets and partitions_per_bucket must both be non-zero");
  }
  
  auto partition_map = std::make_unique<std::map<float, BucketPtr>>();
  
  std::vector<std::vector<float>> hash_points{static_cast<unsigned long>(num_buckets)};
  for (int i = 0; i < num_buckets; ++i) {
    hash_points[i] = std::vector<float>(partitions_per_bucket);
  }

  float delta = 1.0f / (num_buckets * partitions_per_bucket);
  float x = delta;

  for (int j = 0; j < partitions_per_bucket; ++j) {
    for (int i = 0; i < num_buckets; ++i) {
      hash_points[i][j] = x;
      x += delta;
    }
  }

  for (int i = 0; i < num_buckets; ++i) {
    auto pb = std::make_shared<Bucket>("bucket-" + std::to_string(i), hash_points[i]);
    buckets_out.push_back(pb);
    for (int j = 0; j < pb->partitions(); j++) {
      float point = pb->partition_point(j);
      (*partition_map)[point] = pb;
    }
  }

  return partition_map;
  // FUNCTION_ID: distlib_func008 - END
}

}
