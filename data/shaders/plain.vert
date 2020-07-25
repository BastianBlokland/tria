#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform UniformData { vec3 inOffset; };

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main() {
  gl_Position = vec4(inPosition + inOffset, 1.0);
  outColor    = inColor;
}
