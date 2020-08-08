/*
 * Utilities for defining and retreiving per-instance data.
 */

const int maxInstances = 2048;

#define INSTANCE_INPUT_BINDING(SET, DATA)                                                          \
  layout(std140, binding = 0, SET) readonly uniform UniformData { DATA[maxInstances] insts; }      \
  uniformInstances

#define GET_INST() uniformInstances.insts[gl_InstanceIndex]
