#include "loader.hpp"
#include "tria/asset/err/asset_load_err.hpp"
#include "tria/asset/texture.hpp"

namespace tria::asset::internal {

/* Portable Pixmap Format.
 * Ascii format P3 and binary format P6 are both suported.
 * Format specification: https://en.wikipedia.org/wiki/Netpbm
 */

namespace {

enum class PixmapType {
  Unknown,
  Ascii,
  Binary,
};

struct PixmapHeader final {
  PixmapType type;
  unsigned int width;
  unsigned int height;
  unsigned int maxValue;
};

class Reader final {
public:
  Reader(const char* current, const char* end) : m_cur{current}, m_end{end} {}

  [[nodiscard]] auto getCurrent() -> const char*& { return m_cur; }
  [[nodiscard]] auto getEnd() -> const char* { return m_end; }

  auto consumeChar() -> char {
    if (m_cur != m_end) {
      return *m_cur++;
    }
    return '\0';
  }

  [[nodiscard]] auto consumeChar(char c) -> bool {
    if (m_cur != m_end && *m_cur == c) {
      ++m_cur; // Consume the character.
      return true;
    }
    return false;
  }

  auto consumeWhitespace() noexcept -> void {
    for (; m_cur != m_end; ++m_cur) {
      switch (*m_cur) {
      case '\n':
      case '\r':
      case '\t':
      case ' ':
        // Whitespace: keep consuming.
        break;
      default:
        // Not whitespace: stop consuming.
        return;
      }
    }
  }

  auto consumeLine() noexcept -> void {
    for (; m_cur != m_end; ++m_cur) {
      if (*m_cur == '\n') {
        ++m_cur; // Consume the new-line itself.
        return;
      }
    }
  }

  auto consumeWhitespaceOrComment() noexcept -> void {
    for (; m_cur != m_end;) {
      consumeWhitespace();
      if (m_cur != m_end && *m_cur != '#') {
        // Not whitespace or the start of a comment: stop consuming.
        return;
      }
      // Consume the rest of the line as its part of the comment.
      consumeLine();
    }
  }

  auto consumeInt() noexcept -> unsigned int {
    auto result = 0U;
    for (; m_cur != m_end; ++m_cur) {
      auto c = *m_cur;
      if (c < '0' || c > '9') {
        // Not a digit: stop consuming.
        break;
      }
      result *= 10;
      result += c - '0';
    }
    return result;
  }

private:
  const char* m_cur;
  const char* m_end;
};

[[nodiscard]] auto readPixmapType(Reader& reader) noexcept -> PixmapType {
  if (!reader.consumeChar('P')) {
    return PixmapType::Unknown;
  }
  if (reader.consumeChar('3')) {
    return PixmapType::Ascii;
  }
  if (reader.consumeChar('6')) {
    return PixmapType::Binary;
  }
  return PixmapType::Unknown;
}

[[nodiscard]] auto readHeader(Reader& reader) noexcept -> PixmapHeader {
  PixmapHeader result;
  reader.consumeWhitespaceOrComment();
  result.type = readPixmapType(reader);
  reader.consumeWhitespaceOrComment();
  result.width = reader.consumeInt();
  reader.consumeWhitespaceOrComment();
  result.height = reader.consumeInt();
  reader.consumeWhitespaceOrComment();
  result.maxValue = reader.consumeInt();
  return result;
}

[[nodiscard]] auto readPixelsAscii(Reader& reader, unsigned int count) noexcept
    -> math::PodVector<Pixel> {
  auto result = math::PodVector<Pixel>(count);
  for (auto i = 0U; i != count; ++i) {
    reader.consumeWhitespaceOrComment();
    result[i].r() = reader.consumeInt();
    reader.consumeWhitespaceOrComment();
    result[i].g() = reader.consumeInt();
    reader.consumeWhitespaceOrComment();
    result[i].b() = reader.consumeInt();
    result[i].a() = 255; // Pixmap doesn't support alpha, use fully opaque.
  }
  return result;
}

[[nodiscard]] auto readPixelsBinary(Reader& reader, unsigned int count) noexcept
    -> math::PodVector<Pixel> {

  // A single character should separate the header and the data.
  // Note: this means you cannot use a windows style line-ending between the header and data, but a
  // space would work fine.
  reader.consumeChar();

  // Check if enough data for the given pixels is left in the reader.
  if (reader.getEnd() - reader.getCurrent() < count * 3) {
    return {};
  }

  auto result = math::PodVector<Pixel>(count);
  for (auto i = 0U; i != count; ++i) {
    std::memcpy(&result[i], reader.getCurrent(), 3);
    reader.getCurrent() += 3;
    result[i].a() = 255; // Pixmap doesn't support alpha, use fully opaque.
  }
  return result;
}

} // namespace

auto loadTexturePpm(
    log::Logger* /*unused*/,
    DatabaseImpl* /*unused*/,
    AssetId id,
    const fs::path& path,
    math::RawData raw) -> AssetUnique {

  auto reader = Reader{raw.begin(), raw.end()};
  auto header = readHeader(reader);

  if (header.type == PixmapType::Unknown) {
    throw err::AssetLoadErr{path, "Malformed pixmap type, expected 'P3' or 'P6"};
  }
  if (header.width == 0U || header.height == 0U) {
    throw err::AssetLoadErr{path, "Malformed pixmap size, needs to be bigger then 0"};
  }
  if (header.width == 0U || header.height == 0U) {
    throw err::AssetLoadErr{path, "Malformed pixmap size, needs to be bigger then 0"};
  }
  if (header.maxValue != 255) {
    throw err::AssetLoadErr{path, "Only 8bit Pixmap files are supported"};
  }

  auto size       = TextureSize{header.width, header.height};
  auto pixelCount = header.width * header.height;
  auto pixels     = header.type == PixmapType::Ascii ? readPixelsAscii(reader, pixelCount)
                                                 : readPixelsBinary(reader, pixelCount);

  if (pixels.size() < pixelCount) {
    throw err::AssetLoadErr{path, "Not enough pixel data in file for specified amount of pixels"};
  }
  return std::make_unique<Texture>(std::move(id), size, std::move(pixels));
}

} // namespace tria::asset::internal
