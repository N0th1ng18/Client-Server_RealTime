#include "renderer.h"

//Functions
void sendObjectUniforms(RenderResources* renderResources, RenderState* renderState, int i);

void bindProgram(GLuint program_index, RenderResources* renderResources, RenderState* renderState){
    if(program_index != renderState->bound_program_index){
        glUseProgram(renderResources->programs[program_index].id);
        renderState->bound_program_index = program_index;
    }
}
void bindVAO(GLuint vao_index, RenderResources* renderResources, RenderState* renderState){
    if(vao_index != renderState->bound_vao_index){
        glBindVertexArray(renderResources->vaos[vao_index].id);
        renderState->bound_vao_index = vao_index;
    }
}
void bindTexture(GLuint texture_index, RenderResources* renderResources, RenderState* renderState){
    if(texture_index != renderState->bound_texture_index){
        glBindTexture(GL_TEXTURE_2D, renderResources->textures[texture_index]);
        renderState->bound_texture_index = texture_index;
    }
}

//Render Cameras
void renderCameras(RenderResources* renderResources, RenderState* renderState){

    /*
        Cameras[0]      = UI Camera
        Cameras[1 -> n] = Other Cameras

        Render Only Active Camera, then UI Camera
    */
    
    /*
        Each Shader has an active camera
        Each object has an active camera and uniforms are sent for that camera...
    */

    //Only render Active Camera
    // for(int i=0; i < renderState->num_cameras; i++){
    //     if(renderState->slotlist_cameras[i] == true){

    //         //Bind Program (Check if program is already bound)
    //         bindProgram(renderState->cameras[i].program_index, renderResources, renderState);
    //         //Uniforms
    //         glUniformMatrix4fv(glGetUniformLocation(
    //                         renderResources->programs[renderState->cameras[i].program_index].id, 
    //                         "projection"),
    //                         1, GL_FALSE, &renderState->cameras[i].projection[0][0]);
    //         glUniformMatrix4fv(glGetUniformLocation(
    //                         renderResources->programs[renderState->cameras[i].program_index].id, 
    //                         "view"),
    //                         1, GL_FALSE, &renderState->cameras[i].view[0][0]);
    //     }

    // }

}

//Render
void renderObjects(RenderResources* renderResources, RenderState* renderState){

    for(int i=0; i < renderState->num_objects; i++){
        if(renderState->slotlist_cameras[i] == true){

            //Bind Program (Check if program is already bound)
            bindProgram(renderState->objects[i].program_index, renderResources, renderState);
            //Bind VAO
            bindVAO(renderState->objects[i].vao_index, renderResources, renderState);
            //Bind Texture
            bindTexture(renderState->objects[i].texture_index, renderResources, renderState);
            //Uniforms
            glUniformMatrix4fv(glGetUniformLocation(
                            renderResources->programs[renderState->objects[i].program_index].id, 
                            "projection"),
                            1, GL_FALSE, &renderState->cameras[renderState->objects[i].camera_index].projection[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(
                            renderResources->programs[renderState->objects[i].program_index].id, 
                            "view"),
                            1, GL_FALSE, &renderState->cameras[renderState->objects[i].camera_index].view[0][0]);
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
}

void renderTexts(RenderResources* renderResources, RenderState* renderState, int width, int height){

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    for(int i=0; i < renderState->num_texts; i++){
        if(renderState->slotlist_texts[i] == true){

            bindProgram(renderState->texts[i].program_index, renderResources, renderState);
            bindVAO(renderState->texts[i].vao_index, renderResources, renderState);
            bindTexture(renderState->texts[i].texture_index, renderResources, renderState);

            Text* txt = &renderState->texts[i];
            FontFile* ft = &renderResources->fontFiles[renderState->texts[i].fontFile_index];
            char* string = renderState->texts[i].text;
            int index = -1;
            GLfloat cursor = 0.0f;
            GLfloat kerning_offset = 0.0f;

            //For every character 
            while(*string != '\0'){
                
                //Find Coorisponding ascii character for attributes
                for(int j=0; j < (*ft).chars_count; j++){
                    if(string[0] == (char)(*ft).ascii_id[j]){
                        index = j;
                        break;
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
                //Calculate next Kernings
                kerning_offset = 0.0f;
                for(int j=0; j < (*ft).kernings_count; j++){
                    if(string[1] != '\0' && string[0] == (char)(*ft).first[j] && string[1] == (char)(*ft).second[j]){
                        kerning_offset = (GLfloat)(*ft).amount[j];
                    }
                }
                //XAdvance in ScreenSpace
                GLfloat x_advance = (((*ft).xadvance[index] + kerning_offset - 2.0f*(GLfloat)(*ft).padding[0]) * screen_width) / (*ft).scaleW;

                /*
                    TO DO:
                        * Implement multiple lines and alignment
                */

                //Build VBO
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
                            "projection"),
                            1, GL_FALSE, &renderState->cameras[renderState->texts[i].camera_index].projection[0][0]);
                glUniformMatrix4fv(glGetUniformLocation(
                            renderResources->programs[renderState->texts[i].program_index].id, 
                            "view"),
                            1, GL_FALSE, &renderState->cameras[renderState->texts[i].camera_index].view[0][0]);
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
Camera* addCamera(RenderState* renderState){

    //Check MAX
    if(renderState->num_objects > renderState->MAX_CAMERAS){
        return NULL;
    }

    //Mark slot in slotlist
    renderState->slotlist_cameras[renderState->num_cameras] = true;

    //Return pointer to added camera and then increment count
    return &renderState->cameras[renderState->num_cameras++];
}

Object* addObject(RenderState* renderState){

    //Check Max
    if(renderState->num_objects > renderState->MAX_OBJECTS){
        return NULL;
    }

    //Mark slot in slotlist
    renderState->slotlist_objects[renderState->num_objects] = true;

    //Return index and then increment num_cameras
    return &renderState->objects[renderState->num_objects++];
}

Text* addText(RenderState* renderState){

    //Check Max Texts
    if(renderState->num_texts > renderState->MAX_TEXTS){
        return NULL;
    }

    //Mark slot in slotlist
    renderState->slotlist_texts[renderState->num_texts] = true;

    //Return index and then increment num_cameras
    return &renderState->texts[renderState->num_texts++];
}







void removeCamera(RenderState* renderState, int id){
    
    //Boundary Check
    if(id >= renderState->num_cameras || id < 0){
        return;
    }

    //Mark slot as free
    renderState->slotlist_cameras[id] = false;

    //Decrement count
    renderState->num_cameras--;
}

void removeObject(RenderState* renderState, int id){
    
    //Boundary Check
    if(id >= renderState->num_objects || id < 0){
        return;
    }

    //Mark slot as free
    renderState->slotlist_objects[id] = false;

    //Decrement count
    renderState->num_objects--;
}

void removeText(RenderState* renderState, int id){
    //Boundary Check
    if(id >= renderState->num_texts || id < 0){
        return;
    }

    //Mark slot as free
    renderState->slotlist_texts[id] = false;

    //Decrement count
    renderState->num_texts--;
}

void destroyRenderState(RenderState* renderState){

    delete[] renderState->cameras;
    delete[] renderState->objects;
    delete[] renderState->texts;

    delete[] renderState->slotlist_cameras;
    delete[] renderState->slotlist_objects;
    delete[] renderState->slotlist_texts;

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