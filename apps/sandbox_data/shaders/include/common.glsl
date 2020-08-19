const uint graphicSet  = 0; // 'Per graphic' resources, like the mesh and textures.
const uint instanceSet = 1; // 'Per instance' resources, like a transformation matrix.

/*
 * Utilities for defining and retreiving vertex data.
 */

struct VertexData {
  vec4 posAndU;
  vec4 nrmAndV;
};

#define VERTEX_INPUT_BINDING()                                                                     \
  layout(set = graphicSet, binding = 0) readonly buffer VertexBuffer { VertexData[] vertices; }

#define GET_VERT() vertices[gl_VertexIndex]

#define GET_VERT_POS() GET_VERT().posAndU.xyz

#define GET_VERT_NRM() GET_VERT().nrmAndV.xyz

#define GET_VERT_TEXCOORD() vec2(GET_VERT().posAndU.w, GET_VERT().nrmAndV.w)

/*
 * Utilities for defining and retreiving per-instance data.
 */

const uint maxInstances = 2048;

#define INSTANCE_INPUT_BINDING(DATA)                                                               \
  layout(set = instanceSet, binding = 0) readonly uniform InstanceBuffer {                         \
    DATA[maxInstances] instances;                                                                  \
  }

#define GET_INST() instances[gl_InstanceIndex]
