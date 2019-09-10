#include <SDL.h>

#include <epoxy/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <fstream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define VOX_WIDTH 128 // Must be a power of 2
#define VOX_PADDING 8

#define VOX_SPRITES_COUNT 16
#define VOX_SPRITE_WIDTH 8
#define VOX_SPRITES_WIDTH (VOX_SPRITES_COUNT * VOX_SPRITE_WIDTH)

namespace {

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RGB;

RGB palette[] = {
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

glm::vec3 spalette[16];

int salpha_map[] = {
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

int scolor_map[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
};

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
    RGB p = palette[i];
    double dist = color_distance(r, g, b, p.r, p.g, p.b);
    if (dist < min_dist) {
      c = i;
      min_dist = dist;
    }
  }
  return c;
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
  return { (width - w) / 2, (height - w) / 2, w, w };
}

} // namespace

unsigned int ERROR = -1;

std::string read_content(const std::string& filename) {
  std::ifstream f(filename);
  std::string str;

  if (f.fail()) {
    return "";
  }

  f.seekg(0, std::ios::end);
  str.reserve(f.tellg());
  f.seekg(0, std::ios::beg);

  str.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  return str;
}

bool compile_shader(unsigned int id, const std::string& source) {
  const char* p = source.c_str();
  glShaderSource(id, 1, &p, nullptr);
  glCompileShader(id);

  int success;
  glGetShaderiv(id, GL_COMPILE_STATUS, &success);

  if (!success) {
    char info_log[512];
    glGetShaderInfoLog(id, 512, nullptr, info_log);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to compile shader: %s", info_log);
    return false;
  }

  return true;
}

unsigned int load_shader(const char* name) {
  unsigned int shader;

  unsigned int vertex_shader;
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  std::string vertex_shader_file(name);
  vertex_shader_file.append(".vert");
  if (!compile_shader(vertex_shader, read_content(vertex_shader_file))) {
    return ERROR;
  }

  unsigned int fragment_shader;
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  std::string fragment_shader_file(name);
  fragment_shader_file.append(".frag");
  if (!compile_shader(fragment_shader, read_content(fragment_shader_file))) {
    return ERROR;
  }

  shader = glCreateProgram();
  glAttachShader(shader, vertex_shader);
  glAttachShader(shader, fragment_shader);
  glLinkProgram(shader);

  int success;
  glGetProgramiv(shader, GL_LINK_STATUS, &success);
  if (!success) {
    char info_log[512];
    glGetProgramInfoLog(shader, 512, nullptr, info_log);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to link shader: %s", info_log);
    return ERROR;
  }

  return shader;
}

uint8_t* load_image(const char* filename) {
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
      palettized[j * VOX_SPRITES_WIDTH + i] = find_closest_color(p[0], p[1], p[2]);
    }
  }

  stbi_image_free(data);

  return palettized;
}

unsigned int shader;
unsigned int texture;
unsigned int vao;
unsigned int instance_vbo;

#define VOX_MAX_SPRITE_BATCH 8192
glm::uvec2 sprite_batch[VOX_MAX_SPRITE_BATCH];
int sprite_batch_count = 0;

bool load_sprites(const char* filename, bool is_system_sprites = false) {
  if (!texture) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, VOX_SPRITES_WIDTH, 2 * VOX_SPRITES_WIDTH, 0,
                 GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
  }

  uint8_t* data = load_image(filename);

  if (!data) {
    return false;
  }

  int offset = is_system_sprites ? 0 : VOX_SPRITES_WIDTH;
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, offset, VOX_SPRITES_WIDTH, VOX_SPRITES_WIDTH, GL_RED_INTEGER,
                  GL_UNSIGNED_BYTE, data);

  delete[] data;

  return true;
}

void initialize() {
  for (int i = 0; i < 16; ++i) {
    RGB c = palette[i];
    spalette[i] = glm::vec3((float)c.r / 256.0, (float)c.g / 256.0, (float)c.b / 256.0);
  }

  float vertices[] = {
    1.f, 1.f, 0.0f, 1.0f, 1.0f, // top right
    1.f, 0.f, 0.0f, 1.0f, 0.0f, // bottom right
    0.f, 0.f, 0.0f, 0.0f, 0.0f, // bottom left
    0.f, 1.f, 0.0f, 0.0f, 1.0f  // top left
  };

  unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
  };

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &instance_vbo);

  unsigned int vbo;
  glGenBuffers(1, &vbo);

  unsigned int ebo;
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);

  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  }

  {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

  {
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::uvec2) * VOX_MAX_SPRITE_BATCH, nullptr,
                 GL_DYNAMIC_DRAW);

    glVertexAttribIPointer(2, 2, GL_UNSIGNED_INT, 2 * sizeof(GLuint), (void*)0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(2, 1);
  }

  glBindVertexArray(0);

  if (!load_sprites("pico8_font.png", true) || !load_sprites("sprites1.png")) {
    SDL_Quit();
    exit(1);
  }

  shader = load_shader("sprite");
  if (shader == ERROR) {
    SDL_Quit();
    exit(1);
  }

  SDL_Rect screen_rect = calc_screen_rect(800, 800);
  glViewport(screen_rect.x, screen_rect.y, screen_rect.w, screen_rect.h);
}

void flush() {
  if (sprite_batch_count > 0) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::uvec2) * sprite_batch_count, &sprite_batch[0]);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, sprite_batch_count);
    glBindVertexArray(0);
    sprite_batch_count = 0;
  }
}

void pal() {
  flush();
  for (int i = 0; i < 16; ++i) {
    scolor_map[i] = i;
  }
  glUniform1iv(glGetUniformLocation(shader, "colorMap"), 16, scolor_map);
}

void pal(uint8_t c0, uint8_t c1) {
  flush();
  scolor_map[c0] = c1;
  glUniform1iv(glGetUniformLocation(shader, "colorMap"), 16, scolor_map);
}

void palt() {
  flush();
  memset(salpha_map, 0, sizeof(salpha_map));
  salpha_map[0] = 1;
  glUniform1iv(glGetUniformLocation(shader, "alphaMap"), 16, salpha_map);
}

void palt(int c, bool t) {
  flush();
  salpha_map[c] = t;
  glUniform1iv(glGetUniformLocation(shader, "alphaMap"), 16, salpha_map);
}

void sspr_raw(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, bool flipx = false,
              bool flipy = false) {
  if (sprite_batch_count >= VOX_MAX_SPRITE_BATCH) {
    flush();
  }

  if (flipx) dw = -dw;
  if (flipy) dh = -dh;

  glm::uvec2 s;
  s.x = (((uint8_t)(sx + 128) & 0xFF) << 24) | (((uint8_t)(sy + 128) & 0xFF) << 16) |
        (((uint8_t)(sw + 128) & 0xFF) << 8) | (((uint8_t)(sh + 128) & 0xFF) << 0);
  s.y = (((uint8_t)(dx + 128) & 0xFF) << 24) | (((uint8_t)(dy + 128) & 0xFF) << 16) |
        (((uint8_t)(dw + 128) & 0xFF) << 8) | (((uint8_t)(dh + 128) & 0xFF) << 0);

  sprite_batch[sprite_batch_count++] = s;
}

void sspr(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, bool flipx = false,
          bool flipy = false) {
  sspr_raw(sx, sy, sw, sh, dx, dy + VOX_SPRITES_WIDTH, dw, dh, flipx, flipy);
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

void rect(int x0, int y0, int x1, int y1) { sspr_raw(x0, y0, x1 - x0, y1 - y0, 0, 0, 8, 8); }

void print(const char* str, int x, int y, uint8_t c = 7) {
  palt();
  pal(7, c);
  for (const char* c = str; *c; ++c) {
    int nx = (*c % VOX_SPRITES_COUNT) * VOX_SPRITE_WIDTH;
    int ny = (*c / VOX_SPRITES_COUNT) * VOX_SPRITE_WIDTH;
    sspr_raw(x, y, VOX_SPRITE_WIDTH, VOX_SPRITE_WIDTH, nx, ny, VOX_SPRITE_WIDTH, VOX_SPRITE_WIDTH);
    x += VOX_SPRITE_WIDTH / 2;
  }
}

uint64_t wyhash64_x;

uint64_t rnd() {
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
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glBindTexture(GL_TEXTURE_2D, texture);
  glUseProgram(shader);

  glm::mat4 proj = glm::ortho(0.0f, 128.0f, 128.0f, 0.0f, 0.0f, 1.0f);

  glUniformMatrix4fv(glGetUniformLocation(shader, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
  glUniform3fv(glGetUniformLocation(shader, "palette"), 16, glm::value_ptr(spalette[0]));
  palt();
  pal();

  palt(14, true);
#if 1
  for (int i = 0; i < 4000; ++i) {
    spr(16 * 5 + rnd() % 8, rnd() % 128, rnd() % 128);
  }

  print("hello my name is mike!", 0, 8, 8);
#else
  // print("hello my name is mike!", 0, 8);
  // sspr_raw(0, 0, 8, 8, 0, 0, 8, 8);
  // spr(16 * 6, 0, 0, 1, 1, false, true);
  // spr(16 * 6, 0, 0);
  // sspr(0, 0, 16, 16, 0, 8, 8, 8, true);
#endif

  flush();
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
                                        800, 800, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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

  bool is_running = true;

  initialize();

  SDL_Rect screen_rect = calc_screen_rect(800, 800);
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
            screen_rect = calc_screen_rect(event.window.data1, event.window.data2);
            glViewport(screen_rect.x, screen_rect.y, screen_rect.w, screen_rect.h);
          } break;
        }
      }
    }

    update();

    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
    glScissor(screen_rect.x, screen_rect.y, screen_rect.w, screen_rect.h);
    draw();
    glDisable(GL_SCISSOR_TEST);

    SDL_GL_SwapWindow(window);
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
