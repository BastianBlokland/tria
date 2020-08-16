/*
 * Utilities for defining and retreiving vertex data.
 */

struct VertexData {
  vec4 posAndU;
  vec4 nrmAndV;
};

#define VERTEX_INPUT_BINDING(SET)                                                                  \
  layout(binding = 0, SET) readonly buffer VertexBuffer { VertexData[] vertices; }

#define GET_VERT() vertices[gl_VertexIndex]

#define GET_VERT_POS() GET_VERT().posAndU.xyz

#define GET_VERT_NRM() GET_VERT().nrmAndV.xyz

#define GET_VERT_TEXCOORD() vec2(GET_VERT().posAndU.w, GET_VERT().nrmAndV.w)

/*
 * Utilities for defining and retreiving per-instance data.
 */

const int maxInstances = 2048;

#define INSTANCE_INPUT_BINDING(SET, DATA)                                                          \
  layout(binding = 0, SET) readonly uniform InstanceBuffer { DATA[maxInstances] instances; }

#define GET_INST() instances[gl_InstanceIndex]
