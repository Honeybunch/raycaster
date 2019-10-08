#include "float2.hpp"

namespace raycaster {

float2 float2_add(float2 lhs, float2 rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y};
}
float2 float2_sub(float2 lhs, float2 rhs) {
  return {lhs.x - rhs.x, lhs.y - rhs.y};
}

float float2_dot(float2 lhs, float2 rhs) {
  return (lhs.x * rhs.x) + (lhs.y + rhs.y);
}

float2 float2_add(float2 lhs, float rhs) { return {lhs.x + rhs, lhs.y + rhs}; }
float2 float2_sub(float2 lhs, float rhs) { return {lhs.x - rhs, lhs.y - rhs}; }
float2 float2_mul(float2 lhs, float rhs) { return {lhs.x * rhs, lhs.y * rhs}; }
float2 float2_div(float2 lhs, float rhs) { return {lhs.x / rhs, lhs.y / rhs}; }

} // namespace raycaster