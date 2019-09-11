#version 330 core
//layout (location = 0) in uint params;
layout (location = 0) in vec3 pos;

uniform mat4 proj;

//flat out int Color;

void main() {
  //float x = float((params >> 16u) & 0xFFu) - 128;
  //float y = float((params >> 8u) & 0xFFu) - 128;

  //Color = int((params) & 0xFFu);
  gl_Position = proj * vec4(pos, 1.0);
}
