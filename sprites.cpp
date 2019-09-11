#include "sprites.h"

#include "image.h"
#include "shader.h"
#include "vox.h"

#include <epoxy/gl.h>
#include <glm/gtc/type_ptr.hpp>

bool sprites_load_texture(Sprites* sprites, const char* filename, bool is_system_sprites = false) {
  if (!sprites->texture) {
    glGenTextures(1, &sprites->texture);
    glBindTexture(GL_TEXTURE_2D, sprites->texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, VOX_SPRITES_WIDTH, 2 * VOX_SPRITES_WIDTH, 0,
                 GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
  }

  uint8_t* data = image_load(filename);

  if (!data) {
    return false;
  }

  int offset = is_system_sprites ? 0 : VOX_SPRITES_WIDTH;
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, offset, VOX_SPRITES_WIDTH, VOX_SPRITES_WIDTH, GL_RED_INTEGER,
                  GL_UNSIGNED_BYTE, data);

  delete[] data;

  return true;
}

bool sprites_load_shader(Sprites* sprites) {
  sprites->shader = shader_load("sprites");
  return sprites->shader != VOX_ERROR;
}

bool sprites_init(Sprites* sprites) {
  sprites->batch_count = 0;

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

  glGenVertexArrays(1, &sprites->vao);
  glGenBuffers(1, &sprites->instance_vbo);

  unsigned int vbo;
  glGenBuffers(1, &vbo);

  unsigned int ebo;
  glGenBuffers(1, &ebo);

  glBindVertexArray(sprites->vao);

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
    glBindBuffer(GL_ARRAY_BUFFER, sprites->instance_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::uvec2) * VOX_MAX_SPRITE_BATCH, nullptr,
                 GL_DYNAMIC_DRAW);

    glVertexAttribIPointer(2, 2, GL_UNSIGNED_INT, 2 * sizeof(GLuint), (void*)0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(2, 1);
  }

  glBindVertexArray(0);

  return sprites_load_texture(sprites, "pico8_font.png", true) &&
         sprites_load_texture(sprites, "sprites1.png") && sprites_load_shader(sprites);
}

void sprites_flush(Sprites* sprites) {
  if (sprites->batch_count > 0) {
    glBindTexture(GL_TEXTURE_2D, sprites->texture);
    glUseProgram(sprites->shader);

    glUniformMatrix4fv(glGetUniformLocation(sprites->shader, "proj"), 1, GL_FALSE,
                       glm::value_ptr(shader_proj));
    glUniform3fv(glGetUniformLocation(sprites->shader, "palette"), 16,
                 glm::value_ptr(shader_palette[0]));
    glUniform1iv(glGetUniformLocation(sprites->shader, "colorMap"), 16, shader_color_map);
    glUniform1iv(glGetUniformLocation(sprites->shader, "alphaMap"), 16, shader_alpha_map);

    glBindVertexArray(sprites->vao);
    glBindBuffer(GL_ARRAY_BUFFER, sprites->instance_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::uvec2) * sprites->batch_count,
                    &sprites->batch[0]);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, sprites->batch_count);
    glBindVertexArray(0);
    sprites->batch_count = 0;
  }
}

void sprites_draw(Sprites* sprites, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh,
                  bool flipx, bool flipy) {
  if (sprites->batch_count >= VOX_MAX_SPRITE_BATCH) {
    sprites_flush(sprites);
  }

  if (flipx) dw = -dw;
  if (flipy) dh = -dh;

  glm::uvec2 s;
  s.x = (((uint8_t)(sx + 128) & 0xFF) << 24) | (((uint8_t)(sy + 128) & 0xFF) << 16) |
        (((uint8_t)(sw + 128) & 0xFF) << 8) | (((uint8_t)(sh + 128) & 0xFF) << 0);
  s.y = (((uint8_t)(dx + 128) & 0xFF) << 24) | (((uint8_t)(dy + 128) & 0xFF) << 16) |
        (((uint8_t)(dw + 128) & 0xFF) << 8) | (((uint8_t)(dh + 128) & 0xFF) << 0);

  sprites->batch[sprites->batch_count++] = s;
}
