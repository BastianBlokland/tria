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
 * Utilities for defining and retreiving vertex data.
 */

struct VertexData {
  vec4 posAndU;
  vec4 nrmAndV;
  vec4 tan;
};

#define VERTEX_INPUT_BINDING()                                                                     \
  layout(set = graphicSet, binding = 0, std140) readonly buffer VertexBuffer {                     \
    VertexData[] vertices;                                                                         \
  }

#define GET_VERT() vertices[gl_VertexIndex]

#define GET_VERT_POS() GET_VERT().posAndU.xyz

#define GET_VERT_NRM() GET_VERT().nrmAndV.xyz

#define GET_VERT_TAN() GET_VERT().tan

#define GET_VERT_TEXCOORD() vec2(GET_VERT().posAndU.w, GET_VERT().nrmAndV.w)

/*
 * Utilities for defining and retreiving per-instance data.
 */

const uint maxInstances = 2048;

#define INSTANCE_INPUT_BINDING(DATA)                                                               \
  layout(set = instanceSet, binding = 0, std140) readonly uniform InstanceBuffer {                 \
    DATA[maxInstances] instances;                                                                  \
  }

#define GET_INST() instances[gl_InstanceIndex]
