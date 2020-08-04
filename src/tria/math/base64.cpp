#include "tria/math/base64.hpp"
#include <array>

namespace tria::math {

/*
 * MIME Base64 encoding uses 64 'safe' ascii characters to represent 6 bits of data.
 * So to represent 3 bytes of data you need 4 base64 digits (24 bit = 6 * 4).
 *
 * The base64 characters are: 'A - Z', 'a - z' and '0 - 9' and '+' and '/'.
 * '=' is used to pad to a multiple of 4.
 *
 */

namespace {

/* Calculate how many bytes the decoded output will be.
 * Note: Assumes correctly padded input.
 */
[[nodiscard]] auto getDecodedSize(std::string_view input) -> size_t {
  if (input.size() < 2U) {
    // Needs atleast 2 base64 chars to represent a single byte.
    return 0U;
  }
  // Check how many padding characters there are. Either 2, 1 or 0.
  const auto padding = *(input.end() - 2U) == '=' ? 2U : *(input.end() - 1U) == '=' ? 1U : 0U;
  return input.size() / 4U * 3U - padding;
}

/* Mapping of ascii characters, starting at '+' and ending with 'z' to the base64 table.
 * The base64 table can be found on the wiki page: https://en.wikipedia.org/wiki/Base64
 * Note: value of 255 indicates the ascii character is not a valid base64 char.
 */
constexpr std::array<uint8_t, 'z' - '+' + 1> decodeTable = {
    62,  255, 255, 255, 63, 52, 53, 54, 55,  56,  57,  58,  59,  60,  61, 255, 255, 255, 255, 255,
    255, 255, 0,   1,   2,  3,  4,  5,  6,   7,   8,   9,   10,  11,  12, 13,  14,  15,  16,  17,
    18,  19,  20,  21,  22, 23, 24, 25, 255, 255, 255, 255, 255, 255, 26, 27,  28,  29,  30,  31,
    32,  33,  34,  35,  36, 37, 38, 39, 40,  41,  42,  43,  44,  45,  46, 47,  48,  49,  50,  51};

} // namespace

auto base64Decode(std::string_view input) noexcept -> RawData {
  // Implemention based on awnser of 'nunojpg' in the so question:
  // https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c

  auto result = RawData{};
  result.reserve(getDecodedSize(input));

  auto val     = 0U;
  auto valBits = -8; // 0 indicates we have a 'full' 8 bit value in 'val'.
  for (const uint8_t c : input) {
    if (c < '+' || c > 'z') {
      // Non base64 characters found: abort.
      break;
    }
    if (decodeTable[c - '+'] == 255U) {
      // Non base64 character found: abort.
      break;
    }
    // Each Base64 digit contains 6 bits of data, shift the current value over by 6 and put the new
    // data in the least significant bits.
    val = (val << 6U) | decodeTable[c - '+'];
    valBits += 6; // Indicate that we have 6 more bits 'available'.
    if (valBits >= 0) {
      // We have enough bits to form a byte.
      result.push_back(static_cast<uint8_t>(val >> valBits)); // Shift-of any excess bits.
      valBits -= 8;
    }
  }
  return result;
}

} // namespace tria::math
