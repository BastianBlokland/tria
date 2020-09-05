#version 450
#extension GL_GOOGLE_include_directive : enable
#include "include/input.glsl"
#include "include/utils.glsl"

layout(set = graphicSet, binding = 1) uniform sampler2D texSampler;
layout(set = graphicSet, binding = 2) uniform sampler2D nrmSampler;

layout(location = 0) in vec3 inWorldNrm;
layout(location = 1) in vec4 inWorldTan;
layout(location = 2) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

void main() {
  vec3 normal          = textureNrm(nrmSampler, inTexcoord, inWorldNrm, inWorldTan);
  vec3 lightDir        = normalize(vec3(0.2, 1.0, -0.5));
  float lightIntensity = clamp(dot(normal, lightDir), 0.1, 1.0);

  vec4 diffuse = textureSRGB(texSampler, inTexcoord);
  outColor     = diffuse * lightIntensity;
}
