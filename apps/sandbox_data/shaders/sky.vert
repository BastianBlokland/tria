#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#include "include/common.glsl"

VERTEX_INPUT_BINDING(set = 0);

struct InstanceData {
  mat4 viewProjMat;
};
INSTANCE_INPUT_BINDING(set = 1, InstanceData);

layout(location = 0) out vec3 outWorldDir;

void main() {
  // At maximum depth filling the clispace.
  gl_Position = vec4(GET_VERT_POS().xy * 2.0, 1.0, 1.0);

  // Use the inverse of the viewProj to create a matrix that goes from clipspace to worldspace.
  outWorldDir = (inverse(GET_INST().viewProjMat) * gl_Position).xyz;
}
