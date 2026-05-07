#include "leveldb_functions.h"
#include <vector>
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


namespace leveldb {

// ===== EASY FUNCTIONS =====

void EncodeFixed32(char* dst, uint32_t value) {
  // FUNCTION_ID: leveldb_func001 - START
  uint8_t* const buffer = reinterpret_cast<uint8_t*>(dst);
  buffer[0] = static_cast<uint8_t>(value);
  buffer[1] = static_cast<uint8_t>(value >> 8);
  buffer[2] = static_cast<uint8_t>(value >> 16);
  buffer[3] = static_cast<uint8_t>(value >> 24);
  // FUNCTION_ID: leveldb_func001 - END
}

void EncodeFixed64(char* dst, uint64_t value) {
  // FUNCTION_ID: leveldb_func002 - START
  uint8_t* const buffer = reinterpret_cast<uint8_t*>(dst);
  buffer[0] = static_cast<uint8_t>(value);
  buffer[1] = static_cast<uint8_t>(value >> 8);
  buffer[2] = static_cast<uint8_t>(value >> 16);
  buffer[3] = static_cast<uint8_t>(value >> 24);
  buffer[4] = static_cast<uint8_t>(value >> 32);
  buffer[5] = static_cast<uint8_t>(value >> 40);
  buffer[6] = static_cast<uint8_t>(value >> 48);
  buffer[7] = static_cast<uint8_t>(value >> 56);
  // FUNCTION_ID: leveldb_func002 - END
}

uint32_t DecodeFixed32(const char* ptr) {
  // FUNCTION_ID: leveldb_func003 - START
  const uint8_t* const buffer = reinterpret_cast<const uint8_t*>(ptr);
  return (static_cast<uint32_t>(buffer[0])) |
         (static_cast<uint32_t>(buffer[1]) << 8) |
         (static_cast<uint32_t>(buffer[2]) << 16) |
         (static_cast<uint32_t>(buffer[3]) << 24);
  // FUNCTION_ID: leveldb_func003 - END
}

uint64_t DecodeFixed64(const char* ptr) {
  // FUNCTION_ID: leveldb_func004 - START
  const uint8_t* const buffer = reinterpret_cast<const uint8_t*>(ptr);
  return (static_cast<uint64_t>(buffer[0])) |
         (static_cast<uint64_t>(buffer[1]) << 8) |
         (static_cast<uint64_t>(buffer[2]) << 16) |
         (static_cast<uint64_t>(buffer[3]) << 24) |
         (static_cast<uint64_t>(buffer[4]) << 32) |
         (static_cast<uint64_t>(buffer[5]) << 40) |
         (static_cast<uint64_t>(buffer[6]) << 48) |
         (static_cast<uint64_t>(buffer[7]) << 56);
  // FUNCTION_ID: leveldb_func004 - END
}

int VarintLength(uint64_t v) {
  // FUNCTION_ID: leveldb_func005 - START
  int len = 1;
  while (v >= 128) {
    v >>= 7;
    len++;
  }
  return len;
  // FUNCTION_ID: leveldb_func005 - END
}

uint32_t crc32c_Mask(uint32_t crc) {
  // FUNCTION_ID: leveldb_func006 - START
  return ((crc >> 15) | (crc << 17)) + 0xa282ead8ul;
  // FUNCTION_ID: leveldb_func006 - END
}

uint32_t crc32c_Unmask(uint32_t masked_crc) {
  // FUNCTION_ID: leveldb_func007 - START
  uint32_t rot = masked_crc - 0xa282ead8ul;
  return ((rot >> 17) | (rot << 15));
  // FUNCTION_ID: leveldb_func007 - END
}

Slice ExtractUserKey(const Slice& internal_key) {
  // FUNCTION_ID: leveldb_func008 - START
  assert(internal_key.size() >= 8);
  return Slice(internal_key.data(), internal_key.size() - 8);
  // FUNCTION_ID: leveldb_func008 - END
}

// ===== MEDIUM FUNCTIONS =====

char* EncodeVarint32(char* dst, uint32_t v) {
  // FUNCTION_ID: leveldb_func009 - START
  uint8_t* ptr = reinterpret_cast<uint8_t*>(dst);
  static const int B = 128;
  if (v < (1 << 7)) {
    *(ptr++) = v;
  } else if (v < (1 << 14)) {
    *(ptr++) = v | B;
    *(ptr++) = v >> 7;
  } else if (v < (1 << 21)) {
    *(ptr++) = v | B;
    *(ptr++) = (v >> 7) | B;
    *(ptr++) = v >> 14;
  } else if (v < (1 << 28)) {
    *(ptr++) = v | B;
    *(ptr++) = (v >> 7) | B;
    *(ptr++) = (v >> 14) | B;
    *(ptr++) = v >> 21;
  } else {
    *(ptr++) = v | B;
    *(ptr++) = (v >> 7) | B;
    *(ptr++) = (v >> 14) | B;
    *(ptr++) = (v >> 21) | B;
    *(ptr++) = v >> 28;
  }
  return reinterpret_cast<char*>(ptr);
  // FUNCTION_ID: leveldb_func009 - END
}

char* EncodeVarint64(char* dst, uint64_t v) {
  // FUNCTION_ID: leveldb_func010 - START
  static const int B = 128;
  uint8_t* ptr = reinterpret_cast<uint8_t*>(dst);
  while (v >= B) {
    *(ptr++) = v | B;
    v >>= 7;
  }
  *(ptr++) = static_cast<uint8_t>(v);
  return reinterpret_cast<char*>(ptr);
  // FUNCTION_ID: leveldb_func010 - END
}

const char* GetVarint32Ptr(const char* p, const char* limit, uint32_t* value) {
  // FUNCTION_ID: leveldb_func011 - START
  if (p < limit) {
    uint32_t result = *(reinterpret_cast<const uint8_t*>(p));
    if ((result & 128) == 0) {
      *value = result;
      return p + 1;
    }
  }
  
  // Fallback: decode multi-byte varint
  uint32_t result = 0;
  for (uint32_t shift = 0; shift <= 28 && p < limit; shift += 7) {
    uint32_t byte = *(reinterpret_cast<const uint8_t*>(p));
    p++;
    if (byte & 128) {
      result |= ((byte & 127) << shift);
    } else {
      result |= (byte << shift);
      *value = result;
      return reinterpret_cast<const char*>(p);
    }
  }
  return nullptr;
  // FUNCTION_ID: leveldb_func011 - END
}

const char* GetVarint64Ptr(const char* p, const char* limit, uint64_t* value) {
  // FUNCTION_ID: leveldb_func012 - START
  uint64_t result = 0;
  for (uint32_t shift = 0; shift <= 63 && p < limit; shift += 7) {
    uint64_t byte = *(reinterpret_cast<const uint8_t*>(p));
    p++;
    if (byte & 128) {
      result |= ((byte & 127) << shift);
    } else {
      result |= (byte << shift);
      *value = result;
      return reinterpret_cast<const char*>(p);
    }
  }
  return nullptr;
  // FUNCTION_ID: leveldb_func012 - END
}

bool GetVarint32(Slice* input, uint32_t* value) {
  // FUNCTION_ID: leveldb_func013 - START
  const char* p = input->data();
  const char* limit = p + input->size();
  const char* q = GetVarint32Ptr(p, limit, value);
  if (q == nullptr) {
    return false;
  } else {
    *input = Slice(q, limit - q);
    return true;
  }
  // FUNCTION_ID: leveldb_func013 - END
}

bool GetVarint64(Slice* input, uint64_t* value) {
  // FUNCTION_ID: leveldb_func014 - START
  const char* p = input->data();
  const char* limit = p + input->size();
  const char* q = GetVarint64Ptr(p, limit, value);
  if (q == nullptr) {
    return false;
  } else {
    *input = Slice(q, limit - q);
    return true;
  }
  // FUNCTION_ID: leveldb_func014 - END
}

void PutFixed32(std::string* dst, uint32_t value) {
  // FUNCTION_ID: leveldb_func015 - START
  char buf[sizeof(value)];
  EncodeFixed32(buf, value);
  dst->append(buf, sizeof(buf));
  // FUNCTION_ID: leveldb_func015 - END
}

void PutFixed64(std::string* dst, uint64_t value) {
  // FUNCTION_ID: leveldb_func016 - START
  char buf[sizeof(value)];
  EncodeFixed64(buf, value);
  dst->append(buf, sizeof(buf));
  // FUNCTION_ID: leveldb_func016 - END
}

void PutVarint32(std::string* dst, uint32_t v) {
  // FUNCTION_ID: leveldb_func017 - START
  char buf[5];
  char* ptr = EncodeVarint32(buf, v);
  dst->append(buf, ptr - buf);
  // FUNCTION_ID: leveldb_func017 - END
}

void PutVarint64(std::string* dst, uint64_t v) {
  // FUNCTION_ID: leveldb_func018 - START
  char buf[10];
  char* ptr = EncodeVarint64(buf, v);
  dst->append(buf, ptr - buf);
  // FUNCTION_ID: leveldb_func018 - END
}

void PutLengthPrefixedSlice(std::string* dst, const Slice& value) {
  // FUNCTION_ID: leveldb_func019 - START
  PutVarint32(dst, value.size());
  dst->append(value.data(), value.size());
  // FUNCTION_ID: leveldb_func019 - END
}

bool GetLengthPrefixedSlice(Slice* input, Slice* result) {
  // FUNCTION_ID: leveldb_func020 - START
  uint32_t len;
  if (GetVarint32(input, &len) && input->size() >= len) {
    *result = Slice(input->data(), len);
    input->remove_prefix(len);
    return true;
  } else {
    return false;
  }
  // FUNCTION_ID: leveldb_func020 - END
}

uint32_t Hash(const char* data, size_t n, uint32_t seed) {
  // FUNCTION_ID: leveldb_func021 - START
  const uint32_t m = 0xc6a4a793;
  const uint32_t r = 24;
  const char* limit = data + n;
  uint32_t h = seed ^ (n * m);

  while (data + 4 <= limit) {
    uint32_t w = DecodeFixed32(data);
    data += 4;
    h += w;
    h *= m;
    h ^= (h >> 16);
  }

  switch (limit - data) {
    case 3:
      h += static_cast<uint8_t>(data[2]) << 16;
    case 2:
      h += static_cast<uint8_t>(data[1]) << 8;
    case 1:
      h += static_cast<uint8_t>(data[0]);
      h *= m;
      h ^= (h >> r);
      break;
  }
  return h;
  // FUNCTION_ID: leveldb_func021 - END
}

uint32_t BloomHash(const Slice& key) {
  // FUNCTION_ID: leveldb_func022 - START
  return Hash(key.data(), key.size(), 0xbc9f1d34);
  // FUNCTION_ID: leveldb_func022 - END
}

uint32_t crc32c_Extend(uint32_t init_crc, const char* data, size_t n) {
  // FUNCTION_ID: leveldb_func023 - START
  static const uint32_t table[256] = {
    0x00000000, 0xf26b8303, 0xe13b70f7, 0x1350f3f4,
    0xc79a971f, 0x35f1141c, 0x26a1e7e8, 0xd4ca64eb,
    0x8ad958cf, 0x78b2dbcc, 0x6be22838, 0x9989ab3b,
    0x4d43cfd0, 0xbf284cd3, 0xac78bf27, 0x5e133c24,
    0x105ec76f, 0xe235446c, 0xf165b798, 0x030e349b,
    0xd7c45070, 0x25afd373, 0x36ff2087, 0xc494a384,
    0x9a879fa0, 0x68ec1ca3, 0x7bbcef57, 0x89d76c54,
    0x5d1d08bf, 0xaf768bbc, 0xbc267848, 0x4e4dfb4b,
    0x20bd8ede, 0xd2d60ddd, 0xc186fe29, 0x33ed7d2a,
    0xe72719c1, 0x154c9ac2, 0x061c6936, 0xf477ea35,
    0xaa64d611, 0x580f5512, 0x4b5fa6e6, 0xb93425e5,
    0x6dfe410e, 0x9f95c20d, 0x8cc531f9, 0x7eaeb2fa,
    0x30e349b1, 0xc288cab2, 0xd1d83946, 0x23b3ba45,
    0xf779deae, 0x05125dad, 0x1642ae59, 0xe4292d5a,
    0xba3a117e, 0x4851927d, 0x5b016189, 0xa96ae28a,
    0x7da08661, 0x8fcb0562, 0x9c9bf696, 0x6ef07595,
    0x417b1dbc, 0xb3109ebf, 0xa0406d4b, 0x522bee48,
    0x86e18aa3, 0x748a09a0, 0x67dafa54, 0x95b17957,
    0xcba24573, 0x39c9c670, 0x2a993584, 0xd8f2b687,
    0x0c38d26c, 0xfe53516f, 0xed03a29b, 0x1f682198,
    0x5125dad3, 0xa34e59d0, 0xb01eaa24, 0x42752927,
    0x96bf4dcc, 0x64d4cecf, 0x77843d3b, 0x85efbe38,
    0xdbfc821c, 0x2997011f, 0x3ac7f2eb, 0xc8ac71e8,
    0x1c661503, 0xee0d9600, 0xfd5d65f4, 0x0f36e6f7,
    0x61c69362, 0x93ad1061, 0x80fde395, 0x72966096,
    0xa65c047d, 0x5437877e, 0x4767748a, 0xb50cf789,
    0xeb1fcbad, 0x197448ae, 0x0a24bb5a, 0xf84f3859,
    0x2c855cb2, 0xdeeedfb1, 0xcdbe2c45, 0x3fd5af46,
    0x7198540d, 0x83f3d70e, 0x90a324fa, 0x62c8a7f9,
    0xb602c312, 0x44694011, 0x5739b3e5, 0xa55230e6,
    0xfb410cc2, 0x092a8fc1, 0x1a7a7c35, 0xe811ff36,
    0x3cdb9bdd, 0xceb018de, 0xdde0eb2a, 0x2f8b6829,
    0x82f63b78, 0x709db87b, 0x63cd4b8f, 0x91a6c88c,
    0x456cac67, 0xb7072f64, 0xa457dc90, 0x563c5f93,
    0x082f63b7, 0xfa44e0b4, 0xe9141340, 0x1b7f9043,
    0xcfb5f4a8, 0x3dde77ab, 0x2e8e845f, 0xdce5075c,
    0x92a8fc17, 0x60c37f14, 0x73938ce0, 0x81f80fe3,
    0x55326b08, 0xa759e80b, 0xb4091bff, 0x466298fc,
    0x1871a4d8, 0xea1a27db, 0xf94ad42f, 0x0b21572c,
    0xdfeb33c7, 0x2d80b0c4, 0x3ed04330, 0xccbbc033,
    0xa24bb5a6, 0x502036a5, 0x4370c551, 0xb11b4652,
    0x65d122b9, 0x97baa1ba, 0x84ea524e, 0x7681d14d,
    0x2892ed69, 0xdaf96e6a, 0xc9a99d9e, 0x3bc21e9d,
    0xef087a76, 0x1d63f975, 0x0e330a81, 0xfc588982,
    0xb21572c9, 0x407ef1ca, 0x532e023e, 0xa145813d,
    0x758fe5d6, 0x87e466d5, 0x94b49521, 0x66df1622,
    0x38cc2a06, 0xcaa7a905, 0xd9f75af1, 0x2b9cd9f2,
    0xff56bd19, 0x0d3d3e1a, 0x1e6dcdee, 0xec064eed,
    0xc38d26c4, 0x31e6a5c7, 0x22b65633, 0xd0ddd530,
    0x0417b1db, 0xf67c32d8, 0xe52cc12c, 0x1747422f,
    0x49547e0b, 0xbb3ffd08, 0xa86f0efc, 0x5a048dff,
    0x8ecee914, 0x7ca56a17, 0x6ff599e3, 0x9d9e1ae0,
    0xd3d3e1ab, 0x21b862a8, 0x32e8915c, 0xc083125f,
    0x144976b4, 0xe622f5b7, 0xf5720643, 0x07198540,
    0x590ab964, 0xab613a67, 0xb831c993, 0x4a5a4a90,
    0x9e902e7b, 0x6cfbad78, 0x7fab5e8c, 0x8dc0dd8f,
    0xe330a81a, 0x115b2b19, 0x020bd8ed, 0xf0605bee,
    0x24aa3f05, 0xd6c1bc06, 0xc5914ff2, 0x37faccf1,
    0x69e9f0d5, 0x9b8273d6, 0x88d28022, 0x7ab90321,
    0xae7367ca, 0x5c18e4c9, 0x4f48173d, 0xbd23943e,
    0xf36e6f75, 0x0105ec76, 0x12551f82, 0xe03e9c81,
    0x34f4f86a, 0xc69f7b69, 0xd5cf889d, 0x27a40b9e,
    0x79b737ba, 0x8bdcb4b9, 0x988c474d, 0x6ae7c44e,
    0xbe2da0a5, 0x4c4623a6, 0x5f16d052, 0xad7d5351
  };
  
  uint32_t crc = init_crc ^ 0xffffffff;
  for (size_t i = 0; i < n; i++) {
    crc = table[(crc ^ data[i]) & 0xff] ^ (crc >> 8);
  }
  return crc ^ 0xffffffff;
  // FUNCTION_ID: leveldb_func023 - END
}

uint32_t crc32c_Value(const char* data, size_t n) {
  // FUNCTION_ID: leveldb_func024 - START
  return crc32c_Extend(0, data, n);
  // FUNCTION_ID: leveldb_func024 - END
}

void AppendInternalKey(std::string* result, const Slice& user_key, uint64_t sequence, uint8_t type) {
  // FUNCTION_ID: leveldb_func025 - START
  result->append(user_key.data(), user_key.size());
  uint64_t num = (sequence << 8) | type;
  PutFixed64(result, num);
  // FUNCTION_ID: leveldb_func025 - END
}

bool ParseInternalKey(const Slice& internal_key, Slice* user_key, uint64_t* sequence, uint8_t* type) {
  // FUNCTION_ID: leveldb_func026 - START
  const size_t n = internal_key.size();
  if (n < 8) return false;
  uint64_t num = DecodeFixed64(internal_key.data() + n - 8);
  uint8_t c = num & 0xff;
  *sequence = num >> 8;
  *type = c;
  *user_key = Slice(internal_key.data(), n - 8);
  return (c <= 1);
  // FUNCTION_ID: leveldb_func026 - END
}

size_t CalculateSharedPrefix(const Slice& a, const Slice& b) {
  // FUNCTION_ID: leveldb_func027 - START
  size_t min_length = (a.size() < b.size()) ? a.size() : b.size();
  size_t shared = 0;
  while (shared < min_length && a[shared] == b[shared]) {
    shared++;
  }
  return shared;
  // FUNCTION_ID: leveldb_func027 - END
}

uint32_t RandomSeed() {
  // FUNCTION_ID: leveldb_func028 - START
  return 0xdeadbeef;
  // FUNCTION_ID: leveldb_func028 - END
}

// ===== HARD FUNCTIONS =====

// Random class implementation
Random::Random(uint32_t s) : seed_(s & 0x7fffffffu) {
  // FUNCTION_ID: leveldb_func029 - START
  if (seed_ == 0 || seed_ == 2147483647L) {
    seed_ = 1;
  }
  // FUNCTION_ID: leveldb_func029 - END
}

uint32_t Random::Next() {
  // FUNCTION_ID: leveldb_func030 - START
  static const uint32_t M = 2147483647L;
  static const uint64_t A = 16807;
  uint64_t product = seed_ * A;
  seed_ = static_cast<uint32_t>((product >> 31) + (product & M));
  if (seed_ > M) {
    seed_ -= M;
  }
  return seed_;
  // FUNCTION_ID: leveldb_func030 - END
}

uint32_t Random::Uniform(int n) {
  // FUNCTION_ID: leveldb_func039 - START
  return Next() % n;
  // FUNCTION_ID: leveldb_func039 - END
}

bool Random::OneIn(int n) {
  // FUNCTION_ID: leveldb_func040 - START
  return (Next() % n) == 0;
  // FUNCTION_ID: leveldb_func040 - END
}

int SkipListRandomHeight(Random* rnd, int max_height) {
  // FUNCTION_ID: leveldb_func031 - START
  static const unsigned int kBranching = 4;
  int height = 1;
  while (height < max_height && rnd->OneIn(kBranching)) {
    height++;
  }
  return height;
  // FUNCTION_ID: leveldb_func031 - END
}

// Bloom filter operations
void BloomFilter_CreateFilter(const Slice* keys, int n, int bits_per_key, std::string* dst) {
  // FUNCTION_ID: func032 - START
  size_t bits = n * bits_per_key;
  if (bits < 64) bits = 64;
  size_t bytes = (bits + 7) / 8;
  bits = bytes * 8;
  
  size_t k = static_cast<size_t>(bits_per_key * 0.69);
  if (k < 1) k = 1;
  if (k > 30) k = 30;
  
  const size_t init_size = dst->size();
  dst->resize(init_size + bytes, 0);
  dst->push_back(static_cast<char>(k));
  char* array = &(*dst)[init_size];
  
  for (int i = 0; i < n; i++) {
    uint32_t h = BloomHash(keys[i]);
    const uint32_t delta = (h >> 17) | (h << 15);
    for (size_t j = 0; j < k; j++) {
      const uint32_t bitpos = h % bits;
      array[bitpos / 8] |= (1 << (bitpos % 8));
      h += delta;
    }
  }
  // FUNCTION_ID: func032 - END
}

bool BloomFilter_KeyMayMatch(const Slice& key, const Slice& bloom_filter) {
  // FUNCTION_ID: func033 - START
  const size_t len = bloom_filter.size();
  if (len < 2) return false;
  
  const char* array = bloom_filter.data();
  const size_t bits = (len - 1) * 8;
  const size_t k = array[len - 1];
  if (k > 30) return true;
  
  uint32_t h = BloomHash(key);
  const uint32_t delta = (h >> 17) | (h << 15);
  for (size_t j = 0; j < k; j++) {
    const uint32_t bitpos = h % bits;
    if ((array[bitpos / 8] & (1 << (bitpos % 8))) == 0) return false;
    h += delta;
  }
  return true;
  // FUNCTION_ID: func033 - END
}

uint32_t BloomFilter_DoubleHash(uint32_t h, int probe_num) {
  // FUNCTION_ID: func034 - START
  const uint32_t delta = (h >> 17) | (h << 15);
  return h + delta * probe_num;
  // FUNCTION_ID: func034 - END
}

// LRU cache operations (simplified standalone versions)
bool LRUCache_Lookup(const Slice& key, uint32_t hash, void** value) {
  // FUNCTION_ID: func035 - START
  return false;
  // FUNCTION_ID: func035 - END
}

void LRUCache_Evict(size_t target_usage) {
  // FUNCTION_ID: func036 - START
  // FUNCTION_ID: func036 - END
}

// Block builder operations
void BlockBuilder_Add(std::string* buffer, std::string* last_key, const Slice& key, const Slice& value, int restart_interval, int* counter, std::vector<uint32_t>* restarts) {
  // FUNCTION_ID: func037 - START
  Slice last_key_piece(*last_key);
  size_t shared = 0;
  
  if (*counter < restart_interval) {
    const size_t min_length = (last_key_piece.size() < key.size()) ? 
                               last_key_piece.size() : key.size();
    while ((shared < min_length) && (last_key_piece[shared] == key[shared])) {
      shared++;
    }
  } else {
    restarts->push_back(buffer->size());
    *counter = 0;
  }
  
  const size_t non_shared = key.size() - shared;
  
  PutVarint32(buffer, shared);
  PutVarint32(buffer, non_shared);
  PutVarint32(buffer, value.size());
  
  buffer->append(key.data() + shared, non_shared);
  buffer->append(value.data(), value.size());
  
  last_key->resize(shared);
  last_key->append(key.data() + shared, non_shared);
  (*counter)++;
  // FUNCTION_ID: func037 - END
}

Slice BlockBuilder_Finish(std::string* buffer, const std::vector<uint32_t>& restarts) {
  // FUNCTION_ID: func038 - START
  for (size_t i = 0; i < restarts.size(); i++) {
    PutFixed32(buffer, restarts[i]);
  }
  PutFixed32(buffer, restarts.size());
  return Slice(*buffer);
  // FUNCTION_ID: func038 - END
}

}  // namespace leveldb

