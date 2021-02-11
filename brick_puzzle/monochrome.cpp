#include "monochrome.h"

#include <iostream>
#include <limits>
#include <type_traits>
#include <unordered_map>

using std::array;
using std::cout;
using std::endl;
using std::unordered_map;
using std::vector;

namespace
{
const uint32_t kBorderSize = 1;
const uint32_t kNumCodeTypes = 3;
const uint32_t kMaxPatternWidth = 5;
const uint32_t kMaxPatternHeight = 5;

typedef uint32_t CodeUnit;

static unordered_map<CodeUnit, CodeUnit>* s_bad_codes = nullptr;
static uint32_t s_board_width = 1;

// 'x' is the offset from the left border.
template <typename Pattern>
void BuildCodeLine(const Pattern& bad_pattern, const Pattern& mask, uint32_t x,
    uint32_t y, CodeUnit code)
{
  if (y >= kMaxPatternHeight) {
    s_bad_codes->insert({code, code});
    return;
  }

  uint32_t p = 0;
  uint32_t m = 0;
  if (y < bad_pattern.size()) {
    p = bad_pattern[y] >> (32 - kMaxPatternWidth); // Align with board(lower bytes).
    m = mask[y] >> (32 - kMaxPatternWidth);
  }

  uint32_t max_u = (1 << kMaxPatternWidth) - 1;
  for (uint32_t u = 0; u <= max_u; u++) {
    uint32_t patterned_u = (u & ~(m >> x)) | (p >> x);
    CodeUnit merged_code = (code << kMaxPatternWidth) | patterned_u;
    BuildCodeLine(bad_pattern, mask, x, y + 1, merged_code);
  }
}

template <typename Pattern>
void UpdateBadPatternLookupTable(CodeUnit* code_pool, uint32_t pool_size,
    const Pattern& bad_pattern, const Pattern& mask, uint32_t pattern_width)
{
  if (!s_bad_codes)
    s_bad_codes = new std::remove_pointer<decltype(s_bad_codes)>::type();

  // Build merged pattern/mask into integers.
  CodeUnit merged_pattern = 0;
  CodeUnit merged_mask = 0;
  for (uint32_t i = 0; i < kMaxPatternHeight; i++) {
    if (i < bad_pattern.size()) {
      merged_pattern |= (bad_pattern[i] >> (32 - kMaxPatternWidth));
      merged_mask |= (mask[i] >> (32 - kMaxPatternWidth));
    }

    if (i < kMaxPatternHeight - 1) { // No shifting in the last pattern.
      merged_pattern <<= kMaxPatternWidth;
      merged_mask <<= kMaxPatternWidth;
    }
  }

  // Combine the pattern into each possible code.
  for (uint32_t i = 0; i < pool_size; i++) {
    for (uint32_t y = 0; y < kMaxPatternHeight - bad_pattern.size() + 1; y++) {
      for (uint32_t x = 0; x < kMaxPatternWidth - pattern_width + 1; x++) {
        CodeUnit p = merged_pattern >> (y * kMaxPatternWidth + x);
        CodeUnit m = merged_mask >> (y * kMaxPatternWidth + x);
        CodeUnit code = (code_pool[i] & ~m) | p;
        s_bad_codes->insert({code, code});
      }
    }
  }
}

void InitBadPatternLookupTable() {
  const array<uint32_t, 3> bpat1 = { 0x40000000,0xA0000000,0x40000000 };
  const array<uint32_t, 3> mask1 = { 0x40000000,0xE0000000,0x40000000 };
  uint32_t width1 = 3;
  const array<uint32_t, 3> bpat2 = { 0x60000000,0x90000000,0x60000000 };
  const array<uint32_t, 3> mask2 = { 0x60000000,0xF0000000,0x60000000 };
  uint32_t width2 = 4;
  const array<uint32_t, 3> bpat3 = { 0x70000000,0x88000000,0x70000000 };
  const array<uint32_t, 3> mask3 = { 0x70000000,0xF8000000,0x70000000 };
  uint32_t width3 = 5;
  const array<uint32_t, 4> bpat4 = { 0x40000000,0xA0000000,0xA0000000,0x40000000 };
  const array<uint32_t, 4> mask4 = { 0x40000000,0xE0000000,0xE0000000,0x40000000 };
  uint32_t width4 = 3;
  const array<uint32_t, 5> bpat5 = { 0x40000000,0xA0000000,0xA0000000,0xA0000000,0x40000000 };
  const array<uint32_t, 5> mask5 = { 0x40000000,0xE0000000,0xE0000000,0xE0000000,0x40000000 };
  uint32_t width5 = 3;

  cout << "Initializing bad pattern lookup table.." << endl;

  uint32_t pool_size = 1 << (kMaxPatternWidth * kMaxPatternWidth);
  CodeUnit* code_pool = new CodeUnit[pool_size];
  for (uint32_t i = 0; i < pool_size; i++)
    code_pool[i] = i;

  UpdateBadPatternLookupTable(code_pool, pool_size, bpat1, mask1, width1);
  UpdateBadPatternLookupTable(code_pool, pool_size, bpat2, mask2, width2);
  UpdateBadPatternLookupTable(code_pool, pool_size, bpat3, mask3, width3);
  UpdateBadPatternLookupTable(code_pool, pool_size, bpat4, mask4, width4);
  UpdateBadPatternLookupTable(code_pool, pool_size, bpat5, mask5, width5);

  delete[] code_pool;

  cout << "Finished building lookup table. " << s_bad_codes->size() << " codes are added" << endl;
}

bool LookupCode(CodeUnit code)
{
  auto r = s_bad_codes->find(code);
  for (auto i = r; i != s_bad_codes->end(); i++) {
    if (i->second == code) {
      return true;
    }
  }

  return false;
}

CodeUnit ConcatBlockCode(const std::vector<uint32_t>& lines, uint32_t y, uint32_t x)
{
  CodeUnit code = 0;
  for (uint32_t i = 0; i < kMaxPatternHeight; i++) {
    uint32_t u = lines[y + i];
    CodeUnit stripped_u = (u << x) >> (32 - kMaxPatternWidth);  // Stripped all but the content within target scope.
    code = (code << kMaxPatternWidth) | stripped_u;
  }

  return code;
}
}

void Monochrome::Init(uint32_t board_width) 
{
  s_board_width = board_width;
  //InitBadPatternLookupTable();
}

Monochrome::Monochrome()
  : lines_()
{
  lines_.resize(s_board_width + kBorderSize * 2);

  *lines_.begin() = std::numeric_limits<uint32_t>::max();
  *lines_.rbegin() = std::numeric_limits<uint32_t>::max();

  for (uint32_t y = 1; y < s_board_width + 1; y++) {
    lines_[y] = 0x80000000;
    for (uint32_t x = s_board_width + 1; x < sizeof(lines_[y]) * 8; x++) {
      lines_[y] |= 0x80000000 >> x;
    }
  }
}

void Monochrome::PrintLine(uint32_t y) const
{
  if (y >= lines_.size())
    return;

  uint32_t u = lines_[y];
  uint32_t this_width = s_board_width + kBorderSize * 2;
  for (uint32_t x = 0; x < this_width; x++) {
    char c = ((0x80000000 >> x) & u) ? '*' : ' ';
    if (y > x)
      c = '+';

    if (y == 0 || y == (lines_.size() - 1) || x == 0 || x == (this_width - 1))
      c = '%';

    cout << c;
  }
}

bool Monochrome::HitWalls(const Location& pos, const  std::pair<uint32_t, uint32_t>& size)
{
  return pos.x + size.first > s_board_width || pos.y + size.second > s_board_width;
}

template <typename Array>
bool Monochrome::Match1(const Array& bad_pattern, const Array& mask, uint32_t width) const
{
  uint32_t this_width = s_board_width + kBorderSize * 2;
  for (uint32_t y = 0; y < lines_.size() - bad_pattern.size() + 1; y++) {
    for (uint32_t x = 0; x < this_width - width + 1; x++) {
      uint32_t shifted_mask = mask[0] >> x;
      uint32_t masked_u = lines_[y] & shifted_mask;
      if (!(masked_u ^ (bad_pattern[0] >> x))) { // Matched 1st line.
        bool matched = true;
        for (uint32_t i = 1; i < bad_pattern.size(); i++) {
          shifted_mask = mask[i] >> x;
          masked_u = lines_[y + i] & shifted_mask;
          if (masked_u ^ (bad_pattern[i] >> x)) {
            matched = false;
            break;
          }
        }

        if (matched)
          return true;
      }
    }
  }

  return false;
}

// A faster version of pattern recognition.
// We try to flatten the data so as to reduce the times of comparison and the number of code branches.
// The pattern will be ogonzized as "xxx______xxx______xxx______" and the data will be flatten in the same way.
// Basically all the calculation in this process are bitwise operations, so it's time efficient.
template <typename Array>
bool Monochrome::Match2(const Array& bad_pattern, const Array& mask, uint32_t pattern_width) const
{
  uint32_t this_width = s_board_width + kBorderSize * 2;
  uint32_t pattern_height = static_cast<uint32_t>(bad_pattern.size());

  uint64_t flattened_p = bad_pattern[0] >> (32 - this_width);
  uint64_t flattened_m = mask[0] >> (32 - this_width);
  uint64_t flattened_u = lines_[0] >> (32 - this_width);
  for (uint32_t i = 1; i < pattern_height; i++) {
    flattened_p <<= this_width;
    flattened_m <<= this_width;
    flattened_u <<= this_width;

    flattened_p |= (bad_pattern[i] >> (32 - this_width));
    flattened_m |= (mask[i] >> (32 - this_width));
    flattened_u |= (lines_[i] >> (32 - this_width));
  }

  for (uint32_t x = 0; x < this_width - pattern_width + 1; x++) {
    uint64_t masked_u = (flattened_u << x) & flattened_m;
    if (!(masked_u ^ flattened_p))
      return true;
  }

  for (uint32_t y = pattern_height; y < lines_.size(); y++) {
    flattened_u <<= this_width;
    flattened_u |= (lines_[y] >> (32 - this_width));
    for (uint32_t x = 0; x < this_width - pattern_width + 1; x++) {
      uint64_t masked_u = (flattened_u << x) & flattened_m;
      if (!(masked_u ^ flattened_p))
        return true;
    }
  }

  return false;
}

bool Monochrome::DetectBadPattern1(const Location& pos, const std::pair<uint32_t, uint32_t>& size) const
{
  const array<uint32_t, 3> bpat1 = { 0x40000000,0xA0000000,0x40000000 };
  const array<uint32_t, 3> mask1 = { 0x40000000,0xE0000000,0x40000000 };
  uint32_t width1 = 3;
  const array<uint32_t, 3> bpat2 = { 0x60000000,0x90000000,0x60000000 };
  const array<uint32_t, 3> mask2 = { 0x60000000,0xF0000000,0x60000000 };
  uint32_t width2 = 4;
  const array<uint32_t, 3> bpat3 = { 0x70000000,0x88000000,0x70000000 };
  const array<uint32_t, 3> mask3 = { 0x70000000,0xF8000000,0x70000000 };
  uint32_t width3 = 5;
  const array<uint32_t, 4> bpat4 = { 0x40000000,0xA0000000,0xA0000000,0x40000000 };
  const array<uint32_t, 4> mask4 = { 0x40000000,0xE0000000,0xE0000000,0x40000000 };
  uint32_t width4 = 3;
  const array<uint32_t, 5> bpat5 = { 0x40000000,0xA0000000,0xA0000000,0xA0000000,0x40000000 };
  const array<uint32_t, 5> mask5 = { 0x40000000,0xE0000000,0xE0000000,0xE0000000,0x40000000 };
  uint32_t width5 = 3;

  if (Match2(bpat1, mask1, width1))
    return true;

  if (Match2(bpat2, mask2, width2))
    return true;

  if (Match2(bpat3, mask3, width3))
    return true;

  if (Match2(bpat4, mask4, width4))
    return true;

  if (Match2(bpat5, mask5, width5))
    return true;

  return false;
}

bool Monochrome::DetectBadPattern2(const Location& pos, const std::pair<uint32_t, uint32_t>& size) const
{
  uint32_t this_width = s_board_width + kBorderSize * 2;
  uint32_t x_step_size = kMaxPatternWidth - 2;
  uint32_t y_step_size = kMaxPatternHeight - 2;
  for (uint32_t y = 0; y < lines_.size() - kMaxPatternHeight + 1; y += y_step_size) {
    for (uint32_t x = 0; x < this_width - kMaxPatternWidth + 1; x += x_step_size) {
      CodeUnit code = ConcatBlockCode(lines_, y, x);
      if (LookupCode(code))
        return true;
    }

    CodeUnit code = ConcatBlockCode(lines_, y, this_width - kMaxPatternWidth);
    if (LookupCode(code))
      return true;
  }

  uint32_t y = static_cast<uint32_t>(lines_.size()) - kMaxPatternHeight;
  for (uint32_t x = 0; x < this_width - kMaxPatternWidth + 1; x += x_step_size) {
    CodeUnit code = ConcatBlockCode(lines_, y, x);
    if (LookupCode(code))
      return true;
  }

  CodeUnit code = ConcatBlockCode(lines_, y, this_width - kMaxPatternWidth);
  if (LookupCode(code))
    return true;

  return false;
}

bool Monochrome::DetectBadPattern(const Location& pos, const std::pair<uint32_t, uint32_t>& size) const
{
  if (s_bad_codes)
    return DetectBadPattern2(pos, size);

  return DetectBadPattern1(pos, size);
}