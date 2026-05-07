// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "util/coding.h"

namespace leveldb {

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

char* EncodeVarint32(char* dst, uint32_t v) {
  // FUNCTION_ID: leveldb_func009 - START
  // Operate on characters as unsigneds
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

void PutVarint32(std::string* dst, uint32_t v) {
  // FUNCTION_ID: leveldb_func017 - START
  char buf[5];
  char* ptr = EncodeVarint32(buf, v);
  dst->append(buf, ptr - buf);
// FUNCTION_ID: leveldb_func017 - END
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

const char* GetVarint32PtrFallback(const char* p, const char* limit,
                                   uint32_t* value) {
  // FUNCTION_ID: leveldb_func011 - START
  uint32_t result = 0;
  for (uint32_t shift = 0; shift <= 28 && p < limit; shift += 7) {
    uint32_t byte = *(reinterpret_cast<const uint8_t*>(p));
    p++;
    if (byte & 128) {
      // More bytes are present
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

const char* GetVarint64Ptr(const char* p, const char* limit, uint64_t* value) {
  // FUNCTION_ID: leveldb_func012 - START
  uint64_t result = 0;
  for (uint32_t shift = 0; shift <= 63 && p < limit; shift += 7) {
    uint64_t byte = *(reinterpret_cast<const uint8_t*>(p));
    p++;
    if (byte & 128) {
      // More bytes are present
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

}  // namespace leveldb
