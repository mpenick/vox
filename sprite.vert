#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec4 offset;

uniform mat4 proj;

out vec3 Color;
out vec2 TexCoord;

float sx = 1.0/16.0;
float sy = 1.0/11.0;

void main() {
  gl_Position = proj * vec4(pos + vec3(offset.xy, 0.0), 1.0);
  float nx = mod(offset.z, 16);
  float ny = floor(offset.z / 16);
  TexCoord = vec2(sx * nx + tex.x * 1.0/16.0, sy * ny + tex.y * 1.0/11.0);
  //TexCoord = tex;
}
