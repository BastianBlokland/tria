#version 450
#extension GL_GOOGLE_include_directive : enable
#include "include/input.glsl"

VERTEX_INPUT_BINDING();

struct GlobalData {
  mat4 viewProjMat;
};
GLOBAL_INPUT_BINDING(GlobalData);

layout(location = 0) out vec3 outWorldDir;

void main() {
  // Fullscreen at maximum depth (depth 0 due to reversed-z depthbuffer).
  gl_Position = vec4(GET_VERT_POS().xy * 2.0, 0.0, 1.0);

  // Use the inverse of the viewProj to create a matrix that goes from clipspace to worldspace.
  outWorldDir = (inverse(GET_GLOBAL().viewProjMat) * gl_Position).xyz;
}
