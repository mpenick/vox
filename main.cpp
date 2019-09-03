#include <SDL.h>

#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define VOX_WIDTH 128 // Must be a power of 2
#define VOX_PADDING 8

#define VOX_SPRITE_COUNT 512
#define VOX_SPRITE_WIDTH 8

namespace {

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Color;

Color palette[] = {
    {0, 0, 0},       // 0:  Black
    {29, 43, 83},    // 1:  Dark blue
    {126, 37, 83},   // 2:  Dark blue
    {0, 135, 81},    // 3:  Dark grean
    {171, 82, 54},   // 4:  Brown
    {95, 87, 79},    // 5:  Dark gray
    {194, 195, 199}, // 6:  Light gray
    {255, 241, 232}, // 7:  White
    {255, 0, 77},    // 8:  Red
    {255, 163, 0},   // 9:  Orange
    {255, 236, 39},  // 10: Yellow
    {0, 228, 54},    // 11: Green
    {41, 173, 255},  // 12: Blue
    {131, 118, 156}, // 13: Indigo
    {255, 119, 168}, // 14: Pink
    {255, 204, 170}, // 15: Peach
};

double update_rate = 60;
int64_t designed_frametime = SDL_GetPerformanceFrequency() / update_rate;

uint8_t screen[VOX_WIDTH][VOX_WIDTH];
uint8_t sprites[VOX_SPRITE_COUNT][VOX_SPRITE_WIDTH][VOX_SPRITE_WIDTH];

double color_distance(int r1, int g1, int b1, int r2, int g2, int b2) {
  double dr = static_cast<double>(r1 - r2) * 0.30;
  double dg = static_cast<double>(g1 - g2) * 0.59;
  double db = static_cast<double>(b1 - b2) * 0.11;
  return dr * dr + dg * dg + db * db;
}

uint8_t find_closest_color(int r, int g, int b) {
  uint8_t c = 0;
  double min_dist = color_distance(r, g, b, 0, 0, 0);
  for (uint8_t i = 1; i < 16; ++i) {
    Color p = palette[i];
    double dist = color_distance(r, g, b, p.r, p.g, p.b);
    if (dist < min_dist) {
      c = i;
      min_dist = dist;
    }
  }
  return c;
}

SDL_PixelFormat *best_pixel_format(const SDL_RendererInfo *renderer_info) {
  uint32_t format = renderer_info->texture_formats[0];
  for (uint32_t i = 0; i < renderer_info->num_texture_formats; ++i) {
    if (!SDL_ISPIXELFORMAT_FOURCC(renderer_info->texture_formats[i]) &&
        SDL_ISPIXELFORMAT_ALPHA(renderer_info->texture_formats[i])) {
      format = renderer_info->texture_formats[i];
      break;
    }
  }
  return SDL_AllocFormat(format);
}

SDL_Rect calc_screen_rect(int width, int height) {
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
  return {(width - w) / 2, (height - w) / 2, w, w};
}

uint32_t round_to_nearest_multiple(uint32_t value, uint32_t multiple) {
  return (value + (multiple - 1)) & ~(multiple - 1);
}

int import_sprites(const char *filename, int pos) {
  int w, h, n;
  uint8_t *data = stbi_load(filename, &w, &h, &n, 0);

  if (!data) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Unable to import sprites from '%s'", filename);
    return -1;
  }

  if (n != 3 && n != 4) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Unable to handle byte formate of '%s'", filename);
    return -1;
  }

  int sw = w / 8;
  int sh = h / 8;

  for (int j = 0; j < sh; ++j) {
    int oy = n * VOX_SPRITE_WIDTH * w * j;
    for (int i = 0; i < sw; ++i) {
      int ox = n * VOX_SPRITE_WIDTH * i;
      for (int y = 0; y < VOX_SPRITE_WIDTH; y++) {
        for (int x = 0; x < VOX_SPRITE_WIDTH; x++) {
          uint8_t *p = &data[ox + oy + n * y * w + n * x];
          sprites[pos][x][y] = find_closest_color(p[0], p[1], p[2]);
        }
      }
      pos++;
      assert(pos < VOX_SPRITE_COUNT && "Using too many sprites");
    }
  }

  return pos;
}

void cls(uint8_t col = 0) {
  for (int i = 0; i < VOX_WIDTH; ++i) {
    for (int j = 0; j < VOX_WIDTH; ++j) {
      screen[i][j] = col;
    }
  }
}

void spr(int n, int x, int y, int w = VOX_SPRITE_WIDTH,
         int h = VOX_SPRITE_WIDTH) {
  for (int i = 0; i < w; ++i) {
    for (int j = 0; j < h; ++j) {
      screen[x + i][y + j] = sprites[n][i][j];
    }
  }
}

void print(const char *str, int x, int y) {
  for (const char *c = str; *c; ++c) {
    spr(*c, x, y);
    x += VOX_SPRITE_WIDTH / 2;
  }
}

} // namespace

int main(int argc, char *argv[]) {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_RendererInfo renderer_info;
  SDL_Texture *texture;
  SDL_Event event;

  int32_t width = 800, height = 800;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize video: %s",
                 SDL_GetError());
    return 3;
  }

  if (SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_RESIZABLE, &window,
                                  &renderer)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Couldn't create window and renderer: %s", SDL_GetError());
    return 3;
  }

  if (SDL_GetRendererInfo(renderer, &renderer_info)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't get renderer info: %s",
                 SDL_GetError());
    return 3;
  }

  SDL_PixelFormat *pixel_format = best_pixel_format(&renderer_info);

  SDL_Log("Using pixel format: %s",
          SDL_GetPixelFormatName(pixel_format->format));

  int pos = 0;
  pos = import_sprites("pico8_font.png", pos);
  import_sprites("sprites1.png", pos);

  texture = SDL_CreateTexture(renderer, pixel_format->format,
                              SDL_TEXTUREACCESS_STATIC, VOX_WIDTH, VOX_WIDTH);

  // TODO: Allocate properly based on the best format
  uint32_t *pixels = new uint32_t[VOX_WIDTH * VOX_WIDTH];

  memset(pixels, 255, VOX_WIDTH * VOX_WIDTH * sizeof(uint32_t));

  SDL_Rect screen_rect = calc_screen_rect(width, height);

  cls();
  print("hi my name is mike", 64, 0);
  spr(pos + 80, 0, 0);

  while (1) {
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT) {
      break;
    }

    if (event.type == SDL_WINDOWEVENT) {
      switch (event.window.event) {
      case SDL_WINDOWEVENT_RESIZED:
      case SDL_WINDOWEVENT_SIZE_CHANGED:
        screen_rect = calc_screen_rect(event.window.data1, event.window.data2);
        break;
      }
    }

    for (int x = 0; x < VOX_WIDTH; ++x) {
      for (int y = 0; y < VOX_WIDTH; ++y) {
        Color color = palette[screen[x][y]];
        pixels[y * VOX_WIDTH + x] =
            SDL_MapRGBA(pixel_format, color.r, color.g, color.b, 255);
      }
    }

    SDL_UpdateTexture(texture, NULL, pixels, VOX_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, &screen_rect);
    SDL_RenderPresent(renderer);
  }
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  return 0;
}
