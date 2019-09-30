#ifndef FONTLOADER_H
#define FONTLOADER_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <GL/glew.h>
#include "renderer.h"

//https://learnopengl.com/In-Practice/Text-Rendering

GLuint loadFontFile(const char *filePath, RenderResources* renderResources);
void deleteFontFile(FontFile* font);

#endif