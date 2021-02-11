#include <array>
#include <iostream>
#include "brick.h"

#include "board.h"

using std::array;
using std::cout;
using std::endl;

void Brick::BuildMonochromes(Brick* b)
{
  for (int i = 0; i < b->pat_.size(); i++) {
    const auto& pat = b->pat_[i];
    const auto& pat_size = b->size_[i];
    for (int y = 0; y < pat_size.second; y++) {
      uint32_t v = 0;
      for (int x = 0; x < pat_size.first; x++) {
        if (pat[y][x])
          v |= (0x80000000 >> x);
      }

      b->mono_[i][y] = v;
    }
  }
}

void Brick::CompactVariants(Brick* b)
{
  int width = 0;
  int height = 0;
  for (int y = 0; y < PAT_SIZE; y++) {
    for (int x = 0; x < PAT_SIZE; x++) {
      if (b->pat_[0][y][x] > 0) {
        width = std::max(width, x + 1);
        height = std::max(height, y + 1);
      }
    }
  }

  int width_rotated = height;
  int height_rotated = width;

  array<array<array<int, PAT_SIZE>, PAT_SIZE>, NUM_VARIANTS> pat;
  for (int y = 0; y < PAT_SIZE; y++) {
    for (int x = 0; x < PAT_SIZE; x++) {
      int i = PAT_SIZE - height + y;
      int j = PAT_SIZE - width + x;
      pat[1][y][x] = (i >= PAT_SIZE || j >= PAT_SIZE) ? 0 : b->pat_[1][i][j];
      pat[2][y][x] = j >= PAT_SIZE ? 0 : b->pat_[2][y][j];
      pat[3][y][x] = i >= PAT_SIZE ? 0 : b->pat_[3][i][x];

      i = PAT_SIZE - height_rotated + y;
      j = PAT_SIZE - width_rotated + x;
      pat[4][y][x] = b->pat_[4][y][x];
      pat[5][y][x] = (i >= PAT_SIZE || j >= PAT_SIZE) ? 0 : b->pat_[5][i][j];
      pat[6][y][x] = j >= PAT_SIZE ? 0 : b->pat_[6][y][j];
      pat[7][y][x] = i >= PAT_SIZE ? 0 : b->pat_[7][i][x];
    }
  }

  for (int i = 0; i < 4; i++) {
    b->size_[i].first = width;
    b->size_[i].second = height;
    b->size_[4 + i].first = width_rotated;
    b->size_[4 + i].second = height_rotated;
  }

  pat[0] = b->pat_[0];
  b->pat_ = pat;
}

void Brick::MergeVariants(Brick* b)
{
  int n = static_cast<int>(b->pat_.size());
  int last_index = n - 1;
  for (int i = last_index; i > 0; i--) {
    for (int j = i - 1; j >= 0; j--) {
      if (b->pat_[j] == b->pat_[i]) {
        n--;
        std::swap(b->pat_[i], b->pat_[n]);
        std::swap(b->size_[i], b->size_[n]);

        break;
      }
    }
  }
  b->num_variants_ = n;
}

void Brick::InitBricks(std::array<Brick, 12>* bricks)
{
  int p1[PAT_SIZE][PAT_SIZE] = {{1, 0, 1, 0},
                                {1, 1, 1, 0},
                                {0, 0, 0, 0},
                                {0, 0, 0, 0 }};
  int p2[PAT_SIZE][PAT_SIZE] = {{1, 1, 0, 0},
                                {1, 1, 0, 0},
                                {0, 0, 0, 0},
                                {0, 0, 0, 0}};
  int p3[PAT_SIZE][PAT_SIZE] = {{1, 1, 0, 0},
                                {1, 0, 0, 0},
                                {0, 0, 0, 0},
                                {0, 0, 0, 0}};
  int p4[PAT_SIZE][PAT_SIZE] = {{1, 1, 1, 1},
                                {1, 0, 0, 0},
                                {0, 0, 0, 0},
                                {0, 0, 0, 0}};
  int p5[PAT_SIZE][PAT_SIZE] = {{1, 1, 0, 0},
                                {0, 1, 1, 0},
                                {0, 0, 1, 0},
                                {0, 0, 0, 0}};
  int p6[PAT_SIZE][PAT_SIZE] = {{1, 1, 1, 0},
                                {0, 0, 1, 0},
                                {0, 0, 1, 0},
                                {0, 0, 0, 0}};
  int p7[PAT_SIZE][PAT_SIZE] = {{1, 1, 0, 0},
                                {0, 1, 1, 1},
                                {0, 0, 0, 0},
                                {0, 0, 0, 0}};
  int p8[PAT_SIZE][PAT_SIZE] = {{1, 1, 1, 0},
                                {0, 1, 1, 0},
                                {0, 0, 0, 0},
                                {0, 0, 0, 0}};
  int p9[PAT_SIZE][PAT_SIZE] = {{1, 1, 1, 1},
                                {0, 0, 0, 0},
                                {0, 0, 0, 0},
                                {0, 0, 0, 0}};
  int p10[PAT_SIZE][PAT_SIZE] = {{1, 1, 1, 0},
                                 {1, 0, 0, 0},
                                 {0, 0, 0, 0},
                                 {0, 0, 0, 0}};
  int p11[PAT_SIZE][PAT_SIZE] = {{0, 1, 0, 0},
                                 {1, 1, 1, 0},
                                 {0, 1, 0, 0},
                                 {0, 0, 0, 0}};
  int p12[PAT_SIZE][PAT_SIZE] = {{1, 1, 1, 1},
                                 {0, 1, 0, 0},
                                 {0, 0, 0, 0},
                                 {0, 0, 0, 0}};

  array<decltype(p1)*, 12> all_p = { &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10, &p11, &p12 };
  int z = 0;
  for (auto it = all_p.begin(); it != all_p.end(); it++, z++) {
    for (int i = 0; i < PAT_SIZE; i++) {
      for (int j = 0; j < PAT_SIZE; j++) {
        (*bricks)[z].pat_[0][i][j] = (*(*it))[i][j];
        (*bricks)[z].pat_[1][i][j] = (*(*it))[PAT_SIZE - i - 1][PAT_SIZE - j - 1];
        (*bricks)[z].pat_[2][i][j] = (*(*it))[i][PAT_SIZE - j - 1];
        (*bricks)[z].pat_[3][i][j] = (*(*it))[PAT_SIZE - i - 1][j];
        (*bricks)[z].pat_[4][i][j] = (*(*it))[j][i];
        (*bricks)[z].pat_[5][i][j] = (*(*it))[PAT_SIZE - j - 1][PAT_SIZE - i - 1];
        (*bricks)[z].pat_[6][i][j] = (*(*it))[PAT_SIZE - j - 1][i];
        (*bricks)[z].pat_[7][i][j] = (*(*it))[j][PAT_SIZE - i - 1];
      }
    }
  }

  for (int n = 0; n < sizeof(all_p) / sizeof(all_p[0]); n++) {
    (*bricks)[n].id_ = n + 1;
    (*bricks)[n].num_variants_ = NUM_VARIANTS;
    CompactVariants(&(*bricks)[n]);
    MergeVariants(&(*bricks)[n]);
    BuildMonochromes(&(*bricks)[n]);
  }
}

Brick::Brick()
{
  id_ = -1;
  num_variants_ = 0;

  for (auto i = mono_.begin(); i != mono_.end(); i++)
    std::fill(i->begin(), i->end(), 0);
}

int Brick::TryOccupy(const Location& pos, int variant_id, Board* board) const
{
  for (int i = variant_id; i < num_variants_; i++) {
    if (board->Mark(pos, pat_[i], mono_[i], size_[i], id_))
      return i;
  }

  return -1;
}

void Brick::Print()
{
  cout << "Number of variants: " << num_variants_ << endl;

  for (auto s = size_.begin(); s != size_.end(); s++)
    cout << s->first << " x " << s->second << "           ";

  cout << endl;
  for (size_t i = 0; i < PAT_SIZE; i++) {
    for (auto p = pat_.begin(); p != pat_.end(); p++) {
      char flag = (p - pat_.begin()) >= num_variants_ ? 'o' : '*';
      for (size_t j = 0; j < PAT_SIZE; j++)
        cout << ((*p)[i][j] ? flag : ' ');

      cout << "            ";
    }

    cout << endl;
  }
}