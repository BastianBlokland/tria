#ifndef INCLUDE_UTILS
#define INCLUDE_UTILS

/*
 * Common utilities.
 */

/* Decode a srgb encoded color to a linear color.
 * Note: Is a fast approximation, more info: https://en.wikipedia.org/wiki/SRGB
 */
vec3 approxDecodeSRGB(vec3 color) {
  const float inverseGamma = 2.2;
  return pow(color, vec3(inverseGamma));
}

/* Encode a linear color to srgb.
 * Note: Is a fast approximation, more info: https://en.wikipedia.org/wiki/SRGB
 */
vec3 approxEncodeSRGB(vec3 color) {
  const float gamma = 1.0 / 2.2;
  return pow(color, vec3(gamma));
}

/* Sample a srgb encoded texture.
 */
vec4 textureSRGB(sampler2D texSampler, vec2 texcoord) {
  const vec4 raw = texture(texSampler, texcoord);
  return vec4(approxDecodeSRGB(raw.rgb), raw.a);
}

/* Sample a normal texture in tangent space and convert it to the axis system formed by the given
 * normal and tangent, 'w' component of the tangent indicates the handedness.
 */
vec3 textureNrm(sampler2D nrmSampler, vec2 texcoord, vec3 normal, vec4 tangent) {
  const vec3 t   = normalize(tangent.xyz);             // tangent.
  const vec3 n   = normalize(normal);                  // normal.
  const vec3 b   = normalize(cross(n, t) * tangent.w); // binormal.
  const mat3 tbn = mat3(t, b, n);
  return tbn * (texture(nrmSampler, texcoord).xyz * 2.0 - 1.0);
}

#endif
