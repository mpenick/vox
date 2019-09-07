#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in uvec2 params; // sprite_num, posx, posy, widthx, widthy, flipx, flipy

uniform mat4 proj;

out vec3 Color;
out vec2 TexCoord;

float sx = 1.0/16.0;
float sy = 1.0/11.0;
void main() {
  float px = float((params.x >> 16u) & 0xFFu) - 128;
  float py = float((params.x >> 8u) & 0xFFu) - 128;
  float n = float((params.x >> 24u) & 0xFFu);

  gl_Position = proj * vec4(pos + vec3(px, py, 0.0), 1.0);

  float nx = mod(n, 16);
  float ny = floor(n / 16);
  TexCoord = vec2(sx * nx + tex.x * 1.0/16.0, sy * ny + tex.y * 1.0/11.0);
}
