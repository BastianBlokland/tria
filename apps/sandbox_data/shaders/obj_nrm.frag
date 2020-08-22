#version 450
#extension GL_GOOGLE_include_directive : enable
#include "include/input.glsl"

layout(location = 0) in vec3 inNrm;
layout(location = 1) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

void main() { outColor = vec4((inNrm + 1.0) * .5, 1.0); }
