#pragma once
#include "leveldb_types.h"
#include <vector>

namespace leveldb {

// ===== EASY FUNCTIONS (8) =====

// Fixed-width encoding/decoding
void EncodeFixed32(char* dst, uint32_t value);
void EncodeFixed64(char* dst, uint64_t value);
uint32_t DecodeFixed32(const char* ptr);
uint64_t DecodeFixed64(const char* ptr);

// Varint utilities
int VarintLength(uint64_t v);

// CRC32C utilities
uint32_t crc32c_Mask(uint32_t crc);
uint32_t crc32c_Unmask(uint32_t masked_crc);

// Key extraction
Slice ExtractUserKey(const Slice& internal_key);

// ===== MEDIUM FUNCTIONS (20) =====

// Varint encoding/decoding
char* EncodeVarint32(char* dst, uint32_t value);
char* EncodeVarint64(char* dst, uint64_t value);
const char* GetVarint32Ptr(const char* p, const char* limit, uint32_t* value);
const char* GetVarint64Ptr(const char* p, const char* limit, uint64_t* value);
bool GetVarint32(Slice* input, uint32_t* value);
bool GetVarint64(Slice* input, uint64_t* value);

// String encoding
void PutFixed32(std::string* dst, uint32_t value);
void PutFixed64(std::string* dst, uint64_t value);
void PutVarint32(std::string* dst, uint32_t value);
void PutVarint64(std::string* dst, uint64_t value);
void PutLengthPrefixedSlice(std::string* dst, const Slice& value);
bool GetLengthPrefixedSlice(Slice* input, Slice* result);

// Hash functions
uint32_t Hash(const char* data, size_t n, uint32_t seed);
uint32_t BloomHash(const Slice& key);

// CRC32C
uint32_t crc32c_Extend(uint32_t init_crc, const char* data, size_t n);
uint32_t crc32c_Value(const char* data, size_t n);

// Internal key operations
void AppendInternalKey(std::string* result, const Slice& user_key, 
                       uint64_t sequence, uint8_t type);
bool ParseInternalKey(const Slice& internal_key, Slice* user_key,
                      uint64_t* sequence, uint8_t* type);

// Prefix compression
size_t CalculateSharedPrefix(const Slice& a, const Slice& b);

// Random number generation
uint32_t RandomSeed();

// ===== HARD FUNCTIONS (12) =====

// Random class for skip list
class Random {
 public:
  explicit Random(uint32_t s);
  uint32_t Next();
  uint32_t Uniform(int n);
  bool OneIn(int n);
 private:
  uint32_t seed_;
};

// Skip list helper
int SkipListRandomHeight(Random* rnd, int max_height);

// Bloom filter operations
void BloomFilter_CreateFilter(const Slice* keys, int n, int bits_per_key, std::string* dst);
bool BloomFilter_KeyMayMatch(const Slice& key, const Slice& bloom_filter);
uint32_t BloomFilter_DoubleHash(uint32_t h, int probe_num);

// LRU cache operations  
bool LRUCache_Lookup(const Slice& key, uint32_t hash, void** value);
void LRUCache_Evict(size_t target_usage);

// Block builder operations
void BlockBuilder_Add(std::string* buffer, std::string* last_key, 
                      const Slice& key, const Slice& value, 
                      int restart_interval, int* counter,
                      std::vector<uint32_t>* restarts);
Slice BlockBuilder_Finish(std::string* buffer, const std::vector<uint32_t>& restarts);

}  // namespace leveldb
