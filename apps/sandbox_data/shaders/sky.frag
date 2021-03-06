#version 450
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
#include "include/input.glsl"

/* 3 color gradient based on the y component of the world direction. */
const vec4 topColor    = vec4(0.4, 0.5, 0.8, 1);
const vec4 middleColor = vec4(0.7, 0.8, 0.9, 1);
const vec4 bottomColor = vec4(0.2, 0.2, 0.2, 1);

layout(location = 0) in vec3 inWorldDir;

layout(location = 0) out vec4 outColor;

void main() {
  float dirY        = normalize(inWorldDir).y; // -1 to 1.
  float topBlend    = 1.0 - pow(min(1.0, 1.0 - dirY /* 2 to 0 */), 4.0);
  float bottomBlend = 1.0 - pow(min(1.0, 1.0 + dirY /* 0 to 2 */), 30.0);
  float middleBlend = 1.0 - topBlend - bottomBlend; // remaining 'blend'.

  outColor = topColor * topBlend + middleColor * middleBlend + bottomColor * bottomBlend;
}
