#version 330 core
out vec4 FragColor;

//flat in int Color;

uniform vec3 palette[16];
uniform bool alphaMap[16];
uniform int colorMap[16];

void main() {
  //int index = Color;
  //if (alphaMap[index])
  //  discard;
  //FragColor = vec4(palette[7], 1.0);
  //FragColor = vec4(palette[colorMap[index]], 1.0);
  FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
