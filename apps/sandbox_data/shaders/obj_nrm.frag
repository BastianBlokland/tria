#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inNrm;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

void main() { outColor = vec4((inNrm + 1.0) * .5, 1.0); }
