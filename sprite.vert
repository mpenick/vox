#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 tex;
layout (location = 3) in vec4 offset;

uniform mat4 proj;

out vec3 Color;
out vec2 TexCoord;

void main() {
  gl_Position = proj * vec4(pos + vec3(offset.xy, 0.0), 1.0);
  Color = color;
  TexCoord = tex;
}
