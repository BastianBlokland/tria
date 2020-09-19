#version 450
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
#include "include/input.glsl"

layout(location = 0) out vec4 outColor;

void main() { outColor = vec4(0.2, 0.2, 0.2, 1); }
