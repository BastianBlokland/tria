#include "loader.hpp"
#include "tria/asset/err/shader_spv_err.hpp"
#include "tria/asset/shader.hpp"
#include <array>
#include <utility>

// The spriv.h header can unfortunately exist in a few places depending on the vulkan sdk version.
#if __has_include(<spirv-headers/spirv.h>)
#include <spirv-headers/spirv.h>
#elif __has_include(<spirv/1.0/spirv.h>)
#include <spirv/1.0/spirv.h>
#elif __has_include(<vulkan/spirv.h>)
#include <vulkan/spirv.h>
#else
static_assert(false, "No Vulkan SpirV header found")
#endif

namespace tria::asset::internal {

/*
 * Spir-V (Standard Portable Intermediate Representation 5)
 * Spec: https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html
 */

namespace {

class Reader final {
public:
  Reader(const uint32_t* current, const uint32_t* end) : m_cur{current}, m_end{end} {}

  /* Get the current read head.
   */
  [[nodiscard]] auto getCur() noexcept { return m_cur; }

  /* Check how many words are remaining.
   */
  [[nodiscard]] auto getRemainingCount() noexcept { return static_cast<size_t>(m_end - m_cur); }

  [[nodiscard]] auto consume() noexcept {
    assert(getRemainingCount() > 0);
    return *m_cur++;
  }

  auto skip(size_t words) noexcept {
    assert(getRemainingCount() >= words);
    m_cur += words;
  }

  auto assertRemainingSize(size_t size) {
    if (getRemainingCount() < size) {
      throw err::ShaderSpvErr{"Unexpected end of file"};
    }
  }

private:
  const uint32_t* m_cur;
  const uint32_t* m_end;
};

[[nodiscard]] auto decodeVersion(uint32_t raw) noexcept {
  return std::make_pair(static_cast<uint8_t>(raw >> 16U), static_cast<uint8_t>(raw >> 8U));
}

[[nodiscard]] auto decodeInstructionHeader(uint32_t raw) noexcept {
  return std::make_pair(static_cast<uint16_t>(raw), static_cast<uint16_t>(raw >> 16U));
}

enum class SpvIdKind {
  Unknown,
  Variable,
  TypePointer,
  TypeStruct,
  TypeSampledImage,
};

struct SpvId {
  SpvIdKind kind               = SpvIdKind::Unknown;
  uint32_t set                 = UINT32_MAX;
  uint32_t binding             = UINT32_MAX;
  uint32_t typeId              = UINT32_MAX;
  SpvStorageClass storageClass = SpvStorageClassMax;
  std::string name; // Note: Does not exist in optimized spir-v files.
};

struct SpvProgram {
  SpvExecutionModel execModel = SpvExecutionModelMax;
  std::string entryPointName;
  std::vector<SpvId> ids;
};

auto readStringLiteral(const uint32_t* data, size_t maxWordSize) {
  std::string result;
  const auto* strData    = reinterpret_cast<const uint8_t*>(data);
  const auto* strDataEnd = strData + maxWordSize * 4U;
  for (; strData != strDataEnd; ++strData) {
    if (*strData == '\0') {
      const auto wordSize = (result.size() + 1U + 4U) / 4U; // +1 for null-term and +3 to round up.
      return std::make_pair(result, wordSize);
    }
    result += *strData;
  }
  throw err::ShaderSpvErr{"Unterminated string literal"};
}

auto readProgram(Reader& reader, uint32_t maxId) -> SpvProgram {
  auto program = SpvProgram{};
  program.ids  = std::vector<SpvId>(maxId);
  while (reader.getRemainingCount() > 0) {
    const auto* instrBase       = reader.getCur();
    const auto [opCode, opSize] = decodeInstructionHeader(instrBase[0]);
    if (opCode == 0) {
      throw err::ShaderSpvErr{"Unexpected end of file"};
    }
    reader.assertRemainingSize(opSize);

    switch (opCode) {

    case SpvOpEntryPoint: {
      /* Entry point definiton, we gather the execution model (stage) and the entryPointName here.
       * https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#OpEntryPoint
       */
      if (!program.entryPointName.empty()) {
        throw err::ShaderSpvErr{"Multiple entrypoints are not supported"};
      }
      reader.assertRemainingSize(4);
      program.execModel = static_cast<SpvExecutionModel>(instrBase[1]);
      // const auto entryPointId = instrBase[2];
      program.entryPointName =
          readStringLiteral(instrBase + 3U, static_cast<size_t>(opSize - 3U)).first;
    } break;
    case SpvOpDecorate: {
      /* Id decoration, we can gather which descriptor set and binding an id belongs to.
       * https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#OpDecorate
       * https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#Decoration
       */
      reader.assertRemainingSize(4); // 3 for OpDecorate, and 1 for set / binding def.
      const auto targetId = instrBase[1];
      switch (instrBase[2]) {
      case SpvDecorationDescriptorSet:
        if (targetId >= maxId) {
          throw err::ShaderSpvErr{"SpirV id out of bounds"};
        }
        program.ids[targetId].set = instrBase[3];
        break;
      case SpvDecorationBinding:
        if (targetId >= maxId) {
          throw err::ShaderSpvErr{"SpirV id out of bounds"};
        }
        program.ids[targetId].binding = instrBase[3];
        break;
      }
    } break;

    case SpvOpVariable: {
      /* Variable declaration, gather the type and the storage class of the variable.
       * https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#OpVariable
       */
      reader.assertRemainingSize(4);
      const auto typeId = instrBase[1];
      const auto id     = instrBase[2];
      if (typeId >= maxId || id >= maxId) {
        throw err::ShaderSpvErr{"SpirV id out of bounds"};
      }
      if (program.ids[id].kind != SpvIdKind::Unknown) {
        throw err::ShaderSpvErr{"SpirV id already declared"};
      }
      program.ids[id].kind         = SpvIdKind::Variable;
      program.ids[id].typeId       = typeId;
      program.ids[id].storageClass = static_cast<SpvStorageClass>(instrBase[3]);
    } break;

    case SpvOpTypePointer: {
      /* Pointer type declaration.
       * https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#OpTypePointer
       */
      reader.assertRemainingSize(4);
      const auto id     = instrBase[1];
      const auto typeId = instrBase[3];
      if (id >= maxId || typeId >= maxId) {
        throw err::ShaderSpvErr{"SpirV id out of bounds"};
      }
      if (program.ids[id].kind != SpvIdKind::Unknown) {
        throw err::ShaderSpvErr{"SpirV id already declared"};
      }
      program.ids[id].kind         = SpvIdKind::TypePointer;
      program.ids[id].storageClass = static_cast<SpvStorageClass>(instrBase[2]);
      program.ids[id].typeId       = typeId;
    } break;

    case SpvOpTypeStruct: {
      /* Struct declaration.
       * https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#OpTypeStruct
       */
      reader.assertRemainingSize(2);
      const auto id = instrBase[1];
      if (id >= maxId) {
        throw err::ShaderSpvErr{"SpirV id out of bounds"};
      }
      if (program.ids[id].kind != SpvIdKind::Unknown) {
        throw err::ShaderSpvErr{"SpirV id already declared"};
      }
      program.ids[id].kind = SpvIdKind::TypeStruct;
    } break;

    case SpvOpTypeSampledImage: {
      /* Sampled image declaration.
       * https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#OpTypeSampledImage
       */
      reader.assertRemainingSize(3);
      const auto id = instrBase[1];
      if (id >= maxId) {
        throw err::ShaderSpvErr{"SpirV id out of bounds"};
      }
      if (program.ids[id].kind != SpvIdKind::Unknown) {
        throw err::ShaderSpvErr{"SpirV id already declared"};
      }
      program.ids[id].kind = SpvIdKind::TypeSampledImage;
    } break;

    case SpvOpName: {
      /* (Debug) Name, usefull for debugging.
       * Note: Not present in optimized SpirV files.
       * https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#OpName
       */
      reader.assertRemainingSize(3);
      const auto id = instrBase[1];
      if (id >= maxId) {
        throw err::ShaderSpvErr{"SpirV id out of bounds"};
      }
      program.ids[id].name =
          readStringLiteral(instrBase + 2U, static_cast<size_t>(opSize - 2U)).first;
    }
    }
    reader.skip(opSize);
  }
  return program;
}

[[nodiscard]] auto getShaderKind(SpvExecutionModel execModel) {
  switch (execModel) {
  case SpvExecutionModelVertex:
    return ShaderKind::SpvVertex;
  case SpvExecutionModelFragment:
    return ShaderKind::SpvFragment;
  default:
    throw err::ShaderSpvErr{"Unsupported execution model (shader kind)"};
  }
}

[[nodiscard]] auto isShaderResource(const SpvId& spvId) {
  if (spvId.kind != SpvIdKind::Variable) {
    return false;
  }
  switch (spvId.storageClass) {
  case SpvStorageClass::SpvStorageClassUniform:
  case SpvStorageClass::SpvStorageClassUniformConstant:
  case SpvStorageClass::SpvStorageClassStorageBuffer:
    return true;
  default:
    return false;
  }
}

[[nodiscard]] auto
getShaderResourceKind(const SpvProgram& program, uint32_t typeIdx, SpvStorageClass varStorageClass)
    -> ShaderResourceKind {
  assert(typeIdx < program.ids.size());
  const auto& spvId = program.ids[typeIdx];

  switch (spvId.kind) {
  case SpvIdKind::TypePointer:
    // If the type is a pointer then follow it.
    return getShaderResourceKind(program, spvId.typeId, varStorageClass);
  case SpvIdKind::TypeSampledImage:
    return ShaderResourceKind::Texture;
  case SpvIdKind::TypeStruct:
    switch (varStorageClass) {
    case SpvStorageClassUniform:
    case SpvStorageClassUniformConstant:
      return ShaderResourceKind::UniformBuffer;
    case SpvStorageClassStorageBuffer:
      return ShaderResourceKind::StorageBuffer;
    default:
      [[fallthrough]];
    }
  default:
    throw err::ShaderSpvErr{"Unsupported shader resource found in SpirV"};
  }
}

[[nodiscard]] auto getResources(const SpvProgram& program) -> std::vector<ShaderResource> {
  static_assert(sizeof(uint32_t) == maxShaderBindings / 8);
  std::array<uint32_t, maxShaderSets> usedSlots = {};

  auto result = std::vector<ShaderResource>{};
  for (const auto& spvId : program.ids) {
    if (isShaderResource(spvId)) {
      const auto kind = getShaderResourceKind(program, spvId.typeId, spvId.storageClass);
      if (spvId.set == UINT32_MAX || spvId.binding == UINT32_MAX) {
        throw err::ShaderSpvErr{"Shader resource without set and binding found in SpirV"};
      }
      if (spvId.set >= maxShaderSets) {
        throw err::ShaderSpvErr{"Shader resource set exceeds maximum"};
      }
      if (spvId.binding >= maxShaderBindings) {
        throw err::ShaderSpvErr{"Shader resource binding exceeds maximum"};
      }
      if (usedSlots[spvId.set] & (1U << spvId.binding)) {
        throw err::ShaderSpvErr{"Multiple resources are using the same set + binding"};
      }
      usedSlots[spvId.set] |= 1U << spvId.binding;
      result.emplace_back(kind, spvId.set, spvId.binding);
    }
  }
  return result;
}

} // namespace

auto loadShaderSpv(log::Logger* /*unused*/, DatabaseImpl* /*unused*/, AssetId id, math::RawData raw)
    -> AssetUnique {

  // SpirV consists of 32 bit words so we interpret the file as a set of 32 bit words.
  // TODO(bastian): Consider endianness differences, because SpirV consists of only words it might
  // be easiest to just convert to whole data to host endianess if we are running on a big-endian
  // system.
  if (raw.size() % 4 != 0) {
    throw err::ShaderSpvErr{"Malformed SpirV"};
  }
  const auto* begin = reinterpret_cast<const uint32_t*>(raw.data());
  auto reader       = Reader{begin, begin + raw.size() / 4U};

  // Read the header.
  reader.assertRemainingSize(5); // Space needed for the header.
  if (reader.consume() != SpvMagicNumber) {
    throw err::ShaderSpvErr{"Malformed SpirV"};
  }
  const auto [majorVersion, minorVersion] = decodeVersion(reader.consume());
  if (majorVersion <= 1U && minorVersion < 3U) {
    throw err::ShaderSpvErr{"Unsupported SpirV version, atleast 1.3 is required"};
  }

  reader.skip(1); // Generators magic number.
  const auto maxId = reader.consume();
  reader.skip(1); // Reserved.

  // Read the program.
  auto program          = readProgram(reader, maxId);
  const auto shaderKind = getShaderKind(program.execModel);
  auto resources        = getResources(program);

  return std::make_unique<Shader>(
      std::move(id),
      shaderKind,
      std::move(program.entryPointName),
      std::move(resources),
      std::move(raw));
}

} // namespace tria::asset::internal
