#pragma once
#include "geometry.h"

namespace TinyRenderer {

// ============================================================================
// GEOMETRY FUNCTIONS (Easy - Medium)
// ============================================================================

// Function 1: Vector dot product (EASY)
double vec3_dot(const vec3& lhs, const vec3& rhs);

// Function 2: Vector addition (EASY)
vec3 vec3_add(const vec3& lhs, const vec3& rhs);

// Function 3: Vector subtraction (EASY)
vec3 vec3_subtract(const vec3& lhs, const vec3& rhs);

// Function 4: Vector scalar multiplication (EASY)
vec3 vec3_scale(const vec3& v, const double scalar);

// Function 5: Vector norm/magnitude (EASY)
double vec3_norm(const vec3& v);

// Function 6: Vector normalization (MEDIUM)
vec3 vec3_normalize(const vec3& v);

// Function 7: Cross product (EASY)
vec3 vec3_cross(const vec3& v1, const vec3& v2);

// Function 8: Matrix transpose 3x3 (MEDIUM)
mat3 mat3_transpose(const mat3& m);

// Function 9: Matrix multiplication 3x3 (MEDIUM)
mat3 mat3_multiply(const mat3& lhs, const mat3& rhs);

// Function 10: Matrix-vector multiplication 3x3 (MEDIUM)
vec3 mat3_vec3_multiply(const mat3& m, const vec3& v);

// ============================================================================
// ADVANCED GEOMETRY FUNCTIONS (Medium - Hard)
// ============================================================================

// Function 11: Matrix determinant 3x3 (HARD)
double mat3_determinant(const mat3& m);

// Function 12: Matrix cofactor 3x3 (HARD)
double mat3_cofactor(const mat3& m, const int row, const int col);

// Function 13: Matrix inversion 3x3 (HARD)
mat3 mat3_invert(const mat3& m);

// Function 14: Matrix transpose 4x4 (MEDIUM)
mat4 mat4_transpose(const mat4& m);

// Function 15: Matrix multiplication 4x4 (MEDIUM)
mat4 mat4_multiply(const mat4& lhs, const mat4& rhs);

// Function 16: Matrix-vector multiplication 4x4 (MEDIUM)
vec4 mat4_vec4_multiply(const mat4& m, const vec4& v);

// ============================================================================
// CAMERA & TRANSFORMATION FUNCTIONS (Hard)
// ============================================================================

// Function 17: LookAt view matrix (HARD)
mat4 lookat(const vec3& eye, const vec3& center, const vec3& up);

// Function 18: Perspective projection matrix (MEDIUM)
mat4 perspective(const double fov);

// Function 19: Viewport transformation matrix (MEDIUM)
mat4 viewport(const int x, const int y, const int w, const int h);

// ============================================================================
// RASTERIZATION FUNCTIONS (Medium - Hard)
// ============================================================================

// Function 20: Barycentric coordinates (MEDIUM)
vec3 barycentric(const vec2& p, const vec2& a, const vec2& b, const vec2& c);

// Function 21: Backface culling test (MEDIUM)
bool is_backface(const vec2& a, const vec2& b, const vec2& c);

} // namespace TinyRenderer
