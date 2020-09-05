#version 450
#extension GL_GOOGLE_include_directive : enable
#include "include/input.glsl"

layout(location = 0) in flat vec4 inLineColor;

layout(location = 0) out vec4 outColor;

void main() {
  // Fade the alpha out based on the depth-buffer value.
  // Note: we are using a reversed-z depth buffer and infinite far plane, so 1 is on the near plane
  // and 0 is infinitly far away.
  float fade = min((gl_FragCoord.z - .0006) * 100., 1.);
  outColor   = vec4(inLineColor.rgb, inLineColor.a * fade);
}
