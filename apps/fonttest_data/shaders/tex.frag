#version 450
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
#include "include/input.glsl"
#include "include/utils.glsl"

layout(set = graphicSet, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

void main() { outColor = textureSRGB(texSampler, inTexcoord); }
