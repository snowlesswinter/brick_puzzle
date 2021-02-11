#pragma once

#include <array>
#include "base.h"
#include "monochrome.h"

class Board
{
private:
  static const uint32_t BOARD_SIZE = 10;
  std::array<std::array<int, BOARD_SIZE>, BOARD_SIZE> board_;
  Monochrome mono_;

  static bool DetectBadPatterns(const decltype(Board::board_)& bo);
  static bool MarkImpl(const Location& pos, const std::array<std::array<int, PAT_SIZE>, PAT_SIZE>& pat,
      const std::pair<int, int>& size, int id, decltype(Board::board_)* bo);
  static void PrintImpl(const decltype(Board::board_)& bo, const decltype(Board::mono_)& mono);

public:
  static void Init();
  static std::vector<Location> SplitRegion(int n);

  Board();

  void Print();
  
  Location GetNextVacancy(const Location& from);
  Location GetNextVacancy(const Location& from, const Location& to);
  bool Mark(const Location& pos, const std::array<std::array<int, PAT_SIZE>, PAT_SIZE>& pat,
      const std::array<uint32_t, PAT_SIZE>& mono, const std::pair<int, int>& size, int id);
};