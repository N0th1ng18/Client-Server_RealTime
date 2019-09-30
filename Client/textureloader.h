#ifndef TEXTURELOADER_H
#define TEXTRUELOADER_H

#include <iostream>
#include <src/SOIL.h>
#include <GL/glew.h>
#include "renderer.h"

GLuint loadTexture(const char *filePath, RenderResources* renderResources);

#endif