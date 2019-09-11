#include "shader.h"

#include "color.h"

#include <SDL_log.h>
#include <epoxy/gl.h>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

const glm::mat4 shader_proj = glm::ortho(0.0f, 128.0f, 128.0f, 0.0f, 0.0f, 1.0f);

const glm::vec3 shader_palette[] = {
  glm::vec3(color_palette[0].r / 256.0, color_palette[0].g / 256.0, color_palette[0].b / 256.0),
  glm::vec3(color_palette[1].r / 256.0, color_palette[1].g / 256.0, color_palette[1].b / 256.0),
  glm::vec3(color_palette[2].r / 256.0, color_palette[2].g / 256.0, color_palette[2].b / 256.0),
  glm::vec3(color_palette[3].r / 256.0, color_palette[3].g / 256.0, color_palette[3].b / 256.0),
  glm::vec3(color_palette[4].r / 256.0, color_palette[4].g / 256.0, color_palette[4].b / 256.0),
  glm::vec3(color_palette[5].r / 256.0, color_palette[5].g / 256.0, color_palette[5].b / 256.0),
  glm::vec3(color_palette[6].r / 256.0, color_palette[6].g / 256.0, color_palette[6].b / 256.0),
  glm::vec3(color_palette[7].r / 256.0, color_palette[7].g / 256.0, color_palette[7].b / 256.0),
  glm::vec3(color_palette[8].r / 256.0, color_palette[8].g / 256.0, color_palette[8].b / 256.0),
  glm::vec3(color_palette[9].r / 256.0, color_palette[9].g / 256.0, color_palette[10].b / 256.0),
  glm::vec3(color_palette[10].r / 256.0, color_palette[10].g / 256.0, color_palette[10].b / 256.0),
  glm::vec3(color_palette[11].r / 256.0, color_palette[10].g / 256.0, color_palette[11].b / 256.0),
  glm::vec3(color_palette[12].r / 256.0, color_palette[12].g / 256.0, color_palette[12].b / 256.0),
  glm::vec3(color_palette[13].r / 256.0, color_palette[13].g / 256.0, color_palette[13].b / 256.0),
  glm::vec3(color_palette[14].r / 256.0, color_palette[14].g / 256.0, color_palette[14].b / 256.0),
  glm::vec3(color_palette[15].r / 256.0, color_palette[15].g / 256.0, color_palette[15].b / 256.0),
};

int shader_alpha_map[] = {
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

int shader_color_map[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
};

std::string read_content(const std::string& filename) {
  std::ifstream f(filename);
  std::string str;

  if (f.fail()) {
    return "";
  }

  f.seekg(0, std::ios::end);
  str.reserve(static_cast<size_t>(f.tellg()));
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

unsigned int shader_load(const char* name) {
  unsigned int shader;

  unsigned int vertex_shader;
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  std::string vertex_shader_file(name);
  vertex_shader_file.append(".vert");
  if (!compile_shader(vertex_shader, read_content(vertex_shader_file))) {
    return -1;
  }

  unsigned int fragment_shader;
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  std::string fragment_shader_file(name);
  fragment_shader_file.append(".frag");
  if (!compile_shader(fragment_shader, read_content(fragment_shader_file))) {
    return -1;
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
    return -1;
  }

  return shader;
}
