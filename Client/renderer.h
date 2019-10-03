#ifndef RENDERER_H
#define RENDERER_H

#include "Windows.h"
#include <iostream>         //For console
#include <GL/glew.h>        //Determining which OpenGL extensions are supported on device
#include <GLFW/glfw3.h>     //Handle for the window
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iterator>


//Resources
#define MAX_TEXTURES 2
#define MAX_VAOS 2
#define MAX_VBOS 1
#define MAX_PROGRAMS 2
#define MAX_FONTFILES 1

//Entities
#define MAX_OBJECTS 1
#define MAX_TEXTS 1

//Structures
struct Program{
    GLuint programID; 
    //Layouts - Used for VAO creation
    int num_layouts = 0;
    std::vector<int> layout_locations;
    std::vector<std::string> layout_types;
    std::vector<std::string> layout_names;
    //Uniforms - Used for uploading uniforms to the GPU
    int num_vert_uniforms = 0;
    std::vector<std::string> uniform_vert_types;
    std::vector<std::string> uniform_vert_names;
    int num_frag_uniforms = 0;
    std::vector<std::string> uniform_frag_types;
    std::vector<std::string> uniform_frag_names;
};

struct FontFile{
    //Info
    std::string font_name;
    int size;
    bool bold;
    bool italic;
    std::string charset;
    bool unicode;
    int stretchH;
    bool smooth;
    bool aa;
    int padding[4] = {0, 0, 0, 0};
    int spacing[2] = {0, 0};
    //Common
    int line_height;
    int base;
    int scaleW;
    int scaleH;
    int pages;
    bool packed;
    //Page
    int page_id;
    std::string filename;
    //Chars
    int chars_count;
    //Char
    int* ascii_id;
    int* x;
    int* y;
    int* width;
    int* height;
    int* xoffset;
    int* yoffset;
    int* xadvance;
    int* page;
    int* chnl;
    //Kernings
    int kernings_count;
    //Kerning
    int* first;
    int* second;
    int* amount;
};

struct VAO{
    GLuint id;
    GLuint indices_size;
};

struct Camera{
    //Position
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 acc;
    //Transforms
    glm::vec3 offset;
    glm::vec3 scale;
    glm::vec3 rotate;
    //Matrix
    glm::mat4 uniform_mvp[3] = {glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f)}; // {Model, View, Projection}
};

struct Object{
    GLuint programID;
    GLuint vaoID;
    GLuint textureID;
    //Position
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 acc;
    //Transforms
    glm::vec3 offset;
    glm::vec3 scale;
    glm::vec3 rotate;
    //Matrix
    glm::mat4 uniform_mvp[3] = {glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f)}; // {Model, View, Projection}
    //...
};

struct Text{
    GLuint programID;
    GLuint vaoID;
    GLuint textureID;
    int fontFile;
    //glm::vec3 pos;
    //...
};

struct RenderResources{
	int num_textures = 0;
	int num_vaos = 0;
    int num_vbos = 0;
	int num_programs = 0;
	int num_fontFiles = 0;

	GLuint textures[MAX_TEXTURES];
	VAO vaos[MAX_VAOS];          
    GLuint vbos[MAX_VBOS];
	Program programs[MAX_PROGRAMS];
    FontFile fontFiles[MAX_FONTFILES];
    //Object Files -> enum to discribe object type
};

struct RenderState{
    int num_objects = 0;
    int num_texts = 0;

    //Cameras
    Object objects[MAX_OBJECTS];
    Text texts[MAX_TEXTS];
    //Text
    
};

//Add Resources
int addTexture(RenderResources* renderResources, GLuint texture);
int addVAO_Elements(RenderResources* renderResources, GLuint vao, GLuint indices_size);
int addVAO(RenderResources* renderResources, GLuint vao);
int addVBO(RenderResources* renderResources, GLuint vao);
int addProgram(RenderResources* renderResources);
int addFontFile(RenderResources* renderResources);

//Delete Resources

//Add Entities
int createObject(RenderState* renderState, const char* path, int width, int height);
int createText(RenderState* renderState, const char* path);

//Remove Entities

//Destroy Entities

//Render Functions
void renderObjects(RenderResources* renderResources, RenderState* renderState);
void renderTexts(RenderResources* renderResources, RenderState* renderState); //renderState, Font, Color, Pos, Size, Orientation

#endif