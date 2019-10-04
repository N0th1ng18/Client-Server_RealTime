#include "renderer.h"

//Functions
void sendObjectUniforms(RenderResources* renderResources, RenderState* renderState, int i);

void bindProgram(GLuint program_index, RenderResources* renderResources, RenderState* renderState){
    if(program_index != renderState->bound_program_index){
        glUseProgram(renderResources->programs[program_index].id);
        renderState->bound_program_index = program_index;
        std::cout << "Program bound = " << renderResources->programs[program_index].id << std::endl;
    }
}
void bindVAO(GLuint vao_index, RenderResources* renderResources, RenderState* renderState){
    if(vao_index != renderState->bound_vao_index){
        glBindVertexArray(renderResources->vaos[vao_index].id);
        renderState->bound_vao_index = vao_index;
        std::cout << "VAO bound = " << renderResources->vaos[vao_index].id << std::endl;
    }
}
void bindTexture(GLuint texture_index, RenderResources* renderResources, RenderState* renderState){
    if(texture_index != renderState->bound_texture_index){
        glBindTexture(GL_TEXTURE_2D, renderResources->textures[texture_index]);
        renderState->bound_texture_index = texture_index;
        std::cout << "Texture bound = " << renderResources->textures[texture_index] << std::endl;
    }
}

//Render Cameras
void renderCameras(RenderResources* renderResources, RenderState* renderState){

    for(int i=0; i < renderState->num_cameras; i++){

        //Bind Program (Check if program is already bound)
        bindProgram(renderState->cameras[i].program_index, renderResources, renderState);
        //Uniforms
        glUniformMatrix4fv(glGetUniformLocation(
                        renderResources->programs[renderState->cameras[i].program_index].id, 
                        "projection"),
                        1, GL_FALSE, &renderState->cameras[i].projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(
                        renderResources->programs[renderState->cameras[i].program_index].id, 
                        "view"),
                        1, GL_FALSE, &renderState->cameras[i].view[0][0]);

    }

}

//Render
void renderObjects(RenderResources* renderResources, RenderState* renderState){

    for(int i=0; i < renderState->num_objects; i++){

        //Bind Program (Check if program is already bound)
        bindProgram(renderState->objects[i].program_index, renderResources, renderState);
        //Bind VAO
        bindVAO(renderState->objects[i].vao_index, renderResources, renderState);
        //Bind Texture
        bindTexture(renderState->objects[i].texture_index, renderResources, renderState);
        //Uniforms
        glUniformMatrix4fv(glGetUniformLocation(
                        renderResources->programs[renderState->objects[i].program_index].id, 
                        "transformation"),
                        1, GL_FALSE, &renderState->objects[i].transformation[0][0]);
        glUniform1i(glGetUniformLocation(
                                renderResources->textures[renderState->objects[i].texture_index], 
                                "texture1"), 
                                0); 

        //Draw
        glDrawElements(GL_TRIANGLES, renderResources->vaos[renderState->objects[i].vao_index].indices_size, GL_UNSIGNED_INT, (void*)0);

    }
}

void renderTexts(RenderResources* renderResources, RenderState* renderState){

    for(int i=0; i < renderState->num_texts; i++){
        
        //render text

        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

        // glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);

        // glDisable(GL_BLEND);

    }
}















//Resources
int addTexture(RenderResources* renderResources, GLuint texture){
    renderResources->textures[renderResources->num_textures] = texture;
    return renderResources->num_textures++;
}

int addVAO_Elements(RenderResources* renderResources, GLuint vao, GLuint indices_size){
    renderResources->vaos[renderResources->num_vaos].id = vao;
    renderResources->vaos[renderResources->num_vaos].indices_size = indices_size;
    return renderResources->num_vaos++;
}
int addVAO(RenderResources* renderResources, GLuint vao){
    renderResources->vaos[renderResources->num_vaos].id = vao;
    renderResources->vaos[renderResources->num_vaos].indices_size = 0;
    return renderResources->num_vaos++;
}

int addVBO(RenderResources* renderResources, GLuint vbo){
    renderResources->vbos[renderResources->num_vbos] = vbo;
    return renderResources->num_vbos++;
}

int addProgram(RenderResources* renderResources){
    return renderResources->num_programs++;
}

int addFontFile(RenderResources* renderResources){
    return renderResources->num_fontFiles++;
}

//Add Entities
int createCamera(RenderState* renderState, int width, int height){

    if(renderState->num_cameras > MAX_CAMERAS){
        return renderState->num_cameras;
    }

    renderState->cameras[renderState->num_cameras].program_index = 0;

    //Initial Position
    renderState->cameras[renderState->num_cameras].pos = glm::vec3(200.0f, 200.0f, 0.0f);

    //Projection Matrix
    renderState->cameras[renderState->num_cameras].projection = glm::ortho<float>(0.0f, static_cast<float>(width), 0.0f,  static_cast<float>(height),  -1.0f, 1.0f);

    //View Matrix
    renderState->cameras[renderState->num_cameras].view = glm::translate(renderState->cameras[renderState->num_cameras].view, renderState->cameras[renderState->num_cameras].pos);

    return renderState->num_cameras++;
}

int createObject(RenderState* renderState, const char* path){

    /* READ FROM "PATH" TO GET OBJECT RESOURCES & ATTRIBUTES */

    //Check Max Objects
    if(renderState->num_objects > MAX_OBJECTS){
        return renderState->num_objects;
    }

    renderState->objects[renderState->num_objects].program_index = 0;
    renderState->objects[renderState->num_objects].vao_index = 0;
    renderState->objects[renderState->num_objects].texture_index = 0;

    //Initial Position
    renderState->objects[renderState->num_objects].offset = glm::vec3(0.0f, 0.0f, 0.0f);
    renderState->objects[renderState->num_objects].pos = glm::vec3(0.0f, 0.0f, 0.0f);
    renderState->objects[renderState->num_objects].scale = glm::vec3(200.0f, 200.0f, 1.0f);
    renderState->objects[renderState->num_objects].rotate = glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f));

    //Transformation Matrix
    renderState->objects[renderState->num_objects].transformation = glm::scale(renderState->objects[renderState->num_objects].transformation, renderState->objects[renderState->num_objects].scale);// Scale
    renderState->objects[renderState->num_objects].transformation = glm::rotate(renderState->objects[renderState->num_objects].transformation, renderState->objects[renderState->num_objects].rotate.x, glm::vec3(1.0f, 0.0f, 0.0f));// Rotate X
    renderState->objects[renderState->num_objects].transformation = glm::rotate(renderState->objects[renderState->num_objects].transformation, renderState->objects[renderState->num_objects].rotate.y, glm::vec3(0.0f, 1.0f, 0.0f));// Rotate Y  
    renderState->objects[renderState->num_objects].transformation = glm::rotate(renderState->objects[renderState->num_objects].transformation, renderState->objects[renderState->num_objects].rotate.z, glm::vec3(0.0f, 0.0f, 1.0f));// Rotate Z  
    renderState->objects[renderState->num_objects].transformation = glm::translate(renderState->objects[renderState->num_objects].transformation, renderState->objects[renderState->num_objects].pos);// Translate

    return renderState->num_objects++;
}

int createText(RenderState* renderState, const char* path){

    renderState->texts[renderState->num_texts].texture_index = 1;
    renderState->texts[renderState->num_texts].vao_index = 1;
    renderState->texts[renderState->num_texts].program_index = 1;
    renderState->texts[renderState->num_texts].fontFile_index = 0;

    return renderState->num_texts++;
}


// void sendObjectUniforms(RenderResources* renderResources, RenderState* renderState, int i){

//         //Vertex Shader Uniforms
//         for(int j=0; j < renderResources->programs[renderState->objects[i].programID].num_vert_uniforms; j++){
            
//             if(renderResources->programs[renderState->objects[i].programID].uniform_vert_types[j] == "mat4"){

//                 glUniformMatrix4fv(glGetUniformLocation(
//                         renderResources->programs[renderState->objects[i].programID].programID, 
//                         renderResources->programs[renderState->objects[i].programID].uniform_vert_names[j].c_str()),
//                         1, GL_FALSE, glm::value_ptr(renderState->objects[i].uniform_mvp[j]));

//             }else{
//                 std::cout << "Undefined Vertex Shader Uniform" << std::endl;
//             }
//         }
//         //Fragment Shader Uniforms
//         for(int j=0; j < renderResources->programs[renderState->objects[i].programID].num_frag_uniforms; j++){

//             if(renderResources->programs[renderState->objects[i].programID].uniform_frag_types[j] == "sampler2D"){
//                 glUniform1i(glGetUniformLocation(
//                         renderResources->programs[renderState->objects[i].programID].programID, 
//                         renderResources->programs[renderState->objects[i].programID].uniform_frag_names[j].c_str()), 
//                         j); 
//             }else{
//                 std::cout << "Undefined Fragment Shader Uniform" << std::endl;
//             }
//         }

// }