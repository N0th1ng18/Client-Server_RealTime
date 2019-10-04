#include "renderer.h"

//Functions
void sendObjectUniforms(RenderResources* renderResources, RenderState* renderState, int i);

//Render Cameras

//Render
void renderObjects(RenderResources* renderResources, RenderState* renderState){

    for(int i=0; i < renderState->num_objects; i++){

        //Bind Program (Check if program is already bound)
        if(renderResources->programs[renderState->objects[i].programID].programID != renderState->bound_program){
            glUseProgram(renderResources->programs[renderState->objects[i].programID].programID);
            renderState->bound_program = renderResources->programs[renderState->objects[i].programID].programID;
        }
        //Bind VAO
        if(renderResources->vaos[renderState->objects[i].vaoID].id != renderState->bound_vao){
            glBindVertexArray(renderResources->vaos[renderState->objects[i].vaoID].id);
            renderState->bound_vao = renderResources->vaos[renderState->objects[i].vaoID].id;
        }
        //Bind Texture
        if(renderResources->textures[renderState->objects[i].textureID] != renderState->bound_texture){
            glBindTexture(GL_TEXTURE_2D, renderResources->textures[renderState->objects[i].textureID]);
            renderState->bound_texture = renderResources->textures[renderState->objects[i].textureID];
        }
        //Uniforms
        sendObjectUniforms(renderResources, renderState, i);
        //Draw
        glDrawElements(GL_TRIANGLES, renderResources->vaos[renderState->objects[i].vaoID].indices_size, GL_UNSIGNED_INT, (void*)0);

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
int createObject(RenderState* renderState, const char* path, int width, int height){

    /* READ FROM "PATH" TO GET OBJECT RESOURCES & ATTRIBUTES */

    //Check Max Objects

    renderState->objects[renderState->num_objects].textureID = 0;
    renderState->objects[renderState->num_objects].vaoID = 0;
    renderState->objects[renderState->num_objects].programID = 0;

    //Initial Position
    renderState->objects[renderState->num_objects].offset = glm::vec3(0.0f, 0.0f, 0.0f);
    renderState->objects[renderState->num_objects].pos = glm::vec3(0.0f, 0.0f, 0.0f);
    renderState->objects[renderState->num_objects].scale = glm::vec3(200.0f, 200.0f, 1.0f);
    renderState->objects[renderState->num_objects].rotate = glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f));

    //Transformation Matrix
    renderState->objects[renderState->num_objects].uniform_mvp[0] = glm::scale(renderState->objects[renderState->num_objects].uniform_mvp[0], renderState->objects[renderState->num_objects].scale);// Scale
    renderState->objects[renderState->num_objects].uniform_mvp[0] = glm::rotate(renderState->objects[renderState->num_objects].uniform_mvp[0], renderState->objects[renderState->num_objects].rotate.x, glm::vec3(1.0f, 0.0f, 0.0f));// Rotate X
    renderState->objects[renderState->num_objects].uniform_mvp[0] = glm::rotate(renderState->objects[renderState->num_objects].uniform_mvp[0], renderState->objects[renderState->num_objects].rotate.y, glm::vec3(0.0f, 1.0f, 0.0f));// Rotate Y  
    renderState->objects[renderState->num_objects].uniform_mvp[0] = glm::rotate(renderState->objects[renderState->num_objects].uniform_mvp[0], renderState->objects[renderState->num_objects].rotate.z, glm::vec3(0.0f, 0.0f, 1.0f));// Rotate Z  
    renderState->objects[renderState->num_objects].uniform_mvp[0] = glm::translate(renderState->objects[renderState->num_objects].uniform_mvp[0], renderState->objects[renderState->num_objects].pos);// Translate
    //View Matrix
    //Add Camera and active camera's view matrix is sent for world Objects

    //Projection Matrix
    renderState->objects[renderState->num_objects].uniform_mvp[2] = glm::ortho<float>(0.0f, static_cast<float>(width), 0.0f,  static_cast<float>(height),  -1.0f, 1.0f);

    return renderState->num_objects++;
}

int createText(RenderState* renderState, const char* path){

    renderState->texts[renderState->num_texts].textureID = 1;
    renderState->texts[renderState->num_texts].vaoID = 1;
    renderState->texts[renderState->num_texts].programID = 1;
    renderState->texts[renderState->num_texts].fontFile = 0;

    return renderState->num_texts++;
}

void sendObjectUniforms(RenderResources* renderResources, RenderState* renderState, int i){

        //Vertex Shader Uniforms
        for(int j=0; j < renderResources->programs[renderState->objects[i].programID].num_vert_uniforms; j++){
            
            if(renderResources->programs[renderState->objects[i].programID].uniform_vert_types[j] == "mat4"){

                glUniformMatrix4fv(glGetUniformLocation(
                        renderResources->programs[renderState->objects[i].programID].programID, 
                        renderResources->programs[renderState->objects[i].programID].uniform_vert_names[j].c_str()),
                        1, GL_FALSE, glm::value_ptr(renderState->objects[i].uniform_mvp[j]));

            }else{
                std::cout << "Undefined Vertex Shader Uniform" << std::endl;
            }
        }
        //Fragment Shader Uniforms
        for(int j=0; j < renderResources->programs[renderState->objects[i].programID].num_frag_uniforms; j++){

            if(renderResources->programs[renderState->objects[i].programID].uniform_frag_types[j] == "sampler2D"){
                glUniform1i(glGetUniformLocation(
                        renderResources->programs[renderState->objects[i].programID].programID, 
                        renderResources->programs[renderState->objects[i].programID].uniform_frag_names[j].c_str()), 
                        j); 
            }else{
                std::cout << "Undefined Fragment Shader Uniform" << std::endl;
            }
        }

}