#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 inNrm;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

void main() { outColor = texture(texSampler, inTexcoord); }