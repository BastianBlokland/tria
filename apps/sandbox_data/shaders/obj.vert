#version 450
#extension GL_GOOGLE_include_directive : enable
#include "include/input.glsl"

struct GlobalData {
  mat4 viewProjMat;
};
GLOBAL_INPUT_BINDING(GlobalData);

VERTEX_INPUT_BINDING();

struct InstanceData {
  mat4 mat;
};
INSTANCE_INPUT_BINDING(InstanceData);

layout(location = 0) out vec3 outNrm;
layout(location = 1) out vec2 outTexcoord;

void main() {
  gl_Position = GET_GLOBAL().viewProjMat * GET_INST().mat * vec4(GET_VERT_POS(), 1.0);
  outNrm      = GET_VERT_NRM();
  outTexcoord = GET_VERT_TEXCOORD();
}
