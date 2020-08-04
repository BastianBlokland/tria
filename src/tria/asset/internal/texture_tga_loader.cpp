#include "loader.hpp"
#include "tria/asset/err/asset_load_err.hpp"
#include "tria/asset/texture.hpp"
#include <limits>
#include <optional>

namespace tria::asset::internal {

/* Truevision TGA.
 * Supports 24 bit (rgb) and 32 bit (rgba) and optionally rle compressed.
 * Format information: https://en.wikipedia.org/wiki/Truevision_TGA
 * Format examples: http://www.gamers.org/dEngine/quake3/TGA.txt
 * Color info: http://www.ryanjuckett.com/programming/parsing-colors-in-a-tga-file/
 */

namespace {

// clang-format off
template <size_t Size>
using uint_t =  std::conditional_t<Size < 1, void,
                std::conditional_t<Size < 2, uint8_t,
                std::conditional_t<Size < 3, uint16_t,
                std::conditional_t<Size < 5, uint32_t, void>>>>;
// clang-format on

enum class TgaColorMapType : uint8_t {
  Absent  = 0,
  Present = 1,
};

enum class TgaImageType : uint8_t {
  ColorMapped    = 1,
  TrueColor      = 2,
  Grayscale      = 3,
  RleColorMapped = 9,
  RleTrueColor   = 10,
  RleGrayscale   = 11,
};

enum class TgaOrigin : uint8_t {
  LowerLeft  = 0,
  LowerRight = 1,
  UpperLeft  = 2,
  UpperRight = 3,
};

enum class TgaInterleave : uint8_t {
  None    = 0,
  EvenOdd = 1,
  FourWay = 2,
};

struct TgaColorMapSpec final {
  uint16_t mapStart;
  uint16_t mapLength;
  uint8_t entrySize;
};

struct TgaImageDescriptor final {
  uint8_t attributeDepth;
  TgaOrigin origin;
  TgaInterleave interleave;
};

struct TgaImageSpec final {
  math::Vec<uint16_t, 2> origin;
  math::Vec<uint16_t, 2> size;
  uint8_t bitsPerPixel;
  TgaImageDescriptor descriptor;
};

struct TgaHeader final {
  uint8_t idLength;
  TgaColorMapType colorMapType;
  TgaImageType imageType;
  TgaColorMapSpec colorMapSpec;
  TgaImageSpec imageSpec;
};

class Reader final {
public:
  Reader(const uint8_t* current, const uint8_t* end) : m_cur{current}, m_end{end} {}

  /* Check how much data is remaining.
   */
  auto getRemainingCount() { return m_end - m_cur; }

  /* Skip an amount of bytes.
   * Returns true if succeeded (enough data available) otherwise false (and pointer is not
   * modified).
   */
  [[nodiscard]] auto skip(unsigned int count) {
    if (getRemainingCount() < count) {
      return false;
    }
    m_cur += count;
    return true;
  }

  /* Consume a single 'value'.
   * Tga images are in little-endian, we read byte for byte to host endianness.
   */
  template <unsigned int Size>
  auto consume() -> uint_t<Size> {
    auto result = uint_t<Size>{};
    for (auto i = 0U; i != Size; ++i) {
      const auto byte = static_cast<uint8_t>(*m_cur++);
      result |= static_cast<uint_t<Size>>(byte) << (i * 8U);
    }
    return result;
  }

private:
  const uint8_t* m_cur;
  const uint8_t* m_end;
};

/* Decode a tga image-descriptor byte.
 */
[[nodiscard]] auto getTgaImageDescriptor(uint8_t raw) noexcept -> TgaImageDescriptor {
  TgaImageDescriptor desc = {};
  desc.attributeDepth     = raw & 0b0000'1111;
  desc.origin             = static_cast<TgaOrigin>((raw & 0b0011'0000) >> 4U);
  desc.interleave = static_cast<TgaInterleave>((raw & static_cast<uint8_t>(0b1100'0000)) >> 6U);
  return desc;
}

[[nodiscard]] auto readTgaHeader(Reader& reader) noexcept -> std::optional<TgaHeader> {
  if (reader.getRemainingCount() < 18U) {
    // Not enough data for a tga-header.
    return std::nullopt;
  }
  TgaHeader header              = {};
  header.idLength               = reader.consume<1>();
  header.colorMapType           = static_cast<TgaColorMapType>(reader.consume<1>());
  header.imageType              = static_cast<TgaImageType>(reader.consume<1>());
  header.colorMapSpec.mapStart  = reader.consume<2>();
  header.colorMapSpec.mapLength = reader.consume<2>();
  header.colorMapSpec.entrySize = reader.consume<1>();
  header.imageSpec.origin.x()   = reader.consume<2>();
  header.imageSpec.origin.y()   = reader.consume<2>();
  header.imageSpec.size.x()     = reader.consume<2>();
  header.imageSpec.size.y()     = reader.consume<2>();
  header.imageSpec.bitsPerPixel = reader.consume<1>();
  header.imageSpec.descriptor   = getTgaImageDescriptor(reader.consume<1>());
  return header;
}

/* Read tga pixel data.
 * Returns empty vector when not enough data is available in the reader.
 */
template <bool HasAlpha, bool Rle, bool YFlip>
[[nodiscard]] auto readTgaPixels(Reader& reader, TextureSize size) noexcept
    -> math::PodVector<Pixel> {

  const auto pixelCount    = static_cast<uint32_t>(size.x()) * size.y();
  constexpr auto pixelSize = HasAlpha ? 4U : 3U;

  if constexpr (!Rle) {
    // Without run-length-encoding we can ahead of time check if enough data is available.
    if (reader.getRemainingCount() < pixelCount * pixelSize) {
      return {};
    }
  }

  auto result = math::PodVector<Pixel>(pixelCount);

  auto packetRemaining = 0U; // How many pixels are left in the current rle packet.
  auto packetRefPixel  = std::numeric_limits<unsigned int>::max();
  for (auto y = 0U; y != size.y(); ++y) {
    for (auto x = 0U; x != size.x(); ++x) {

      // Either fill pixels from top to bottom - left to right, or bottom to top - left to right.
      auto i = (YFlip ? (size.y() - 1U - y) * size.x() : y * size.x()) + x;

      if constexpr (Rle) {
        /* For run-length-encoding there is a header before each 'packet' with information about the
         * packet.
         * There are two types of packets:
         * - run-length-packet: Contains a repetition count and a single pixel to repeat.
         * - raw-packet: Contains a count of how many 'raw' pixels will follow.
         */
        if (packetRemaining == 0U) {
          // If no pixels are remaining then read a new packet header.

          if (reader.getRemainingCount() <= pixelSize) {
            // Not enough data for a header byte and a single pixel.
            return {};
          }
          auto packetHeader = reader.consume<1>();
          auto isRlePacket  = (packetHeader & 0b1000'0000) != 0U; // Msb indicates packet type.
          packetRefPixel    = isRlePacket ? i : std::numeric_limits<unsigned int>::max();
          packetRemaining   = (packetHeader & 0b0111'1111); // Remaining 7 bits are the rep count.

          // For raw packets there needs to be enough data left for 'count' of pixels.
          if (!isRlePacket && reader.getRemainingCount() < packetRemaining * pixelSize) {
            return {};
          }
        } else {
          // This pixel is still start of the same packet.
          --packetRemaining;
        }
      }

      if (packetRefPixel < i) {
        // If there is a reference pixel then we copy that one. This happens in rle packets.
        result[i] = result[packetRefPixel];
      } else {
        // Read a new pixel value.
        result[i].b() = reader.consume<1>();
        result[i].g() = reader.consume<1>();
        result[i].r() = reader.consume<1>();
        if constexpr (HasAlpha) {
          result[i].a() = reader.consume<1>();
        } else {
          result[i].a() = 255; // Treat images without alpha as fully opaque.
        }
      }
    }
  }
  return result;
}

/* Read tga pixel data.
 * Returns empty vector when not enough data is available in the reader.
 */
[[nodiscard]] auto
readTgaPixels(Reader& reader, TextureSize size, bool hasAlpha, bool rle, TgaOrigin origin) noexcept
    -> math::PodVector<Pixel> {
  // TODO(bastian): Support images with the origin on the right? Haven't seen any in the wild.
  switch (origin) {
  case TgaOrigin::LowerLeft:
  case TgaOrigin::LowerRight:
    return rle ? hasAlpha ? readTgaPixels<true, true, true>(reader, size)
                          : readTgaPixels<false, true, true>(reader, size)
               : hasAlpha ? readTgaPixels<true, false, true>(reader, size)
                          : readTgaPixels<false, false, true>(reader, size);
  case TgaOrigin::UpperLeft:
  case TgaOrigin::UpperRight:
  default:
    return rle ? hasAlpha ? readTgaPixels<true, true, false>(reader, size)
                          : readTgaPixels<false, true, false>(reader, size)
               : hasAlpha ? readTgaPixels<true, false, false>(reader, size)
                          : readTgaPixels<false, false, false>(reader, size);
  }
}

} // namespace

auto loadTextureTga(
    log::Logger* /*unused*/,
    DatabaseImpl* /*unused*/,
    AssetId id,
    const fs::path& path,
    math::RawData raw) -> AssetUnique {

  auto reader       = Reader{raw.begin(), raw.end()};
  const auto header = readTgaHeader(reader);

  if (!header) {
    throw err::AssetLoadErr{path, "Malformed tga header"};
  }
  if (header->colorMapType == TgaColorMapType::Present) {
    throw err::AssetLoadErr{path, "Colormapped tga files are not supported"};
  }
  if (header->imageSpec.bitsPerPixel != 24 && header->imageSpec.bitsPerPixel != 32U) {
    throw err::AssetLoadErr{
        path, "Unsupported pixel bit depth, only 24 bit (RGB) and 32 bit (RGBA) are supported"};
  }
  const auto hasAlpha = header->imageSpec.bitsPerPixel == 32U;
  if (hasAlpha && header->imageSpec.descriptor.attributeDepth != 8U) {
    throw err::AssetLoadErr{path, "Only 8 bit alpha channel is supported"};
  }
  if (header->imageSpec.descriptor.interleave != TgaInterleave::None) {
    throw err::AssetLoadErr{path, "Interleaved tga files are not supported"};
  }
  if (header->imageType != TgaImageType::TrueColor &&
      header->imageType != TgaImageType::RleTrueColor) {
    throw err::AssetLoadErr{path, "Unsupported image-type, only TrueColor is supported"};
  }
  const auto isRle = header->imageType == TgaImageType::RleTrueColor;

  // Skip over the id field.
  if (!reader.skip(header->idLength)) {
    throw err::AssetLoadErr{path, "Unexpected end of tga file"};
  }

  const auto origin = header->imageSpec.descriptor.origin;
  const auto size   = TextureSize{header->imageSpec.size};
  if (size.x() == 0U || size.y() == 0U) {
    throw err::AssetLoadErr{path, "Malformed tga size, needs to be bigger then 0"};
  }

  const auto pixelCount = static_cast<uint32_t>(size.x()) * size.y();

  auto pixels = readTgaPixels(reader, size, hasAlpha, isRle, origin);
  if (pixels.size() != pixelCount) {
    throw err::AssetLoadErr{path, "Unexpected end of tga file"};
  }

  return std::make_unique<Texture>(std::move(id), size, std::move(pixels));
}

} // namespace tria::asset::internal
