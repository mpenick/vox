#ifndef SPRITES_H
#define SPRITES_H

#include <glm/glm.hpp>

#define VOX_MAX_SPRITE_BATCH 8192

struct Sprites {
  unsigned int shader;
  unsigned int texture;
  unsigned int vao;
  unsigned int instance_vbo;
  glm::uvec2 batch[VOX_MAX_SPRITE_BATCH];
  unsigned int batch_count;
};

bool sprites_init(Sprites* sprites);
void sprites_flush(Sprites* sprites);
void sprites_draw(Sprites* sprites, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh,
                  bool flipx = false, bool flipy = false);

#endif // SPRITES_H
