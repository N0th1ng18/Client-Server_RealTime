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


//Resources (do dynamically allocated like render state)
#define MAX_TEXTURES 2
#define MAX_MODELS 2
#define MAX_VAOS 2
#define MAX_VBOS 1
#define MAX_PROGRAMS 2
#define MAX_FONTFILES 1

enum ClientStates{
    CONNECT_TO_SERVER = 0,
    CONNECTING = 1,
    FAILED_TO_CONNECT = 2,
    CONNECTED = 3
};

//Structures
struct Program{
    GLuint id; 
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
    //Transforms for View
    glm::vec3 offset;
    glm::vec3 scale;
    glm::vec3 rotate;
    //Matrix
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
};

struct Object{
    GLuint program_index;
    GLuint vao_index;
    GLuint texture_index;
    GLuint camera_index;
    //Position
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 acc;
    //Transforms
    glm::vec3 offset;
    glm::vec3 scale;
    glm::vec3 rotate;
    //Matrix
    glm::mat4 transformation = glm::mat4(1.0f);
    //...
};

struct Text{
    GLuint program_index;
    GLuint vao_index;
    GLuint vbo_index;
    GLuint texture_index;
    int fontFile_index;
    GLuint camera_index;
    //Font Properties
    glm::vec3 f_color;
    float c_width;
    float c_edge;
    GLfloat text_buffer[6*5];
    char* text;
    //Position
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 acc;
    //Transforms
    glm::vec3 offset;
    glm::vec3 scale;
    glm::vec3 rotate;
    //Matrix
    glm::mat4 transformation = glm::mat4(1.0f);
};

struct RenderResources{
	int num_textures = 0;
    int num_models = 0;
	int num_vaos = 0;
    int num_vbos = 0;
	int num_programs = 0;
	int num_fontFiles = 0;

	GLuint textures[MAX_TEXTURES];
	VAO vaos[MAX_VAOS];          
    GLuint vbos[MAX_VBOS];
	Program programs[MAX_PROGRAMS];
    FontFile fontFiles[MAX_FONTFILES];
};

struct RenderState{ 

    RenderState(const int max_cameras, const int max_objects, const int max_texts){
        //Save Size of Arrays
        MAX_CAMERAS = max_cameras;
        MAX_OBJECTS = max_objects;
        MAX_TEXTS = max_texts;

        //Allocate Entities on Heap
        cameras = new Camera[max_cameras];
        objects = new Object[max_objects];
        texts = new Text[max_texts];

        //Initialize
        for(int i=0; i < max_cameras; i++){
            cameras[i] = {};
        }
        for(int i=0; i < max_objects; i++){
            objects[i] = {};
        }
        for(int i=0; i < max_texts; i++){
            texts[i] = {};
        }

        /* 
            Allocate Active Index Arrays for faster Alloc/Deletes
            - Alloc - set object arrays with data and mark slot active
            - Delete - mark slot not active
        */
        slotlist_cameras = new bool[max_cameras];
        slotlist_objects = new bool[max_objects];
        slotlist_texts = new bool[max_texts];

        //Initialize
        for(int i=0; i < max_cameras; i++){
            slotlist_cameras[i] = false;
        }
        for(int i=0; i < max_objects; i++){
            slotlist_objects[i] = false;
        }
        for(int i=0; i < max_texts; i++){
            slotlist_texts[i] = false;
        }

    }

    //Active State
    enum ClientStates clientState;

    //Currently Bound Resources
    GLuint bound_program_index = MAX_PROGRAMS;
    GLuint bound_vao_index = MAX_VAOS;
    GLuint bound_texture_index = MAX_TEXTURES;

    //Cameras
    int active_camera;
    int MAX_CAMERAS;
    bool* slotlist_cameras;
    int num_cameras = 0;
    Camera* cameras;

    //Objects
    int MAX_OBJECTS;
    bool* slotlist_objects;
    int num_objects = 0;
    Object* objects;

    //Texts
    int MAX_TEXTS;
    bool* slotlist_texts;
    int num_texts = 0;
    Text* texts;
    
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
Camera* addCamera(RenderState* renderState);
Object* addObject(RenderState* renderState);
Text* addText(RenderState* renderState);

//Remove Entities
void removeCamera(RenderState* renderState, int id);
void removeObject(RenderState* renderState, int id);
void removeText(RenderState* renderState, int id);

//Render Functions
void renderObjects(RenderResources* renderResources, RenderState* renderState);
void renderTexts(RenderResources* renderResources, RenderState* renderState, int screen_width, int screen_height); //renderState, Font, Color, Pos, Size, Orientation

//Clean Up
void destroyRenderState(RenderState* renderState);

#endif