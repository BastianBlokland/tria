#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#include "include/instance.glsl"

layout(location = 0, set = 0) in vec3 inVertPos;
layout(location = 1, set = 0) in vec3 inVertNrm;
layout(location = 2, set = 0) in vec4 inVertColor;
layout(location = 3, set = 0) in vec2 inVertTexcoord;

struct InstanceData {
  mat4 mat;
};
INSTANCE_INPUT_BINDING(set = 1, InstanceData);

layout(location = 0) out vec3 outNrm;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec2 outTexcoord;

void main() {
  gl_Position = GET_INST().mat * vec4(inVertPos, 1.0);
  outNrm      = inVertNrm;
  outColor    = inVertColor;
  outTexcoord = inVertTexcoord;
}
