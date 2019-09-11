#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Color;

extern Color color_palette[16];

uint8_t color_find_closest(int r, int g, int b);

#endif // COLOR_H
