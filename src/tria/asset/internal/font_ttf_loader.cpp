#include "loader.hpp"
#include "tria/asset/err/font_ttf_err.hpp"
#include "tria/asset/font.hpp"
#include "tria/math/box.hpp"
#include "tria/math/pod_vector_io.hpp"
#include "tria/math/vec.hpp"
#include <optional>
#include <string>
#include <unordered_map>

namespace tria::asset::internal {

/* TrueType font.
 * Only simple TrueType outlines are supported (no composites at this time).
 * Apple docs: https://developer.apple.com/fonts/TrueType-Reference-Manual/
 * Microsoft docs: https://docs.microsoft.com/en-us/typography/opentype/spec/otff
 */

namespace {

// clang-format off
template <size_t Size>
using uint_t =  std::conditional_t<Size < 1, void,
                std::conditional_t<Size < 2, uint8_t,
                std::conditional_t<Size < 3, uint16_t,
                std::conditional_t<Size < 5, uint32_t,
                std::conditional_t<Size < 9, uint64_t, void>>>>>;

template <size_t Size>
using int_t =   std::conditional_t<Size < 1, void,
                std::conditional_t<Size < 2, int8_t,
                std::conditional_t<Size < 3, int16_t,
                std::conditional_t<Size < 5, int32_t,
                std::conditional_t<Size < 9, int64_t, void>>>>>;
// clang-format on

class Reader final {
public:
  Reader(const uint8_t* current, const uint8_t* end) : m_cur{current}, m_end{end} {}

  [[nodiscard]] auto getCurrent() noexcept { return m_cur; }

  /* Check how much data is remaining.
   */
  [[nodiscard]] auto getRemainingCount() noexcept { return m_end - m_cur; }

  /* Skip an amount of bytes.
   * Returns true if succeeded (enough data available) otherwise false (and pointer is not
   * modified).
   */
  [[nodiscard]] auto skip(unsigned int count) noexcept {
    if (getRemainingCount() < count) {
      return false;
    }
    m_cur += count;
    return true;
  }

  /* Consume a single unsigned integer.
   * Ttf fonts are in big-endian, we read byte for byte to host endianness.
   */
  template <unsigned int Size>
  auto consumeUInt() noexcept -> uint_t<Size> {
    auto result = uint_t<Size>{};
    for (auto i = Size; i-- != 0U;) {
      const auto byte = static_cast<uint8_t>(*m_cur++);
      result |= static_cast<uint_t<Size>>(byte) << (i * 8U);
    }
    return result;
  }

  template <unsigned int Size>
  auto consumeUIntChecked() noexcept -> uint_t<Size> {
    if (getRemainingCount() < Size) {
      return {};
    }
    return consumeUInt<Size>();
  }

  /* Consume a single signed integer.
   * Ttf fonts are in big-endian, we read byte for byte to host endianness.
   * Ttf format uses 2's complement integers and we assume the host is using that also.
   */
  template <unsigned int Size>
  auto consumeInt() noexcept -> int_t<Size> {
    const auto raw = consumeUInt<Size>();
    return reinterpret_cast<const int_t<Size>&>(raw);
  }

  template <unsigned int Size>
  auto consumeIntChecked() noexcept -> int_t<Size> {
    if (getRemainingCount() < Size) {
      return {};
    }
    return consumeInt<Size>();
  }

private:
  const uint8_t* m_cur;
  const uint8_t* m_end;
};

/* Record of a table in the ttf file.
 * The tag is used to identify the table.
 */
struct TtfTableRecord final {
  std::string tag;
  uint32_t checksum;
  uint32_t offset;
  uint32_t length;
};

using TtfTableRecordMap = std::unordered_map<std::string, TtfTableRecord>;

/* Offset table contains records for all tables in the file.
 */
struct TtfOffsetTable final {
  uint32_t sfntVersion;
  uint16_t numTables;
  uint16_t searchRange;
  uint16_t entrySelector;
  uint16_t rangeShift;
  TtfTableRecordMap records;
};

/* Head table acts a global header for the file.
 */
struct TtfHeadTable final {
  uint16_t majorVersion;
  uint16_t minorVersion;
  float fontRevision;
  uint32_t checksumAdjustment;
  uint32_t magicNumber;
  uint16_t flags;
  uint16_t unitsPerEm;
  int64_t dateCreated;
  int64_t dateModified;
  math::Box<int16_t, 2> glyphBounds;
  uint16_t macStyle;
  uint16_t lowestRecPpem;
  int16_t fontDirectionHint;
  int16_t indexToLocFormat;
  int16_t glyphDataFormat;
};

/* Maxp table contains memory requirements for this font.
 */
struct TtfMaxpTable final {
  float version;
  uint16_t numGlyphs;
  uint16_t maxPoints;
  uint16_t maxContours;
  uint16_t maxCompositePoints;
  uint16_t maxCompositeContours;
  uint16_t maxZones;
  uint16_t maxTwilightPoints;
  uint16_t maxStorage;
  uint16_t maxFunctionDefs;
  uint16_t maxInstructionDefs;
  uint16_t maxStackElements;
  uint16_t maxSizeOfInstructions;
  uint16_t maxComponentElements;
  uint16_t maxComponentDepth;
};

struct TtfEncodingRecord final {
  uint16_t platformId;
  uint16_t encodingId;
  size_t offset; // From the start of the file.
};

struct TtfCmapTable final {
  uint16_t version;
  math::PodVector<TtfEncodingRecord> subtables;
};

struct TtfGlyphHeader final {
  int16_t numContours;
  math::Box<int16_t, 2> glyphBounds;
};

struct TtfGlyph final {
  size_t dataOffset; // From the start of the file.
  size_t dataSize;
  math::PodVector<CodePoint> codepoints;
  math::PodVector<math::Vec2f> points;
  math::PodVector<GlyphSegment> segments;
};

/* Four character string used to identify tables.
 * More info: https://docs.microsoft.com/en-us/typography/opentype/spec/otff#data-types
 */
[[nodiscard]] auto readTtfTag(Reader& reader) noexcept -> std::string {
  std::string result = {};
  if (reader.getRemainingCount() >= 4U) {
    for (auto i = 0U; i != 4U; ++i) {
      result += static_cast<char>(reader.consumeUInt<1>());
    }
  }
  return result;
}

/* Read a 32 bit signed fixed-point number (16.16).
 * Note: Does not check if enough space is available in the reader.
 */
[[nodiscard]] auto readTtfFixed(Reader& reader) noexcept -> float {
  return reader.consumeInt<4>() / static_cast<float>(1U << 16U);
}

[[nodiscard]] auto readTtfOffsetTable(Reader& reader) noexcept -> std::optional<TtfOffsetTable> {
  if (reader.getRemainingCount() < 12U) {
    // Not enough data for a offset table.
    return std::nullopt;
  }
  TtfOffsetTable result = {};
  result.sfntVersion    = reader.consumeUInt<4>();
  result.numTables      = reader.consumeUInt<2>();
  result.searchRange    = reader.consumeUInt<2>();
  result.entrySelector  = reader.consumeUInt<2>();
  result.rangeShift     = reader.consumeUInt<2>();

  if (reader.getRemainingCount() < result.numTables * 16U) {
    // Not enough space for records for all the tables.
    return {};
  }
  for (auto i = 0U; i != result.numTables; ++i) {
    TtfTableRecord rec;
    rec.tag      = readTtfTag(reader);
    rec.checksum = reader.consumeUInt<4>();
    rec.offset   = reader.consumeUInt<4>();
    rec.length   = reader.consumeUInt<4>();
    result.records.emplace(rec.tag, rec);
  }
  return result;
}

/* Calculate a checksum of the input data.
 * Both offset and length have to be aligned to a 4 byte boundary.
 * More info: https://docs.microsoft.com/en-us/typography/opentype/spec/otff#calculating-checksums
 */
[[nodiscard]] auto calcTtfChecksum(const math::RawData& raw, uint32_t offset, uint32_t length) {
  assert((offset % 4U) == 0U);
  assert((length % 4U) == 0U);
  assert(raw.size() >= offset + length);

  auto reader = Reader{raw.begin() + offset, raw.begin() + offset + length};
  auto sum    = 0U;
  while (reader.getRemainingCount()) {
    sum += reader.consumeUInt<4>();
  }
  return sum;
}

/* Verify the contents of the ttf file against the table records.
 */
[[nodiscard]] auto
validateTtfFile(const math::RawData& raw, const TtfTableRecordMap& tables) noexcept {
  for (const auto& [tableName, table] : tables) {
    if (table.offset % 4U) {
      // All tables should be aligned on a 4 byte boundary.
      return false;
    }
    const auto alignedLength = (table.length + 3) & ~3;
    if (raw.size() < table.offset + alignedLength) {
      // Not enough data is available for this table.
      // Note: ttf tables have to be padded to the next 4 byte boundary.
      return false;
    }
    if (tableName == "head") {
      // TODO(bastian): Validate head table checksum, for the head table the checksum works
      // differently as it contains a checksum adjustment for the entire font.
      continue;
    }
    if (calcTtfChecksum(raw, table.offset, alignedLength) != table.checksum) {
      // Mismatching checksum, file must corrupted.
      return false;
    }
  }
  return true;
}

[[nodiscard]] auto
readHeadTable(const math::RawData& raw, const TtfTableRecordMap& recordMap) noexcept
    -> std::optional<TtfHeadTable> {

  const auto headItr = recordMap.find("head");
  if (headItr == recordMap.end()) {
    return std::nullopt;
  }
  const auto dataOffset = headItr->second.offset;
  const auto dataLength = headItr->second.length;

  auto reader = Reader{raw.begin() + dataOffset, raw.begin() + dataOffset + dataLength};
  if (reader.getRemainingCount() < 54U) {
    // Not enough data for a head table.
    return std::nullopt;
  }
  TtfHeadTable result          = {};
  result.majorVersion          = reader.consumeUInt<2>();
  result.minorVersion          = reader.consumeUInt<2>();
  result.fontRevision          = readTtfFixed(reader);
  result.checksumAdjustment    = reader.consumeUInt<4>();
  result.magicNumber           = reader.consumeUInt<4>();
  result.flags                 = reader.consumeUInt<2>();
  result.unitsPerEm            = reader.consumeUInt<2>();
  result.dateCreated           = reader.consumeInt<8>();
  result.dateModified          = reader.consumeInt<8>();
  result.glyphBounds.min().x() = reader.consumeInt<2>();
  result.glyphBounds.min().y() = reader.consumeInt<2>();
  result.glyphBounds.max().x() = reader.consumeInt<2>();
  result.glyphBounds.max().y() = reader.consumeInt<2>();
  result.macStyle              = reader.consumeUInt<2>();
  result.lowestRecPpem         = reader.consumeUInt<2>();
  result.fontDirectionHint     = reader.consumeInt<2>();
  result.indexToLocFormat      = reader.consumeInt<2>();
  result.glyphDataFormat       = reader.consumeInt<2>();
  return result;
}

[[nodiscard]] auto
readMaxpTable(const math::RawData& raw, const TtfTableRecordMap& recordMap) noexcept
    -> std::optional<TtfMaxpTable> {

  const auto maxpItr = recordMap.find("maxp");
  if (maxpItr == recordMap.end()) {
    return std::nullopt;
  }
  const auto dataOffset = maxpItr->second.offset;
  const auto dataLength = maxpItr->second.length;

  auto reader = Reader{raw.begin() + dataOffset, raw.begin() + dataOffset + dataLength};
  if (reader.getRemainingCount() < 32U) {
    // Not enough data for a v1 maxp table.
    return std::nullopt;
  }
  TtfMaxpTable result          = {};
  result.version               = readTtfFixed(reader);
  result.numGlyphs             = reader.consumeUInt<2>();
  result.maxPoints             = reader.consumeUInt<2>();
  result.maxContours           = reader.consumeUInt<2>();
  result.maxCompositePoints    = reader.consumeUInt<2>();
  result.maxCompositeContours  = reader.consumeUInt<2>();
  result.maxZones              = reader.consumeUInt<2>();
  result.maxTwilightPoints     = reader.consumeUInt<2>();
  result.maxStorage            = reader.consumeUInt<2>();
  result.maxFunctionDefs       = reader.consumeUInt<2>();
  result.maxInstructionDefs    = reader.consumeUInt<2>();
  result.maxStackElements      = reader.consumeUInt<2>();
  result.maxSizeOfInstructions = reader.consumeUInt<2>();
  result.maxComponentElements  = reader.consumeUInt<2>();
  result.maxComponentDepth     = reader.consumeUInt<2>();
  return result;
}

[[nodiscard]] auto readCmap(const math::RawData& raw, const TtfTableRecordMap& recordMap) noexcept
    -> std::optional<TtfCmapTable> {

  const auto cmapItr = recordMap.find("cmap");
  if (cmapItr == recordMap.end()) {
    return std::nullopt;
  }
  const auto cmapOffset = cmapItr->second.offset;
  const auto cmapLength = cmapItr->second.length;

  auto reader = Reader{raw.begin() + cmapOffset, raw.begin() + cmapOffset + cmapLength};
  if (reader.getRemainingCount() < 4U) {
    return std::nullopt; // Not enough data for cmap header.
  }

  TtfCmapTable result;
  result.version = reader.consumeUInt<2>();
  if (result.version != 0U) {
    return std::nullopt; // Unsupported cmap version.
  }

  const auto numTables = reader.consumeUInt<2>();
  if (reader.getRemainingCount() < numTables * 8U) {
    return std::nullopt; // Not enough data for the encoding tables.
  }

  result.subtables = math::PodVector<TtfEncodingRecord>{numTables};
  for (auto i = 0U; i != numTables; ++i) {
    result.subtables[i].platformId = reader.consumeUInt<2>();
    result.subtables[i].encodingId = reader.consumeUInt<2>();
    result.subtables[i].offset     = cmapOffset + reader.consumeUInt<4>();
  }

  return std::optional{std::move(result)};
}

[[nodiscard]] auto readCmapFormat4(
    const math::RawData& raw, size_t offset, size_t size, std::vector<TtfGlyph>& output) noexcept {

  auto reader = Reader{raw.begin() + offset, raw.begin() + offset + size};

  struct {
    uint16_t language; // Unused as we only support unicode (non language specific).
    uint16_t doubleSegcount;
    uint16_t searchRange;
    uint16_t entrySelector;
    uint16_t rangeShift;
    math::PodVector<uint16_t> endCodes;
    uint16_t reservedPad;
    math::PodVector<uint16_t> startCodes;
    math::PodVector<uint16_t> deltas;
    math::PodVector<size_t> rangeOffsets;
  } header;
  header.language       = reader.consumeUIntChecked<2>();
  header.doubleSegcount = reader.consumeUIntChecked<2>();
  const auto segCount   = header.doubleSegcount / 2U;
  header.searchRange    = reader.consumeUIntChecked<2>();
  header.entrySelector  = reader.consumeUIntChecked<2>();
  header.rangeShift     = reader.consumeUIntChecked<2>();
  header.endCodes       = math::PodVector<uint16_t>{segCount};
  for (auto i = 0U; i != segCount; ++i) {
    header.endCodes[i] = reader.consumeUInt<2>();
  }
  header.reservedPad = reader.consumeUIntChecked<2U>();
  header.startCodes  = math::PodVector<uint16_t>{segCount};
  for (auto i = 0U; i != segCount; ++i) {
    header.startCodes[i] = reader.consumeUIntChecked<2>();
  }
  header.deltas = math::PodVector<uint16_t>{segCount};
  for (auto i = 0U; i != segCount; ++i) {
    header.deltas[i] = reader.consumeIntChecked<2>();
  }
  header.rangeOffsets = math::PodVector<size_t>{segCount};
  for (auto i = 0U; i != segCount; ++i) {
    const auto rangeOffset = reader.consumeUIntChecked<2>();
    // Range offsets are a tad strange, as they are actually an offset from the current location in
    // the file, here we convert them to absolute offsets into the file.
    header.rangeOffsets[i] =
        rangeOffset == 0U ? 0U : reader.getCurrent() - raw.begin() + rangeOffset - 2U;
  }

  // Iterate over every segment (block of codepoints) and map the codepoint to the glyph.
  for (auto segmentIdx = 0U; segmentIdx != segCount; ++segmentIdx) {
    const auto startCode   = header.startCodes[segmentIdx];
    const auto endCode     = header.endCodes[segmentIdx];
    const auto delta       = header.deltas[segmentIdx];
    const auto rangeOffset = header.rangeOffsets[segmentIdx];
    if (startCode == 0xFFFF || endCode == 0xFFFF) {
      continue; // 0xFFFF in the ending segment as a stop sentinel.
    }
    for (auto code = startCode; code <= endCode; ++code) {
      // There are two different ways of mapping segments to glyphs, either a direct mapping (with
      // an offset) or a lookup table.
      if (rangeOffset == 0U) {
        // Directly map a code-point to a glyph (with a offset named 'delta').
        const auto glyphId = (code + delta) % 65536U;
        if (glyphId < output.size()) {
          output[glyphId].codepoints.push_back(code);
        }
      } else {
        // Read the glyph-id from a lookup table.
        auto glyphIdReader = Reader{raw.begin() + rangeOffset + (code - startCode) * 2U, raw.end()};
        auto glyphId       = glyphIdReader.consumeUIntChecked<2>();
        if (glyphId < output.size()) {
          output[glyphId].codepoints.push_back(code);
        }
      }
    }
  }
  return true;
}

[[nodiscard]] auto readCodepoints(
    const math::RawData& raw,
    const TtfCmapTable& cmapTable,
    std::vector<TtfGlyph>& output) noexcept {

  for (const auto& subtable : cmapTable.subtables) {
    auto reader = Reader{raw.begin() + subtable.offset, raw.end()};
    if (reader.getRemainingCount() < 2U) {
      continue; // Not enough data remaining for a format id.
    }
    switch (reader.consumeUInt<2U>()) {
    case 4U:
      const auto size = reader.consumeUIntChecked<2>();
      return readCmapFormat4(raw, subtable.offset + 4U, size, output);
    }
  }
  return false;
}

/* Get the offsets and sizes in the file for all glyphs.
 * Returns mapping of glyph index to its [offset, size] in the file, size can be zero if there is no
 * data for that glyph index.
 */
[[nodiscard]] auto readGlyphDataPositions(
    const math::RawData& raw,
    const TtfTableRecordMap& recordMap,
    const TtfHeadTable& headTable,
    const TtfMaxpTable& maxExpTable,
    std::vector<TtfGlyph>& output) noexcept {

  const auto locaItr = recordMap.find("loca");
  if (locaItr == recordMap.end()) {
    return false;
  }
  const auto locaOffset = locaItr->second.offset;
  const auto locaLength = locaItr->second.length;

  const auto glyfItr = recordMap.find("glyf");
  if (locaItr == recordMap.end()) {
    return false;
  }

  auto reader = Reader{raw.begin() + locaOffset, raw.begin() + locaOffset + locaLength};

  const auto numGlyphs = maxExpTable.numGlyphs;
  auto offsets         = math::PodVector<size_t>(numGlyphs + 1U); // +1 for the end offset.
  if (headTable.indexToLocFormat == 1U) {
    /* Long version of the loca table (32 bit offsets).
     */
    if (locaLength != offsets.size() * 4U) {
      // Not enough space to have offsets for all glyphs.
      return false;
    }
    for (auto i = 0U; i != offsets.size(); ++i) {
      offsets[i] = glyfItr->second.offset + reader.consumeUInt<4>();
    }

  } else {
    /* Short version of the loca table (16 bit offsets divided by two).
     */
    if (locaLength != offsets.size() * 2U) {
      // Not enough space to have offsets for all glyphs.
      return false;
    }
    for (auto i = 0U; i != offsets.size(); ++i) {
      offsets[i] = glyfItr->second.offset + reader.consumeUInt<2>() * 2U;
    }
  }

  /* Calculate size based on the difference between this entries offset and the next, there is a
   * extra end offset after the last entry.
   */
  for (auto i = 0U; i != numGlyphs; ++i) {
    assert(offsets[i] <= glyfItr->second.offset + glyfItr->second.length);
    output[i].dataOffset = offsets[i];
    output[i].dataSize   = offsets[i + 1U] - offsets[i];
  }
  return true;
}

/* Calculate the amount of points in a glyph. Takes the implicit points that 'close' contours and
 * the implicit onCurve point between two control points into account.
 */
[[nodiscard]] auto
getGlyphPointCount(const math::PodVector<uint8_t>& flags, uint16_t numContours) noexcept {
  auto count = 1U + numContours;
  for (auto i = 1U; i != flags.size(); ++i) {
    const auto onCurveA = flags[i - 1U] & 0x01;
    const auto onCurveB = flags[i - 0U] & 0x01;
    count += 1U + (!onCurveA && !onCurveB);
  }
  return count;
}

/* Construct a glyph out of the ttf data. This will decode the lines and quadratic beziers and make
 * all implicit points explicit.
 */
[[nodiscard]] auto buildGlyph(
    const math::PodVector<uint8_t>& ttfFlags,
    const math::PodVector<uint16_t>& ttfContourEndPoints,
    const math::PodVector<math::Vec2f>& ttfPoints,
    TtfGlyph& output) noexcept {

  auto pointCount = getGlyphPointCount(ttfFlags, ttfContourEndPoints.size());
  output.points.reserve(pointCount);

  for (auto c = 0U; c != ttfContourEndPoints.size(); ++c) {
    const auto cStart = c == 0U ? 0U : ttfContourEndPoints[c - 1U];
    const auto cEnd   = ttfContourEndPoints[c];
    if ((cEnd - cStart) < 2U) {
      continue; // Not enough points in this contour to form a segment.
    }

    // TODO(bastian): Check if the TTF spec enforces the first point to be 'onCurve'.
    assert(ttfFlags[cStart] & 0x01); // Assume 'onCurve'.
    output.points.push_back(ttfPoints[cStart]);
    for (auto i = cStart; i != cEnd; ++i) {
      const auto cur  = i;
      const auto next = (i + 1U) == cEnd ? cStart : i + 1U; // Wraps around for the last entry.

      const auto curOnCurve  = ttfFlags[cur] & 0x01;
      const auto nextOnCurve = ttfFlags[next] & 0x01;

      if (nextOnCurve) {
        /* Next is a point on the curve.
         * If the current is also on the curve then there is a straight line between them.
         * Otherwise this point 'finishes' the previous curve.
         */
        if (curOnCurve) {
          output.segments.emplace_back(GlyphSegmentType::Line, output.points.size() - 1U);
        }
      } else {
        /* Next is a control point.
         * If the current is also a control point we synthesize the implicit 'on curve' point to
         * finish the previous curve.
         */
        if (!curOnCurve) {
          output.points.push_back((ttfPoints[cur] + ttfPoints[next]) * .5f);
        }
        output.segments.emplace_back(GlyphSegmentType::QuadraticBezier, output.points.size() - 1U);
        assert(i + 1U < cEnd); // One additional point has to follow this to 'finnish' the curve.
      }

      output.points.push_back(ttfPoints[next]);
    }
  }

  assert(output.points.size() == pointCount);
  return true;
}

[[nodiscard]] auto readGlyphFlags(Reader& reader, const uint16_t count) noexcept
    -> math::PodVector<uint8_t> {
  auto flags = math::PodVector<uint8_t>(count);
  for (auto i = 0U; i != count;) {
    const auto f          = reader.consumeUIntChecked<1>();
    const auto isRepeated = f & 0x08;
    auto repeatCount      = isRepeated ? reader.consumeUIntChecked<1>() + 1U : 1U;
    while (repeatCount--) {
      flags[i++] = f;
    }
  }
  return flags;
}

[[nodiscard]] auto readGlyphSimple(
    Reader& reader,
    TtfGlyphHeader header,
    math::Box<int16_t, 2> bounds,
    TtfGlyph& output) noexcept {

  // Read contour data.
  if (reader.getRemainingCount() < header.numContours * 2U) {
    return false; // Not enough data for contour endpoints.
  }
  auto contourEndPoints = math::PodVector<uint16_t>(header.numContours);
  for (auto i = 0U; i != contourEndPoints.size(); ++i) {
    // +1 because 'end' meaning one past the last one is more idiomatic c++.
    contourEndPoints[i] = reader.consumeUInt<2>() + 1U;
  }

  // Skip over ttf instruction byte code for hinting, we do not support it.
  if (reader.getRemainingCount() < 2U) {
    return false; // Not enough data for instruction length;
  }
  const auto instructionLength = reader.consumeUInt<2>();
  if (!reader.skip(instructionLength)) {
    return false; // Not enough data for instructions;
  }

  auto numPoints = contourEndPoints.empty() ? 0U : contourEndPoints.back();
  auto flags     = readGlyphFlags(reader, numPoints);
  auto points    = math::PodVector<math::Vec2f>(numPoints);

  // Read the x coordinates for all points.
  auto xPos = 0;
  for (auto i = 0U; i != points.size(); ++i) {
    const auto isShort = flags[i] & 0x02;
    if (isShort) {
      xPos += reader.consumeUIntChecked<1>() * ((flags[i] & 0x10) ? 1 : -1);
    } else {
      xPos += (flags[i] & 0x10) ? 0 : reader.consumeIntChecked<2>();
    }
    points[i].x() = math::unlerp(
        static_cast<float>(bounds.min().x()),
        static_cast<float>(bounds.max().x()),
        static_cast<float>(xPos));
  }

  // Read the y coordinates for all points.
  auto yPos = 0;
  for (auto i = 0U; i != points.size(); ++i) {
    const auto isShort = flags[i] & 0x04;
    if (isShort) {
      yPos += reader.consumeUIntChecked<1>() * ((flags[i] & 0x20) ? 1 : -1);
    } else {
      yPos += (flags[i] & 0x20) ? 0 : reader.consumeIntChecked<2>();
    }
    points[i].y() = math::unlerp(
        static_cast<float>(bounds.min().y()),
        static_cast<float>(bounds.max().y()),
        static_cast<float>(yPos));
  }

  return buildGlyph(flags, contourEndPoints, points, output);
}

[[nodiscard]] auto
readGlyph(const math::RawData& raw, math::Box<int16_t, 2> bounds, TtfGlyph& output) noexcept {

  auto reader =
      Reader{raw.begin() + output.dataOffset, raw.begin() + output.dataOffset + output.dataSize};
  if (reader.getRemainingCount() < 10U) {
    return false; // Not enough space for a glyph header.
  }

  TtfGlyphHeader header;
  header.numContours           = reader.consumeInt<2>();
  header.glyphBounds.min().x() = reader.consumeInt<2>();
  header.glyphBounds.min().y() = reader.consumeInt<2>();
  header.glyphBounds.max().x() = reader.consumeInt<2>();
  header.glyphBounds.max().y() = reader.consumeInt<2>();

  if (header.numContours == 0) {
    // Glyphs with no contours are valid, for example a space character glyph.
    return true;
  }

  if (header.numContours < 0) {
    // Composite glyphs (negative numContours) are not supported at this time.
    return false;
  }

  return readGlyphSimple(reader, header, bounds, output);
}

} // namespace

auto loadFontTtf(log::Logger* logger, DatabaseImpl* /*unused*/, AssetId id, math::RawData raw)
    -> AssetUnique {

  auto reader               = Reader{raw.begin(), raw.end()};
  const auto offsetTableOpt = readTtfOffsetTable(reader);
  if (!offsetTableOpt) {
    throw err::FontTtfErr{"Invalid offsets table"};
  }
  if (offsetTableOpt->sfntVersion != 0x10000) {
    throw err::FontTtfErr{"Unsupported sfntVersion: Only TrueType outlines are supported"};
  }
  if (!validateTtfFile(raw, offsetTableOpt->records)) {
    throw err::FontTtfErr{"Malformed ttf file"};
  }
  const auto headTableOpt = readHeadTable(raw, offsetTableOpt->records);
  if (!headTableOpt || headTableOpt->magicNumber != 0x5F0F3CF5) {
    throw err::FontTtfErr{"Invalid head table"};
  }
  if (headTableOpt->majorVersion != 1U && headTableOpt->majorVersion != 0U) {
    throw err::FontTtfErr{"Unsupported head table version"};
  }
  if (headTableOpt->fontDirectionHint != 2U) {
    throw err::FontTtfErr{"fontDirectionHint is deprecated"};
  }
  const auto maxpTableOpt = readMaxpTable(raw, offsetTableOpt->records);
  if (!maxpTableOpt) {
    throw err::FontTtfErr{"Invalid maxp table"};
  }

  auto glyphData = std::vector<TtfGlyph>(maxpTableOpt->numGlyphs);

  const auto cmapTableOpt = readCmap(raw, offsetTableOpt->records);
  if (!cmapTableOpt) {
    throw err::FontTtfErr{"Invalid cmap table"};
  }
  if (!readCodepoints(raw, *cmapTableOpt, glyphData)) {
    throw err::FontTtfErr{"Unable to read codepoints (no supported cmap encoding?)"};
  }

  if (!readGlyphDataPositions(
          raw, offsetTableOpt->records, *headTableOpt, *maxpTableOpt, glyphData)) {
    throw err::FontTtfErr{"Unable to locate glyph data"};
  }
  for (auto i = 0U; i != glyphData.size(); ++i) {
    if (glyphData[i].dataSize == 0U) {
      continue; // Glyphs without data are valid, for example a space character glyph.
    }

    if (!readGlyph(raw, headTableOpt->glyphBounds, glyphData[i])) {
      LOG_W(
          logger,
          "Failed to read glyph",
          {"glyphId", i},
          {"codepoints", glyphData[i].codepoints},
          {"dataOffset", glyphData[i].dataOffset},
          {"dataSize", glyphData[i].dataSize});
    }
  }

  // Copy the glyph-data to the final (immutable) glyph structure.
  auto glyphs = std::vector<Glyph>{};
  glyphs.reserve(glyphData.size());
  for (auto i = 0U; i != glyphData.size(); ++i) {
    assert(glyphData[i].codepoints.size() < 2);
    glyphs.push_back(Glyph{std::move(glyphData[i].codepoints),
                           std::move(glyphData[i].points),
                           std::move(glyphData[i].segments)});
  }
  return std::make_unique<Font>(std::move(id), std::move(glyphs));
}

} // namespace tria::asset::internal
