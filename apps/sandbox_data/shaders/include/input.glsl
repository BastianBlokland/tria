#ifndef INCLUDE_INPUT
#define INCLUDE_INPUT

const uint globalSet   = 0; // 'Global' resources, like a projection matrix.
const uint graphicSet  = 1; // 'Per graphic' resources, like the mesh and textures.
const uint instanceSet = 2; // 'Per instance' resources, like a transformation matrix.

/*
 * Utilities for defining and retreiving global data.
 */

#define GLOBAL_INPUT_BINDING(DATA)                                                                 \
  layout(set = globalSet, binding = 0, std140) readonly uniform GlobalBuffer { DATA globalData; }

#define GET_GLOBAL() globalData

/*
 * Utilities for defining and retreiving mesh data.
 */

struct MeshMeta {
  vec4 posBoundsMin;  // xyz: min position of aabb, w: unused.
  vec4 posBoundsSize; // xyz: size of the of the aabb, w: unused.
};

struct VertexData {
  f16vec4 posFracAndU;
  f16vec4 nrmAndV;
  f16vec4 tan;
};

#define VERTEX_INPUT_BINDING()                                                                     \
  layout(set = graphicSet, binding = 0, std140) readonly buffer VertexBuffer {                     \
    MeshMeta meshMeta;                                                                             \
    VertexData[] vertices;                                                                         \
  }

#define GET_VERT() vertices[gl_VertexIndex]

#define GET_VERT_POS(VERT)                                                                         \
  vec3(                                                                                            \
      meshMeta.posBoundsMin.x + meshMeta.posBoundsSize.x * (VERT).posFracAndU.x,                   \
      meshMeta.posBoundsMin.y + meshMeta.posBoundsSize.y * (VERT).posFracAndU.y,                   \
      meshMeta.posBoundsMin.z + meshMeta.posBoundsSize.z * (VERT).posFracAndU.z)

#define GET_VERT_NRM(VERT) (VERT).nrmAndV.xyz

#define GET_VERT_TAN(VERT) (VERT).tan

#define GET_VERT_TEXCOORD(VERT) vec2((VERT).posFracAndU.w, (VERT).nrmAndV.w)

/*
 * Utilities for defining and retreiving per-instance data.
 */

const uint maxInstances = 2048;

#define INSTANCE_INPUT_BINDING(DATA)                                                               \
  layout(set = instanceSet, binding = 0, std140) readonly uniform InstanceBuffer {                 \
    DATA[maxInstances] instances;                                                                  \
  }

#define GET_INST() instances[gl_InstanceIndex]

#endif
