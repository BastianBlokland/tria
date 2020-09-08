#version 450
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
#include "include/input.glsl"

layout(location = 0) in vec3 inWorldNrm;
layout(location = 1) in vec4 inWorldTan;
layout(location = 2) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

void main() { outColor = vec4((normalize(inWorldNrm) + 1.0) * .5, 1.0); }
