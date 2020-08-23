#version 450
#extension GL_GOOGLE_include_directive : enable
#include "include/input.glsl"

const uint highlightInterval = 5;
const vec4 normalColor       = vec4(.1, .1, .1, .4);
const vec4 highlightColor    = vec4(.1, .1, .1, .9);

struct GlobalData {
  mat4 viewProjMat;
};
GLOBAL_INPUT_BINDING(GlobalData);

struct InstanceData {
  vec3 cameraPos;
  int segments;
};
INSTANCE_INPUT_BINDING(InstanceData);

layout(location = 0) out vec4 outLineColor;

void main() {
  // TODO(bastian): Currently the grid is centered on the camera pos, but checking the frustum
  // bounds would be more effiencient.
  int centerX = int(GET_INST().cameraPos.x);
  int centerZ = int(GET_INST().cameraPos.z);

  int numSegs       = GET_INST().segments;
  int halfNumSegs   = numSegs >> 1;
  int vertIndex     = gl_VertexIndex;
  int halfVertIndex = gl_VertexIndex >> 1;

  // First half of the vertices we draw horizontal lines and the other half vertical lines.
  bool isHor = vertIndex < numSegs * 2;

  // From -halfNumSegs to +halfNumSegs increasing by one every 2 vertices.
  int a = (halfVertIndex % numSegs) - halfNumSegs;

  // Every vertex ping-pong between -halfNumSegs and + halfNumSegs.
  int b = (gl_VertexIndex & 1) * numSegs - halfNumSegs;

  bool isHighlight = (abs((isHor ? centerZ : centerX) + a) % highlightInterval) == 0;
  outLineColor     = isHighlight ? highlightColor : normalColor;

  float x     = centerX + (isHor ? b : a);
  float z     = centerZ + (isHor ? a : b);
  gl_Position = GET_GLOBAL().viewProjMat * vec4(x, isHighlight ? .01 : 0, z, 1);
}
