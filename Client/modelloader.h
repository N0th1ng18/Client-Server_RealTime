#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <iostream>
#include <GL/glew.h>
#include "renderer.h"

struct Model{
    GLuint attribute_stride;
    GLuint buffer_size;
    GLuint indices_size;
    GLfloat* buffer;
    GLuint* indices;
};

Model* loadModel(const char *filePath, RenderResources* renderResources);
void createStaticObject_VAO(Model* model, RenderResources* renderResources);
void createDynamic2DText_VAO_VBO(RenderResources* renderResources);

#endif