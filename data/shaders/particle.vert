#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0, set = 0) in vec3 inVertPos;
layout(location = 1, set = 0) in vec4 inVertColor;
layout(location = 2, set = 0) in vec2 inVertTexcoord;

struct InstanceData {
  vec2 pos;
  vec2 velocity;
  vec2 size;
  vec2 screenSize;
  vec4 color;
  float lifetime;
};

layout(std140, binding = 0, set = 1) uniform UniformData { InstanceData[2048] instances; };

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outTexcoord;

vec4 toNdc(vec2 vertPos, vec2 pos, vec2 size, vec2 screenSize) {
  const vec2 screenPos = vertPos * size + pos;
  return vec4(screenPos / screenSize * 2.0 - 1.0, 0.0, 1.0);
}

void main() {
  InstanceData data = instances[gl_InstanceIndex];
  gl_Position       = toNdc(inVertPos.xy, data.pos, data.size, data.screenSize);
  outColor          = data.color;
  outTexcoord       = inVertTexcoord;
}
