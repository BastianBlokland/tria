#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#include "include/common.glsl"

layout(set = graphicSet, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 inNrm;
layout(location = 1) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

void main() { outColor = texture(texSampler, inTexcoord); }
