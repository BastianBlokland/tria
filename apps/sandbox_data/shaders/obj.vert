#version 450
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
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

layout(location = 0) out vec3 outWorldNrm;
layout(location = 1) out vec4 outWorldTan;
layout(location = 2) out vec2 outTexcoord;

void main() {
  VertexData vert   = GET_VERT();
  InstanceData inst = GET_INST();
  GlobalData global = GET_GLOBAL();

  gl_Position = global.viewProjMat * inst.mat * vec4(GET_VERT_POS(vert), 1.0);
  outWorldNrm = mat3(inst.mat) * GET_VERT_NRM(vert);
  outWorldTan = vec4(mat3(inst.mat) * GET_VERT_TAN(vert).xyz, GET_VERT_TAN(vert).w);
  outTexcoord = GET_VERT_TEXCOORD(vert);
}
