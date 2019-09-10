#include "screen.hpp"

#include "vox.h"

#include <algorithm>

SDL_Rect screen_calc_rect(int width, int height) {
  int w = 1;
  int m = std::min(width, height);

  if (m < VOX_WIDTH) {
    int n = 6;
    while ((1 << n) > m) {
      n--;
    }
    w = 1 << n;
  } else {
    m -= VOX_PADDING;
    int n = 1;
    while (n * VOX_WIDTH < m) {
      n++;
    }
    w = (n - 1) * VOX_WIDTH;
  }
  return { (width - w) / 2, (height - w) / 2, w, w };
}
