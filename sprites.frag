#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform usampler2D Texture;

uniform vec3 palette[16];
uniform bool alphaMap[16];
uniform int colorMap[16];

void main() {
  int index = int(texture(Texture, TexCoord).r);
  if (alphaMap[index])
    discard;
  FragColor = vec4(palette[colorMap[index]], 1.0);
}
