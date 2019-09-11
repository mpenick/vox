#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in uvec2 params;

uniform mat4 proj;

out float Color;

void main() {
  float x0 = float((params.x >> 24u) & 0xFFu) - 127;
  float y0 = float((params.x >> 16u) & 0xFFu) - 127;
  float x1 = float((params.x >> 8u) & 0xFFu) - 127;
  float y1 = float((params.x) & 0xFFu) - 127;

  float w = x1 - x0;
  float h = y1 - y0;

  Color = float(params.y);
  gl_Position = proj * vec4((pos * vec3(w, h, 1.0)) + vec3(x0, y0, 0.0), 1.0);
}
