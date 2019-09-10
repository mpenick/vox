#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in uvec2 params;

// params.x =  (posx, posy, width, height)
// params.y = (tex_posx, tex_posy, tex_width, tex_height) // Use the sign of tex_width/tex_height for flipping

uniform mat4 proj;

out vec3 Color;
out vec2 TexCoord;

const float SPRITE_TEX_WIDTH = 128.0;
const float SPRITE_TEX_HEIGHT = 256.0;

void main() {
  float sx = float((params.x >> 24u) & 0xFFu) - 128;
  float sy = float((params.x >> 16u) & 0xFFu) - 128;
  float sw = float((params.x >> 8u) & 0xFFu) - 128;
  float sh = float((params.x) & 0xFFu) - 128;

  float dx = float((params.y >> 24u) & 0xFFu) - 128;
  float dy = float((params.y >> 16u) & 0xFFu) - 128;
  float dw = float((params.y >> 8u) & 0xFFu) - 128;
  float dh = float((params.y) & 0xFFu) - 128;

  float texx = dw < 0 ? 1 - tex.x : tex.x;
  float texy = dh < 0 ? 1 - tex.y : tex.y;

  float texw = abs(dw) / SPRITE_TEX_WIDTH;
  float texh = abs(dh) / SPRITE_TEX_HEIGHT;

  float texox = dx / SPRITE_TEX_WIDTH;
  float texoy = dy / SPRITE_TEX_HEIGHT;

  gl_Position = proj * vec4((pos * vec3(sw, sh, 1.0)) + vec3(sx, sy, 0.0), 1.0);
  TexCoord = vec2(texox + texx * texw, texoy + texy * texh);
  //TexCoord = tex;
}
