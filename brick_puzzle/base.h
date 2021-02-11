#pragma once

static const size_t PAT_SIZE = 4;

struct Location
{
  int x;
  int y;

  Location() : x(-1), y(-1) {}
  Location(int x, int y) : x(x), y(y) {}

  bool Valid() {
    return x >= 0 && y >= 0;
  }
};