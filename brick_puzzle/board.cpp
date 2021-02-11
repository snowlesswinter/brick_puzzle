#include <iostream>
#include "board.h"

using std::array;
using std::cout;
using std::endl;
using std::pair;

bool Board::MarkImpl(const Location& pos, const array<array<int, PAT_SIZE>, PAT_SIZE>& pat, const pair<int, int>& size,
    int id, decltype(Board::board_)* bo)
{
  if (pos.x + size.first > BOARD_SIZE || pos.y + size.second > BOARD_SIZE)
    return false;

  for (int i = 0; i < size.second; i++) {
    for (int j = 0; j < size.first; j++) {
      if (!pat[i][j])
        continue;

      if ((*bo)[pos.y + i][pos.x + j] != 0) // Occupied or out of scope.
        return false;
    }
  }

  for (int i = 0; i < size.second; i++) {
    for (int j = 0; j < size.first; j++) {
      if (pat[i][j])
        (*bo)[pos.y + i][pos.x + j] = id;
    }
  }

  return true;
}

void Board::PrintImpl(const decltype(Board::board_)& bo, const decltype(Board::mono_)& mono)
{
  mono.PrintLine(0);
  cout << endl;

  int l = 0;
  for (auto i = bo.begin(); i != bo.end(); i++) {
    mono.PrintLine(++l);
    cout << "                     ";

    for (auto j = i->begin(); j != i->end(); j++) {
      char c = (*j > -1 ? (*j > 0 ? ('0' + *j) : 'o') : ' ');
      cout << c;
    }

    cout << endl;
  }

  mono.PrintLine(l + 1);
  cout << endl;
}

void Board::Init()
{
  Monochrome::Init(BOARD_SIZE);
}

std::vector<Location> Board::SplitRegion(int n)
{
  std::vector<Location> v;
  int total = (BOARD_SIZE + 1) * BOARD_SIZE / 2;
  float piece_size = static_cast<float>(total) / n;
  int c = 0;
  v.push_back(Location(0, 0));

  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      if (c >= piece_size) {
        c = 0;
        v.push_back(Location(j, i));
      }

      if (j >= i)
        c++;
    }
  }

  v.push_back(Location(BOARD_SIZE - 1, BOARD_SIZE - 1));
  return v;
}

Board::Board() : board_(), mono_()
{
  for (size_t i = 0; i < BOARD_SIZE; i++)
    for (size_t j = 0; j < BOARD_SIZE; j++)
      board_[i][j] = j >= i ? 0 : -1;

  mono_.Mark(Location(0, 0), board_, std::pair<uint32_t, uint32_t>(BOARD_SIZE, BOARD_SIZE));
}

void Board::Print()
{
  PrintImpl(board_, mono_);
}

Location Board::GetNextVacancy(const Location& from)
{
  if (from.x < 0 || from.y < 0)
    return Location(0, 0);

  for (int x = from.x + 1; x < BOARD_SIZE; x++)
    if (!board_[from.y][x])
      return Location(x, from.y);

  for (int y = from.y + 1; y < BOARD_SIZE; y++) {
    for (int x = 0; x < BOARD_SIZE; x++) {
      if (!board_[y][x])
        return Location(x, y);
    }
  }
  return Location();
}

Location Board::GetNextVacancy(const Location& from, const Location& to)
{
  if (from.x < 0 || from.y < 0)
    return Location(0, 0);

  for (int x = from.x + 1; x < BOARD_SIZE; x++) {
    if (from.y > to.y && from.x > to.x)
      return Location();

    if (!board_[from.y][x])
      return Location(x, from.y);
  }

  for (int y = from.y + 1; y < BOARD_SIZE; y++) {
    for (int x = 0; x < BOARD_SIZE; x++) {
      if (from.y > to.y && from.x > to.x)
        return Location();

      if (!board_[y][x])
        return Location(x, y);
    }
  }
  return Location();
}

bool Board::Mark(const Location& pos, const array<array<int, PAT_SIZE>, PAT_SIZE>& pat,
    const array<uint32_t, PAT_SIZE>& mono, const pair<int, int>& size, int id)
{
  if (mono_.IsOverlapped(pos, pat, mono, size))
    return false;

  mono_.Mark(pos, pat, size);
  if (mono_.DetectBadPattern(pos, size)) {
    mono_.Unmark(pos, pat, size);
    return false;
  }
  
  MarkImpl(pos, pat, size, id, &board_);
  return true;
}