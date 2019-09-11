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

#define MAX_LINES 128

unsigned int shader;
unsigned int vao;
unsigned int vbo;
glm::vec3 batch[MAX_LINES];
size_t batch_count = 0;

void lines_flush() {
  if (batch_count > 0) {
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "proj"), 1, GL_FALSE,
                       glm::value_ptr(shader_proj));
    glUniform3fv(glGetUniformLocation(shader, "palette"), 16,
                 glm::value_ptr(shader_palette[0]));
    glUniform1iv(glGetUniformLocation(shader, "colorMap"), 16, shader_color_map);
    glUniform1iv(glGetUniformLocation(shader, "alphaMap"), 16, shader_alpha_map);


    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * batch_count,
                    glm::value_ptr(batch[0]));
    glDrawArrays(GL_LINES, 0, batch_count);

    batch_count = 0;
  }
}

void lines_draw(int x0, int y0, int x1, int y1, int c) {
  if (batch_count >= MAX_LINES) {
    lines_flush();
  }
  batch[batch_count++] = glm::vec3(x0, y0, c);
  batch[batch_count++] = glm::vec3(x1, y1, c);
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

bool init() {
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * MAX_LINES, nullptr, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(0);

  shader = shader_load("lines");
  return shader != -1;
}

void draw() {
  //lines_draw(0, 0, 32, 32, 8);
  //lines_draw(32, 32, 48, 32, 7);
  int lastx = rnd() % 128, lasty = rnd() % 128;
  for (int i = 0; i < 4000; ++i) {
    int nextx = rnd() % 128, nexty = rnd() % 128;
    lines_draw(lasty, lasty, nextx, nexty, rnd() % 15 + 1);
    lastx = nextx;
    lasty = nexty;
  }
  lines_flush();
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

  SDL_Window* window = SDL_CreateWindow("Lines", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
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
