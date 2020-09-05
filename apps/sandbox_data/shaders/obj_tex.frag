#version 450
#extension GL_GOOGLE_include_directive : enable
#include "include/input.glsl"
#include "include/utils.glsl"

layout(set = graphicSet, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 inWorldNrm;
layout(location = 1) in vec4 inWorldTan;
layout(location = 2) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

void main() { outColor = textureSRGB(texSampler, inTexcoord); }
