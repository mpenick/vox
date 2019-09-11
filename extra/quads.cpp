#include "shader.h"

#include <SDL.h>

#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define LINES_DEFAULT_SCREEN_WIDTH 800
#define LINES_DEFAULT_SCREEN_HEIGHT 800

void on_debug_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                      const GLchar* message, const void* user_data) {
  SDL_Log("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

#define VOX_MAX_LINE_BATCH 8192

struct Quads {
  unsigned int shader;
  unsigned int vao;
  unsigned int instance_vbo;
  glm::uvec2 batch[VOX_MAX_LINE_BATCH];
  size_t batch_count;
};

bool quads_init(Quads* quads) {
  quads->batch_count = 0;

  float vertices[] = {
    1.f, 1.f, 0.0f, 1.f, 0.f, 0.0f, 0.f, 0.f, 0.0f, 0.f, 1.f, 0.0f,
  };

  unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
  };

  glGenVertexArrays(1, &quads->vao);
  glGenBuffers(1, &quads->instance_vbo);

  unsigned int vbo;
  glGenBuffers(1, &vbo);

  unsigned int ebo;
  glGenBuffers(1, &ebo);

  glBindVertexArray(quads->vao);

  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  }

  {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
  }

#if 1
  {
    glBindBuffer(GL_ARRAY_BUFFER, quads->instance_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::uvec2) * VOX_MAX_LINE_BATCH, nullptr,
                 GL_DYNAMIC_DRAW);

    glVertexAttribIPointer(1, 2, GL_UNSIGNED_INT, 2 * sizeof(GLuint), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(1, 1);
  }
#endif

  glBindVertexArray(0);

  quads->shader = shader_load("quads");
  return quads->shader != -1;
}

void quads_flush(Quads* quads) {
  if (quads->batch_count > 0) {
    glUseProgram(quads->shader);
    glUniformMatrix4fv(glGetUniformLocation(quads->shader, "proj"), 1, GL_FALSE,
                       glm::value_ptr(shader_proj));
    glUniform3fv(glGetUniformLocation(quads->shader, "palette"), 16,
                 glm::value_ptr(shader_palette[0]));
    glUniform1iv(glGetUniformLocation(quads->shader, "colorMap"), 16, shader_color_map);
    glUniform1iv(glGetUniformLocation(quads->shader, "alphaMap"), 16, shader_alpha_map);

    glBindVertexArray(quads->vao);
    glBindBuffer(GL_ARRAY_BUFFER, quads->instance_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::uvec2) * quads->batch_count, &quads->batch[0]);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, quads->batch_count);
    glBindVertexArray(0);

    quads->batch_count = 0;
  }
}

int clamp(int v) {
  if (v < -128) return -128;
  if (v >= 128) return 128;
  return v;
}

void quads_draw(Quads* quads, int x0, int y0, int x1, int y1, int c) {
  if (quads->batch_count >= VOX_MAX_LINE_BATCH) {
    quads_flush(quads);
  }

  x0 = clamp(x0);
  y0 = clamp(y0);
  x1 = clamp(x1);
  y1 = clamp(y1);

  glm::uvec2 r;
  r.x = (((uint8_t)(x0 + 127) & 0xFF) << 24) | (((uint8_t)(y0 + 127) & 0xFF) << 16) |
        (((uint8_t)(x1 + 127) & 0xFF) << 8) | (((uint8_t)(y1 + 127) & 0xFF) << 0);
  r.y = c;
  quads->batch[quads->batch_count++] = r;
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

Quads quads;

bool init() { return quads_init(&quads); }

void draw() {
  // quads_draw(&quads, 0, 0, 32, 32, 8);
  // quads_draw(&quads, 32, 32, 32 + 48, 48, 7);

  for (int i = 0; i < 100000; ++i) {
    int nextx = rnd() % 128, nexty = rnd() % 128;
    quads_draw(&quads, nextx, nexty, nextx + 1, nexty + 1, rnd() % 15 + 1);
  }

  quads_flush(&quads);
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

  SDL_Window* window = SDL_CreateWindow("Quads", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                        LINES_DEFAULT_SCREEN_WIDTH, LINES_DEFAULT_SCREEN_HEIGHT,
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

  glViewport(0, 0, LINES_DEFAULT_SCREEN_WIDTH, LINES_DEFAULT_SCREEN_HEIGHT);

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
            glViewport(0, 0, event.window.data1, event.window.data2);
          } break;
        }
      }
    }

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    draw();

    SDL_GL_SwapWindow(window);
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
