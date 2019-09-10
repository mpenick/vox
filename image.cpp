#include "image.h"

#include "color.h"
#include "vox.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <SDL_log.h>
#include <algorithm>

uint8_t* image_load(const char* filename) {
  int n, width, height;
  uint8_t* data = stbi_load(filename, &width, &height, &n, 0);

  if (!data) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to load image: %s", filename);
    return nullptr;
  }

  uint8_t* palettized = new uint8_t[VOX_SPRITES_WIDTH * VOX_SPRITES_WIDTH];

  memset(palettized, 0, VOX_SPRITES_WIDTH * VOX_SPRITES_WIDTH);

  int minw = std::min(VOX_SPRITES_WIDTH, width);
  int minh = std::min(VOX_SPRITES_WIDTH, height);

  for (int i = 0; i < minw; ++i) {
    for (int j = 0; j < minh; ++j) {
      uint8_t* p = &data[j * width * n + i * n];
      palettized[j * VOX_SPRITES_WIDTH + i] = color_find_closest(p[0], p[1], p[2]);
    }
  }

  stbi_image_free(data);

  return palettized;
}
