#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <cassert>

namespace leveldb {

// Minimal Slice implementation
class Slice {
 public:
  Slice() : data_(""), size_(0) {}
  Slice(const char* d, size_t n) : data_(d), size_(n) {}
  Slice(const std::string& s) : data_(s.data()), size_(s.size()) {}
  Slice(const char* s) : data_(s), size_(strlen(s)) {}

  const char* data() const { return data_; }
  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }
  char operator[](size_t n) const { return data_[n]; }
  
  void remove_prefix(size_t n) {
    assert(n <= size_);
    data_ += n;
    size_ -= n;
  }

 private:
  const char* data_;
  size_t size_;
};

}  // namespace leveldb
