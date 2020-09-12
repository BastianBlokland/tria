#version 450
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
#include "include/input.glsl"

struct InstanceData {
  vec4[1024] points;
};
INSTANCE_INPUT_BINDING(InstanceData);

void main() {
  InstanceData inst = GET_INST();

  vec2 point  = inst.points[gl_VertexIndex].xy;
  point.y     = 1 - point.y;
  gl_Position = vec4(point * 2.0 - 1.0, 0.0, 1.0);
}
