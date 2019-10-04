#include "textureloader.h"

GLuint loadTexture(const char *filePath, RenderResources* renderResources){

    int width,height;
    unsigned char* image;
    
    //Load Image
    image = SOIL_load_image(filePath, &width, &height, 0, SOIL_LOAD_RGBA);

    //Check if image loading failed
    if (!image){
        std::cout << "SOIL loading error: " << SOIL_last_result() << std::endl;
        return 1;
    }

    //Generate Texture
    GLuint texture;
    glGenTextures(1, &texture);

    //Bind Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    //Render Settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //Load Texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D); 

    //Free image from memory
    SOIL_free_image_data(image);

    glBindTexture(GL_TEXTURE_2D, 0);

    //Add Texture to Render Resources
    addTexture(renderResources, texture);

    return 0;
}