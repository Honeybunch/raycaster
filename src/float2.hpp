#pragma once

namespace raycaster {

struct float2 {
  float x;
  float y;
};

float2 float2_add(float2 lhs, float2 rhs);
float2 float2_sub(float2 lhs, float2 rhs);

float float2_dot(float2 lhs, float2 rhs);

float2 float2_add(float2 lhs, float rhs);
float2 float2_sub(float2 lhs, float rhs);
float2 float2_mul(float2 lhs, float rhs);
float2 float2_div(float2 lhs, float rhs);

} // namespace raycaster