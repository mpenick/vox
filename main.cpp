#include "color.h"
#include "image.h"
#include "lines.h"
#include "screen.hpp"
#include "shader.h"
#include "sprites.h"
#include "vox.h"

#include <SDL.h>

#include <epoxy/gl.h>

#include <glm/glm.hpp>

static Sprites sprites;
static Quads lines;
static SDL_Rect screen_rect;

bool init() { return sprites_init(&sprites); }

void flush() { sprites_flush(&sprites); }

void cls(int c = 0) {
  glClearColor(shader_palette[c].r, shader_palette[c].g, shader_palette[c].b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glScissor(screen_rect.x, screen_rect.y, screen_rect.w, screen_rect.h);
}

void pal() {
  flush();
  for (int i = 0; i < 16; ++i) {
    shader_color_map[i] = i;
  }
}

void pal(uint8_t c0, uint8_t c1) {
  flush();
  shader_color_map[c0] = c1;
}

void palt() {
  flush();
  memset(shader_alpha_map, 0, sizeof(shader_alpha_map));
  shader_alpha_map[0] = 1;
}

void palt(int c, bool t) {
  flush();
  shader_alpha_map[c] = t;
}

void sspr(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, bool flipx = false,
          bool flipy = false) {
  sprites_draw(&sprites, sx, sy, sw, sh, dx, dy + VOX_SPRITES_WIDTH, dw, dh, flipx, flipy);
}

void sspr(int sx, int sy, int sw, int sh, int dx, int dy) { sspr(sx, sy, sw, sh, dx, dy, sw, sh); }

void spr(int n, int x, int y, int w = 1, int h = 1, bool flipx = false, bool flipy = false) {
  if (w < 0) w = 0;
  if (h < 0) h = 0;
  if (w > VOX_SPRITES_COUNT) w = VOX_SPRITES_COUNT;
  if (h > VOX_SPRITES_COUNT) h = VOX_SPRITES_COUNT;

  int nx = (n % VOX_SPRITES_COUNT) * VOX_SPRITE_WIDTH;
  int ny = (n / VOX_SPRITES_COUNT) * VOX_SPRITE_WIDTH;

  w *= VOX_SPRITE_WIDTH;
  h *= VOX_SPRITE_WIDTH;

  sspr(x, y, w, h, nx, ny, w, h, flipx, flipy);
}

void rect(int x0, int y0, int x1, int y1, int c = 7) {
  sprites_draw(&sprites, x0, y0, x1 - x0, y1 - y0, 0, 0, 8, 8);
}

void print(const char* str, int x, int y, uint8_t c = 7) {
  palt();
  pal(7, c);
  for (const char* c = str; *c; ++c) {
    int nx = (*c % VOX_SPRITES_COUNT) * VOX_SPRITE_WIDTH;
    int ny = (*c / VOX_SPRITES_COUNT) * VOX_SPRITE_WIDTH;
    sprites_draw(&sprites, x, y, VOX_SPRITE_WIDTH, VOX_SPRITE_WIDTH, nx, ny, VOX_SPRITE_WIDTH,
                 VOX_SPRITE_WIDTH);
    x += VOX_SPRITE_WIDTH / 2;
  }
}

uint64_t rnd() {
  static uint64_t wyhash64_x;
  wyhash64_x += 0x60bee2bee120fc15;
  __uint128_t tmp;
  tmp = (__uint128_t)wyhash64_x * 0xa3b195354a39b70d;
  uint64_t m1 = (tmp >> 64) ^ tmp;
  tmp = (__uint128_t)m1 * 0x1b03738712fad5c9;
  uint64_t m2 = (tmp >> 64) ^ tmp;
  return m2;
}

void update() {}

void draw() {
  cls();

  palt();
  pal();

  palt(14, true);
#if 0
  for (int i = 0; i < 4000; ++i) {
    spr(16 * 5 + rnd() % 8, rnd() % 128, rnd() % 128);
  }

  print("HELLO MY NAME IS MIKE!", 0, 8, 8);
#else

  // int lastx = rnd() % 128, lasty = rnd() % 128;
  // for (int i = 0; i < 10000; ++i) {
  //  int nextx = rnd() % 128, nexty = rnd() % 128;
  //  line(lasty, lasty, nextx, nexty, rnd() % 15 + 1);
  //  lastx = nextx;
  //  lasty = nexty;
  //}
  // line(0, 0, 32, 32);

  // rect(0, 0, 32, 32);

  for (int i = 0; i < 100; ++i) {
    int nextx = rnd() % 128, nexty = rnd() % 128;
    rect(nextx, nexty, nextx + 1, nexty + 1, rnd() % 15 + 1);
  }

  // print("hello my name is mike!", 0, 8);
  // sspr_raw(0, 0, 8, 8, 0, 0, 8, 8);
  // spr(16 * 6, 0, 0, 1, 1, false, true);
  // spr(16 * 6, 0, 0);
  // sspr(0, 0, 16, 16, 0, 8, 8, 8, true);
#endif
}

void on_debug_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                      const GLchar* message, const void* user_data) {
  SDL_Log("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

int main(int argc, char* argv[]) {
  // stbi_set_flip_vertically_on_load(true);

  if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize video: %s", SDL_GetError());
    return 1;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

  SDL_Window* window = SDL_CreateWindow("Vox", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                        VOX_DEFAULT_SCREEN_WIDTH, VOX_DEFAULT_SCREEN_HEIGHT,
                                        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (!window) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create window: %s", SDL_GetError());
    return 1;
  }

  SDL_GLContext context = SDL_GL_CreateContext(window);
  if (!context) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create GL context: %s", SDL_GetError());
    return 1;
  }

  SDL_Log("glGetString(GL_VERSION) returns %s\n", glGetString(GL_VERSION));

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(on_debug_message, 0);

  bool is_running = init();

  screen_rect = screen_calc_rect(VOX_DEFAULT_SCREEN_WIDTH, VOX_DEFAULT_SCREEN_HEIGHT);
  glViewport(screen_rect.x, screen_rect.y, screen_rect.w, screen_rect.h);
  glLineWidth(8.0);

  while (is_running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_running = false;
        break;
      }

      if (event.type == SDL_WINDOWEVENT) {
        switch (event.window.event) {
          case SDL_WINDOWEVENT_RESIZED:
          case SDL_WINDOWEVENT_SIZE_CHANGED: {
            screen_rect = screen_calc_rect(event.window.data1, event.window.data2);
            glViewport(screen_rect.x, screen_rect.y, screen_rect.w, screen_rect.h);
          } break;
        }
      }
    }

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);
    update();
    draw();
    flush();
    glDisable(GL_SCISSOR_TEST);

    SDL_GL_SwapWindow(window);
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
