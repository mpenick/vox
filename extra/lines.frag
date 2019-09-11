#version 330 core
out vec4 FragColor;

uniform vec3 palette[16];
uniform bool alphaMap[16];
uniform int colorMap[16];

in float Color;

void main() {
  int index = int(Color);
  if (alphaMap[index])
    discard;
  FragColor = vec4(palette[index], 1.0f);
} 
