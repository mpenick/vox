#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D Texture;
uniform vec3 palette[16];

void main() {
  FragColor = texture(Texture, TexCoord);
}
