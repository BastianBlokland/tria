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

#endif
