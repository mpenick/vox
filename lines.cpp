#include "lines.h"

#include "shader.h"
#include "vox.h"

#include <epoxy/gl.h>
#include <glm/gtc/type_ptr.hpp>

bool lines_load_shader(Lines* lines) {
  lines->shader = shader_load("lines");
  return lines->shader != VOX_ERROR;
}

bool lines_init(Lines* lines) {
  lines->batch_count = 0;

  glGenVertexArrays(1, &lines->vao);
  glGenBuffers(1, &lines->vbo);

  glBindVertexArray(lines->vao);

  {
    // lines->batch[lines->batch_count++] = glm::vec2(0, 0);
    // lines->batch[lines->batch_count++] = glm::vec2(32, 32);

    float data[] = {
      0.0, 0.0, 1.0, 32.0, 32.0, 1.0,
    };
    glBindBuffer(GL_ARRAY_BUFFER, lines->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    // glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, 2 * sizeof(GLuint), (void*)0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
  }

  // glBindVertexArray(0);

  return lines_load_shader(lines);
}

void lines_flush(Lines* lines) {
  // if (lines->batch_count > 0) {
  glUseProgram(lines->shader);

  glUniformMatrix4fv(glGetUniformLocation(lines->shader, "proj"), 1, GL_FALSE,
                     glm::value_ptr(shader_proj));
  glUniform3fv(glGetUniformLocation(lines->shader, "palette"), 16,
               glm::value_ptr(shader_palette[0]));
  glUniform1iv(glGetUniformLocation(lines->shader, "colorMap"), 16, shader_color_map);
  glUniform1iv(glGetUniformLocation(lines->shader, "alphaMap"), 16, shader_alpha_map);

  glBindVertexArray(lines->vao);
  // glBindBuffer(GL_ARRAY_BUFFER, lines->vbo);
  // glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * lines->batch_count,
  // &lines->batch[0]);
  glDrawArrays(GL_LINES, 0, 2);
  glBindVertexArray(0);
  // lines->batch_count = 0;
  //}
}

void lines_draw(Lines* lines, int x0, int y0, int x1, int y1, int c) {
  /*
  if (lines->batch_count >= VOX_MAX_LINE_BATCH) {
    lines_flush(lines);
  }

  lines->batch[lines->batch_count++] = glm::vec2(x0, y0);
  lines->batch[lines->batch_count++] = glm::vec2(x1, y1);
  */
  /*
  unsigned int l0;
  l0 = (((uint8_t)(x0 + 128) & 0xFF) << 16) | (((uint8_t)(y0 + 128) & 0xFF) << 8) |
       (((uint8_t)(c & 0xFF)) << 0);
  lines->batch[lines->batch_count++] = l0;
  unsigned int l1;
  l1 = (((uint8_t)(x1 + 128) & 0xFF) << 16) | (((uint8_t)(y1 + 128) & 0xFF) << 8) |
       (((uint8_t)(c & 0xFF)) << 0);
  lines->batch[lines->batch_count++] = l1;
  */
}
