#ifndef LINES_H
#define LINES_H

#define VOX_MAX_LINE_BATCH 8192

#include <glm/glm.hpp>

struct Lines {
  unsigned int shader;
  unsigned int vbo;
  unsigned int vao;
  glm::vec2 batch[2 * VOX_MAX_LINE_BATCH];
  unsigned int batch_count;
};

bool lines_init(Lines* lines);
void lines_flush(Lines* lines);
void lines_draw(Lines* lines, int x0, int y0, int x1, int y1, int c);

#endif // LINES_H
