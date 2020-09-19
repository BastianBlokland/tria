#version 450
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
#include "include/input.glsl"

VERTEX_INPUT_BINDING();

struct InstanceData {
  vec4 point;
};
INSTANCE_INPUT_BINDING(InstanceData);

layout(location = 0) out vec2 outTexcoord;

void main() {
  VertexData vert   = GET_VERT();
  InstanceData inst = GET_INST();

  vec2 center = inst.point.xy;
  center.y *= -1.0;

  float scale = .05;

  gl_Position = vec4(center + GET_VERT_POS(vert).xy * scale, 0.0, 1.0);
  outTexcoord = GET_VERT_TEXCOORD(vert);
}
