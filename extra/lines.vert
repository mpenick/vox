#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 proj;

out float Color;

void main() {
    Color = aPos.z;
    gl_Position = proj * vec4(aPos.x, aPos.y, 0.0, 1.0);
}
