#include "color.h"

Color color_palette[] = {
  { 0, 0, 0 },       // 0:  Black
  { 29, 43, 83 },    // 1:  Dark blue
  { 126, 37, 83 },   // 2:  Dark blue
  { 0, 135, 81 },    // 3:  Dark grean
  { 171, 82, 54 },   // 4:  Brown
  { 95, 87, 79 },    // 5:  Dark gray
  { 194, 195, 199 }, // 6:  Light gray
  { 255, 241, 232 }, // 7:  White
  { 255, 0, 77 },    // 8:  Red
  { 255, 163, 0 },   // 9:  Orange
  { 255, 236, 39 },  // 10: Yellow
  { 0, 228, 54 },    // 11: Green
  { 41, 173, 255 },  // 12: Blue
  { 131, 118, 156 }, // 13: Indigo
  { 255, 119, 168 }, // 14: Pink
  { 255, 204, 170 }, // 15: Peach
};

static double color_distance(int r1, int g1, int b1, int r2, int g2, int b2) {
  double dr = static_cast<double>(r1 - r2) * 0.30;
  double dg = static_cast<double>(g1 - g2) * 0.59;
  double db = static_cast<double>(b1 - b2) * 0.11;
  return dr * dr + dg * dg + db * db;
}

uint8_t color_find_closest(int r, int g, int b) {
  uint8_t c = 0;
  double min_dist = color_distance(r, g, b, 0, 0, 0);
  for (uint8_t i = 1; i < 16; ++i) {
    Color p = color_palette[i];
    double dist = color_distance(r, g, b, p.r, p.g, p.b);
    if (dist < min_dist) {
      c = i;
      min_dist = dist;
    }
  }
  return c;
}
