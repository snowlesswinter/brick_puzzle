#pragma once
#include <array>
#include <vector>

#include "base.h"

class Monochrome
{
public:
  static void Init(uint32_t board_width);

  Monochrome();

  void PrintLine(uint32_t y) const;
  bool DetectBadPattern(const Location& pos, const std::pair<uint32_t, uint32_t>& size) const;

  template <typename MultiArray, bool Flip>
  void MarkImpl(const Location& pos, const MultiArray& pat, const std::pair<uint32_t, uint32_t>& size) {
    for (uint32_t y = 0; y < size.second; y++) {
      uint32_t& u = lines_[y + 1 + pos.y];
      uint32_t v = 0;
      for (uint32_t x = 0; x < size.first; x++) {
        if (pat[y][x])
          v |= (0x40000000 >> x);
      }
      v >>= pos.x;

      if (Flip)
        u &= (~v);
      else
        u |= v;
    }
  }
  template <typename MultiArray>
  void Mark(const Location& pos, const MultiArray& pat, const std::pair<uint32_t, uint32_t>& size) {
    MarkImpl<MultiArray, false>(pos, pat, size);
  }

  template <typename MultiArray>
  void Unmark(const Location& pos, const MultiArray& pat, const std::pair<uint32_t, uint32_t>& size) {
    MarkImpl<MultiArray, true>(pos, pat, size);
  }

  template <typename MultiArray>
  bool IsOverlapped(const Location& pos, const MultiArray& pat,
      const std::array<uint32_t, PAT_SIZE>& mono, const  std::pair<uint32_t, uint32_t>& size) const {
    if (HitWalls(pos, size))
      return true;

    for (uint32_t y = 0; y < size.second; y++) {
      uint32_t u = lines_[y + 1 + pos.y];
      uint32_t v = mono[y] >> (pos.x + 1); // Border.
      u &= ~0x801FFFFF; // Strip walls.
      if (((u ^ v) & u) != u)
        return true;
    }

    return false;
  }

private:
  static bool HitWalls(const Location& pos, const  std::pair<uint32_t, uint32_t>& size);

  bool DetectBadPattern1(const Location& pos, const std::pair<uint32_t, uint32_t>& size) const;
  bool DetectBadPattern2(const Location& pos, const std::pair<uint32_t, uint32_t>& size) const;

  template <typename Array>
  bool Match2(const Array& bad_pattern, const Array& mask, uint32_t width) const;

  std::vector<uint32_t> lines_;
};