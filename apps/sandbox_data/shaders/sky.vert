#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#include "include/instance.glsl"

layout(location = 0, set = 0) in vec3 inVertPos;
layout(location = 1, set = 0) in vec3 inVertNrm;
layout(location = 2, set = 0) in vec4 inVertColor;
layout(location = 3, set = 0) in vec2 inVertTexcoord;

struct InstanceData {
  mat4 viewProjMat;
};
INSTANCE_INPUT_BINDING(set = 0, InstanceData);

layout(location = 0) out vec3 outWorldDir;

void main() {
  // At maximum depth filling the clispace.
  gl_Position = vec4(inVertPos.xy * 2.0, 1.0, 1.0);

  // Use the inverse of the viewProj to create a matrix that goes from clipspace to worldspace.
  outWorldDir = (inverse(GET_INST().viewProjMat) * gl_Position).xyz;
}
