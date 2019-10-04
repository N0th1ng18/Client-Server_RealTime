#include "modelloader.h"

void createStaticObject_VAO(Model* model, RenderResources* renderResources){

    //Create VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    //Create 2 VBOs
    GLuint vbo[2];
    glGenBuffers(2, vbo);
    //Setup Data VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, model->buffer_size * sizeof(GLfloat), model->buffer, GL_STATIC_DRAW);
    //Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, model->attribute_stride, 0);
    //Texture Coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, model->attribute_stride, (GLvoid*)(3 * sizeof(GLfloat)));

    //Setup Indices VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (model->indices_size) * sizeof(GLuint), model->indices, GL_STATIC_DRAW);

    //Unbind VAO
    glBindVertexArray(0);

    //Add to Render Resources
    addVAO_Elements(renderResources, vao, model->indices_size);

    delete[] model->buffer;
    delete[] model->indices;
    delete model;
}

void createDynamic2DText_VAO_VBO(RenderResources* renderResources){

    //Create VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //Create 1 VBOs
    GLuint vbo[1];
    glGenBuffers(1, vbo);

    //Setup Data VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
    //Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    //Texture Coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

    //nubind VAO
    glBindVertexArray(0);

    //Add Font to RenderState
    addVAO(renderResources, vao);
    addVBO(renderResources, vbo[0]);
}

Model* loadModel(const char *filePath, RenderResources* renderState){

    GLfloat* buffer = new GLfloat[20]{
        //Positions                             //Texture Coords (Inverted Y for opengl)
        -0.5f,  0.5f,  0.0f,    /* Top left     */  0.0f,  0.0f,    /* Top left     */
         0.5f,  0.5f,  0.0f,    /* Top right    */  1.0f,  0.0f,    /* Top right    */
        -0.5f, -0.5f,  0.0f,    /* Bottom left  */  0.0f,  1.0f,    /* Bottom left  */
         0.5f, -0.5f,  0.0f,    /* Bottom right */  1.0f,  1.0f     /* Bottom right */
    };

    GLuint* indices = new GLuint[6]{
        0, 2, 1,        //Top Triangle
        1, 2, 3         //Bottom Triangle
    };
    
    //Populate model
    Model* model = new Model();

    model->attribute_stride = 5 * sizeof(GLfloat);
    model->buffer_size = 20;
    model->indices_size = 6;
    model->buffer = buffer;
    model->indices = indices;

    return model;
}