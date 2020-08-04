#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform sampler2D texSampler;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

const float discardAlpha = .1;

void main() {
  vec4 colSample = texture(texSampler, inTexcoord);
  if (colSample.a < discardAlpha) {
    discard;
  }
  outColor = colSample;
}
