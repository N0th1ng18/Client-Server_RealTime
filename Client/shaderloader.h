#ifndef SHADERLOADER_H
#define SHADERLOADER_H

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "GL/glew.h"
#include "renderer.h"


GLuint loadShader(const char *vertex_path, const char *fragment_path, RenderResources* renderResources);
void deleteShader(Program* program);

#endif