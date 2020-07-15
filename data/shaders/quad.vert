#version 450
#extension GL_ARB_separate_shader_objects : enable

vec2 centerPos = vec2(0.4, 0);
float scale    = 0.5;

vec2 positions[6] = vec2[](
    centerPos + vec2(-0.5, -0.5) * scale,
    centerPos + vec2(0.5, 0.5) * scale,
    centerPos + vec2(-0.5, 0.5) * scale,

    centerPos + vec2(-0.5, -0.5) * scale,
    centerPos + vec2(0.5, -0.5) * scale,
    centerPos + vec2(0.5, 0.5) * scale);

vec3 colors[6] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),

    vec3(1.0, 0.0, 0.0),
    vec3(1.0, 1.0, 0.0),
    vec3(0.0, 1.0, 0.0));

layout(location = 0) out vec3 fragColor;

void main() {
  gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
  fragColor   = colors[gl_VertexIndex];
}
