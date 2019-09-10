#ifndef SHADER_H
#define SHADER_H

#include <glm/glm.hpp>

unsigned int shader_load(const char* name);

extern const glm::mat4 shader_proj;

extern const glm::vec3 shader_palette[16];

extern int shader_alpha_map[16];

extern int shader_color_map[16];

#endif // SHADER_H
