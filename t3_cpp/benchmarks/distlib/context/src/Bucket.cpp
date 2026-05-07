// Copyright (c) 2016-2020 AlertAvert.com. All rights reserved.
// Created by M. Massenzio (marco@alertavert.com) on 3/6/16.


#include "Bucket.hpp"

#include <algorithm>
#include <ios>
#include <math.h>
#include <utility>

// Tolerance for comparing floating point numbers.
constexpr float kEpsilon = 1e-6;

std::ostream &operator<<(std::ostream &out, const Bucket &bucket) {
    out << "'" << bucket.name() << "' [";
    out.setf(std::ios_base::fixed);
    out.precision(5);

    for (int i = 0; i < bucket.partitions(); ++i) {
        if (i > 0) out << ", ";
        out << bucket.partition_point(i);
    }
    out << "]";

    return out;
}

Bucket::Bucket(std::string name, std::vector<float> hash_points) :
        name_(std::move(name)), hash_points_(std::move(hash_points)) {
    std::sort(hash_points_.begin(), hash_points_.end());
}

std::pair<int, float> Bucket::partition_point(float x) const {
    // FUNCTION_ID: distlib_func002 - START
    auto pos = std::upper_bound(hash_points_.begin(), hash_points_.end(), x);
    if (pos == hash_points_.end()) {
        return std::make_pair(0, hash_points_[0]);
    }
    return std::make_pair(std::distance(hash_points_.cbegin(), pos), *pos);
    // FUNCTION_ID: distlib_func002 - END
}

void Bucket::add_partition_point(float point) {
    // FUNCTION_ID: distlib_func003 - START
    auto pos = hash_points_.begin();
    for (auto x: hash_points_) {
        if (x > point) {
            break;
        }
        pos++;
    }
    if (pos != hash_points_.end()) {
        hash_points_.insert(pos, point);
    } else {
        hash_points_.push_back(point);
    }
    // FUNCTION_ID: distlib_func003 - END
}

void Bucket::remove_partition_point(unsigned int i) {
    // FUNCTION_ID: distlib_func004 - START
    if (i < partitions()) {
        hash_points_.erase(hash_points_.cbegin() + i);
    }
    // FUNCTION_ID: distlib_func004 - END
}

Bucket::operator json() const {
    return nlohmann::json{
            {"name",             name()},
            {"partition_points", partition_points()}
    };
}

bool operator<(const Bucket &lhs, const Bucket &rhs) {
    // FUNCTION_ID: distlib_func005 - START
    if (lhs.name() != rhs.name()) {
        return lhs.name() < rhs.name();
    }
    int count = std::min(lhs.partitions(), rhs.partitions());
    for (int i = 0; i < count; ++i) {
        if (std::abs(lhs.partition_point(i) - rhs.partition_point(i)) > kEpsilon) {
            return lhs.partition_point(i) < rhs.partition_point(i);
        }
    }
    return false;
    // FUNCTION_ID: distlib_func005 - END
}

bool operator==(const Bucket &lhs, const Bucket &rhs) {
    // FUNCTION_ID: distlib_func006 - START
    return !(lhs < rhs) && !(rhs < lhs);
    // FUNCTION_ID: distlib_func006 - END
}
