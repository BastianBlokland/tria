#include "loader.hpp"
#include "tria/asset/err/shader_spv_err.hpp"
#include "tria/asset/shader.hpp"
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

  [[nodiscard]] auto getCur() noexcept { return m_cur; }

  [[nodiscard]] auto getRemainingCount() noexcept { return static_cast<size_t>(m_end - m_cur); }

  [[nodiscard]] auto read() noexcept {
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

auto decodeVersion(uint32_t raw) noexcept {
  return std::make_pair(static_cast<uint8_t>(raw >> 16U), static_cast<uint8_t>(raw >> 8U));
}

auto decodeInstructionHeader(uint32_t raw) noexcept {
  return std::make_pair(static_cast<uint16_t>(raw), static_cast<uint16_t>(raw >> 16U));
}

struct SpvId {};

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
    case SpvOpEntryPoint:
      if (!program.entryPointName.empty()) {
        throw err::ShaderSpvErr{"Multiple entrypoints are not supported"};
      }
      reader.assertRemainingSize(2); // 1 for header + 1 for mode.
      program.execModel = static_cast<SpvExecutionModel>(instrBase[1]);
      // const auto entryPointId = instrBase[2];
      program.entryPointName =
          readStringLiteral(instrBase + 3U, static_cast<size_t>(opSize - 3U)).first;
      break;
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

} // namespace

auto loadShaderSpv(log::Logger* /*unused*/, DatabaseImpl* /*unused*/, AssetId id, math::RawData raw)
    -> AssetUnique {

  // SpirV consists of 32 bit words so we interpret the file as a set of 32 bit words.
  // TODO(bastian): Consider endianness differences.
  if (raw.size() % 4 != 0) {
    throw err::ShaderSpvErr{"Malformed spir-v"};
  }
  const auto* begin = reinterpret_cast<const uint32_t*>(raw.data());
  auto reader       = Reader{begin, begin + raw.size() / 4U};

  // Read the header.
  reader.assertRemainingSize(5); // Space needed for the header.
  if (reader.read() != SpvMagicNumber) {
    throw err::ShaderSpvErr{"Malformed spir-v"};
  }
  decodeVersion(reader.read());
  reader.skip(1); // Generators magic number.
  const auto maxId = reader.read();
  reader.skip(1); // Reserved.

  // Read the program.
  auto program          = readProgram(reader, maxId);
  const auto shaderKind = getShaderKind(program.execModel);

  return std::make_unique<Shader>(
      std::move(id), shaderKind, std::move(program.entryPointName), std::move(raw));
}

} // namespace tria::asset::internal
