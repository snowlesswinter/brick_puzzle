#pragma once

#include "base.h"

class Board;
class Brick
{
private:
  static void BuildMonochromes(Brick* b);
  static void CompactVariants(Brick* b);
  static void MergeVariants(Brick* b);

  static const size_t NUM_VARIANTS = 8;
  std::array<std::array<std::array<int, PAT_SIZE>, PAT_SIZE>, NUM_VARIANTS> pat_;
  std::array<std::array<uint32_t, PAT_SIZE>, NUM_VARIANTS> mono_;
  std::array<std::pair<int, int>, NUM_VARIANTS> size_; // Actual size of the pattern.
  int id_;
  int num_variants_;

public:
  static void InitBricks(std::array<Brick, 12>* bricks);

  Brick();
  int TryOccupy(const Location& pos, int variant_id, Board* board) const;
  void Print();
};
