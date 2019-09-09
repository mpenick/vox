#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;

// params.x = sprite_num, posx, posy
// params.y = widthx, widthy, flipx, flipy

/// TODO:
// posx, posy, width, height
// tex_posx, tex_posy, tex_width, tex_height (use the sign of tex_width/tex_height)

layout (location = 2) in uvec2 params;

uniform mat4 proj;

out vec3 Color;
out vec2 TexCoord;

float sx = 1.0/16.0;
float sy = 1.0/32.0;

void main() {
  float n = float((params.x >> 16u) & 0xFFFFu);
  float px = float((params.x >> 8u) & 0xFFu) - 128;
  float py = float((params.x) & 0xFFu) - 128;

  float w = float((params.y >> 24u) & 0xFFu);
  float h = float((params.y >> 16u) & 0xFFu);
  bool flipx = bool((params.y >> 8u) & 0xFFu);
  bool flipy = bool(params.y & 0xFFu);

  float tx = flipx ? 1 - tex.x : tex.x;
  float ty = flipy ? 1 - tex.y : tex.y;

  gl_Position = proj * vec4((pos * vec3(w, h, 1.0)) + vec3(px, py, 0.0), 1.0);
  TexCoord = vec2(sx * (mod(n, 16) + tx * (w / 8.0)), sy * (floor(n / 16) + ty * (h / 8.0)));
}
