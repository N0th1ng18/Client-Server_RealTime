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

    /*
        Cameras[0]      = UI Camera
        Cameras[1 -> n] = Other Cameras

        Render Only Active Camera, then UI Camera
    */

    //Only render Active Camera
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
                        "translation"),
                        1, GL_FALSE, &renderState->objects[i].transformation[0][0]);
        glUniform1i(glGetUniformLocation(
                        renderResources->textures[renderState->objects[i].texture_index], 
                        "texture1"), 
                        0); 

        //Draw
        glDrawElements(GL_TRIANGLES, renderResources->vaos[renderState->objects[i].vao_index].indices_size, GL_UNSIGNED_INT, (void*)0);

    }
}

void renderTexts(RenderResources* renderResources, RenderState* renderState, int width, int height){

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    for(int i=0; i < renderState->num_texts; i++){
        
        bindProgram(renderState->texts[i].program_index, renderResources, renderState);
        bindVAO(renderState->texts[i].vao_index, renderResources, renderState);
        bindTexture(renderState->texts[i].texture_index, renderResources, renderState);

        Text* txt = &renderState->texts[i];
        FontFile* ft = &renderResources->fontFiles[renderState->texts[i].fontFile_index];
        char* string = renderState->texts[i].text;
        int index = -1;
        GLfloat cursor = 0;

        //For every character 
        while(*string != '\0'){

            //Find Coorisponding ascii character for attributes
            for(int j=0; j < (*ft).chars_count; j++){
                if(string[0] == (char)(*ft).ascii_id[j]){
                    index = j;
                }
            }

            //Screen Width & Screen Height
            GLfloat screen_width = (GLfloat)width;
            GLfloat screen_height = (GLfloat)height;
            //Offsets in Screen Space
            GLfloat x_offset = ((((GLfloat)(*ft).xoffset[index]) - (GLfloat)(*ft).padding[0]) * screen_width) / (GLfloat)(*ft).scaleW;
            GLfloat y_offset = -((((GLfloat)(*ft).yoffset[index]) - (GLfloat)(*ft).padding[1]) * screen_height) / (GLfloat)(*ft).scaleH;
            //Width & Height in Screen Space
            GLfloat ws = ((GLfloat)(*ft).width[index] * screen_width) / (GLfloat)(*ft).scaleW;
            GLfloat hs = ((GLfloat)(*ft).height[index] * screen_height) / (GLfloat)(*ft).scaleH;
            //Width & Height in Texture Space
            GLfloat wt = (GLfloat)(*ft).width[index] / (GLfloat)(*ft).scaleW;
            GLfloat ht = (GLfloat)(*ft).height[index] / (GLfloat)(*ft).scaleH;
            //u & v in Texture Space
            GLfloat u = (GLfloat)(*ft).x[index] / (GLfloat)(*ft).scaleW;
            GLfloat v = (GLfloat)(*ft).y[index] / (GLfloat)(*ft).scaleH;
            //XAdvance in ScreenSpace
            GLfloat x_advance = (((*ft).xadvance[index] - 2.0f*(GLfloat)(*ft).padding[0]) * screen_width) / (*ft).scaleW;

            /*
                TO DO:
                    * Implement kernings
                    * Implement multiple lines and alignment
            */

            //Build VBO - (X,Y,Z U,(inverted)V) Currently only displaying entire texture once
            (*txt).text_buffer[0] = cursor + x_offset; 
            (*txt).text_buffer[1] = y_offset; 

            (*txt).text_buffer[2] = 0.0f;     
            
            (*txt).text_buffer[3] = u;
            (*txt).text_buffer[4] = v;


            (*txt).text_buffer[5] = cursor + x_offset;
            (*txt).text_buffer[6] = y_offset - hs;
            (*txt).text_buffer[7] = 0.0f;    
            
            (*txt).text_buffer[8] = u;
            (*txt).text_buffer[9] = v + ht;


            (*txt).text_buffer[10] = cursor + x_offset + ws; 
            (*txt).text_buffer[11] = y_offset; 
            (*txt).text_buffer[12] = 0.0f;     
            
            (*txt).text_buffer[13] = u + wt; 
            (*txt).text_buffer[14] = v;


            (*txt).text_buffer[15] = cursor + x_offset + ws;
            (*txt).text_buffer[16] = y_offset;
            (*txt).text_buffer[17] = 0.0f;    
            
            (*txt).text_buffer[18] = u + wt;
            (*txt).text_buffer[19] = v;


            (*txt).text_buffer[20] = cursor + x_offset;
            (*txt).text_buffer[21] = y_offset - hs;
            (*txt).text_buffer[22] = 0.0f; 
            
            (*txt).text_buffer[23] = u;
            (*txt).text_buffer[24] = v + ht;


            (*txt).text_buffer[25] = cursor + x_offset + ws;
            (*txt).text_buffer[26] = y_offset - hs;
            (*txt).text_buffer[27] = 0.0f;    
            
            (*txt).text_buffer[28] = u + wt;
            (*txt).text_buffer[29] = v + ht;

            cursor += x_advance;


            //Update VBO Memory
            glBindBuffer(GL_ARRAY_BUFFER, renderResources->vbos[(*txt).vbo_index]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, 6*5*sizeof(GLfloat), (*txt).text_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, 0);


            //Uniforms
            glUniformMatrix4fv(glGetUniformLocation(
                        renderResources->programs[renderState->texts[i].program_index].id, 
                        "translation"),
                        1, GL_FALSE, &renderState->texts[i].transformation[0][0]);
            glUniform1i(glGetUniformLocation(
                        renderResources->textures[renderState->texts[i].texture_index], 
                        "texture1"), 
                        0); 
            glUniform3fv(glGetUniformLocation(
                        renderResources->programs[renderState->texts[i].program_index].id, 
                        "textColor"),
                        1,
                        &renderState->texts[i].f_color[0]);
            glUniform1f(glGetUniformLocation(
                        renderResources->programs[renderState->texts[i].program_index].id, 
                        "cWidth"),
                        renderState->texts[i].c_width);
            glUniform1f(glGetUniformLocation(
                        renderResources->programs[renderState->texts[i].program_index].id, 
                        "cEdge"),
                        renderState->texts[i].c_edge);
            
            

            //Draw each character seperately
            glDrawArrays(GL_TRIANGLES, 0, 6);

            string++;
        }

        glDisable(GL_BLEND);
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
int addCamera(RenderState* renderState, int width, int height){

    if(renderState->num_cameras > MAX_CAMERAS){
        return renderState->num_cameras;
    }

    renderState->cameras[renderState->num_cameras].program_index = 0;

    //Initial Position
    renderState->cameras[renderState->num_cameras].pos = glm::vec3(50.0f, 50.0f, 0.0f);

    //Projection Matrix
    renderState->cameras[renderState->num_cameras].projection = glm::ortho<float>(0.0f, static_cast<float>(width), 0.0f,  static_cast<float>(height),  -1.0f, 1.0f);

    //View Matrix
    renderState->cameras[renderState->num_cameras].view = glm::translate(renderState->cameras[renderState->num_cameras].view, renderState->cameras[renderState->num_cameras].pos);

    return renderState->num_cameras++;
}

int addObject(RenderState* renderState){


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

int addText(RenderState* renderState){

    //Check Max Texts
    if(renderState->num_texts > MAX_TEXTS){
        return renderState->num_texts;
    }

    renderState->texts[renderState->num_texts].texture_index = 0;
    renderState->texts[renderState->num_texts].vao_index = 0;
    renderState->texts[renderState->num_texts].program_index = 0;
    renderState->texts[renderState->num_texts].fontFile_index = 0;
    renderState->texts[renderState->num_texts].text = "Text is working!";
    renderState->texts[renderState->num_texts].f_color = glm::vec3(0.0f, 1.0f, 0.0f);
    renderState->texts[renderState->num_texts].c_width = 0.47f;
    renderState->texts[renderState->num_texts].c_edge = 0.2f;

    //Initial Position
    renderState->texts[renderState->num_texts].offset = glm::vec3(0.0f, 0.0f, 0.0f);
    renderState->texts[renderState->num_texts].pos = glm::vec3(0.0f, 0.0f, 0.0f);
    renderState->texts[renderState->num_texts].scale = glm::vec3(1.0f, 1.0f, 1.0f);
    renderState->texts[renderState->num_texts].rotate = glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f));

    //Transformation Matrix
    renderState->texts[renderState->num_texts].transformation = glm::scale(renderState->texts[renderState->num_texts].transformation, renderState->texts[renderState->num_texts].scale);// Scale
    renderState->texts[renderState->num_texts].transformation = glm::rotate(renderState->texts[renderState->num_texts].transformation, renderState->texts[renderState->num_texts].rotate.x, glm::vec3(1.0f, 0.0f, 0.0f));// Rotate X
    renderState->texts[renderState->num_texts].transformation = glm::rotate(renderState->texts[renderState->num_texts].transformation, renderState->texts[renderState->num_texts].rotate.y, glm::vec3(0.0f, 1.0f, 0.0f));// Rotate Y  
    renderState->texts[renderState->num_texts].transformation = glm::rotate(renderState->texts[renderState->num_texts].transformation, renderState->texts[renderState->num_texts].rotate.z, glm::vec3(0.0f, 0.0f, 1.0f));// Rotate Z  
    renderState->texts[renderState->num_texts].transformation = glm::translate(renderState->texts[renderState->num_texts].transformation, renderState->texts[renderState->num_texts].pos);// Translate

    return renderState->num_texts++;
}

void removeCamera(RenderState* renderState, int id){
    
    //Boundary Check
    if(id >= renderState->num_cameras || id < 0){
        return;
    }

    //Delete Heap

    //Shift
    while(id != renderState->num_cameras){
        renderState->cameras[id] = renderState->cameras[id+1];
        id++;
    }
    renderState->num_cameras--;
}

void removeObject(RenderState* renderState, int id){
    
    //Boundary Check
    if(id >= renderState->num_objects || id < 0){
        return;
    }

    //Delete Heap

    //Shift
    while(id != renderState->num_objects){
        renderState->objects[id] = renderState->objects[id+1];
        id++;
    }
    renderState->num_objects--;
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