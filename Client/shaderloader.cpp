#include "shaderloader.h"

//Resources: https://www.khronos.org/opengl/wiki/Shader_Compilation

std::string readShader(const char *filePath) {
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);

    if(!fileStream.is_open()) {
        std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        return "";
    }

    std::string line = "";
    while(!fileStream.eof()) {
        std::getline(fileStream, line);
        content.append(line + "\n");
    }

    fileStream.close();
    return content;
}


GLuint loadShader(const char *vertex_path, const char *fragment_path, RenderResources* renderResources) {
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
                                    /* GL_TESS_CONTROL_SHADER
                                       GL_TESS_EVALUATION_SHADER
                                       GL_GEOMETRY_SHADER
                                       GL_COMPUTE_SHADER */

    // Load Vertex Shaders
    std::string vertShaderStr = readShader(vertex_path);
    
    // Load Layout and Uniforms variables
    std::string temp;
    std::stringstream stream(vertShaderStr);
    while(!stream.eof()){
        stream >> temp;

        if(temp == "layout"){
            //Add new layout to program structure

            while(stream >> temp){

                if(temp.substr(0,10) == "(location="){

                    //Get Location
                    size_t equal = temp.find_first_of('=');
                    size_t closeP = temp.find_last_of(')');
                    temp = temp.substr(equal+1, closeP - (equal+1));
                    renderResources->programs[renderResources->num_programs].layout_locations.push_back(std::stoi(temp));

                }else if(temp == "in"){

                    //Get Type
                    stream >> temp;
                    renderResources->programs[renderResources->num_programs].layout_types.push_back(temp);

                }else if(temp.substr(temp.size()-1, temp.size()) == ";"){

                    //Get Name
                    temp = temp.substr(0, temp.find(';'));
                    renderResources->programs[renderResources->num_programs].layout_names.push_back(temp);
                    renderResources->programs[renderResources->num_programs].num_layouts++;
                    break;
                }
            }

        }else if(temp == "uniform"){
            //Add new uniform to program structure

            //Get Type
            stream >> temp;
            renderResources->programs[renderResources->num_programs].uniform_vert_types.push_back(temp);

            //Get Name
            stream >> temp;
            temp = temp.substr(0, temp.find(';'));
            renderResources->programs[renderResources->num_programs].uniform_vert_names.push_back(temp);
            renderResources->programs[renderResources->num_programs].num_vert_uniforms++;
        }
    }
    stream.clear();

    // Compile vertex shader
    const char *vertShaderSrc = vertShaderStr.c_str();
    glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
    glCompileShader(vertShader);

    // Check vertex shader
    GLint maxLength = 0;
    GLint isCompiled = 0;
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE){
        glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> vertErrorLog(maxLength);
        glGetShaderInfoLog(vertShader, maxLength, &maxLength, &vertErrorLog[0]);
        std::cout << &vertErrorLog[0] << std::endl;
        return 1;
    }
    
    // Load Fragment Shader
    std::string fragShaderStr = readShader(fragment_path);

    // Load Uniforms
    stream.str(fragShaderStr);
    while(!stream.eof()){
        stream >> temp;

        if(temp == "uniform"){
            //Add new uniform to program structure

            //Get Type
            stream >> temp;
            renderResources->programs[renderResources->num_programs].uniform_frag_types.push_back(temp);

            //Get Name
            stream >> temp;
            temp = temp.substr(0, temp.find(';'));
            renderResources->programs[renderResources->num_programs].uniform_frag_names.push_back(temp);
            renderResources->programs[renderResources->num_programs].num_frag_uniforms++;
        }
    }

    // Compile fragment shader
    const char *fragShaderSrc = fragShaderStr.c_str();
    glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
    glCompileShader(fragShader);

    // Check fragment shader
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE){
        glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> fragErrorLog(maxLength);
        glGetShaderInfoLog(fragShader, maxLength, &maxLength, &fragErrorLog[0]);
        std::cout << &fragErrorLog[0] << std::endl;
        return 1;
    }

    // Create Program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    // Link Program
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if(isLinked == GL_FALSE){
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> linkErrorLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &linkErrorLog[0]);
        std::cout << &linkErrorLog[0] << std::endl;
        return 1;
    }

    const GLchar* vPos = "vPos";
    const GLchar* vTex = "vTex";

    // Assign Attrib Locations
    glUseProgram(program);
    std::vector<int>::iterator i;
    std::vector<std::string>::iterator j;
    for(i = renderResources->programs[renderResources->num_programs].layout_locations.begin(), 
        j = renderResources->programs[renderResources->num_programs].layout_names.begin();
        i != renderResources->programs[renderResources->num_programs].layout_locations.end() &&
        j != renderResources->programs[renderResources->num_programs].layout_names.end();
        ++i, ++j
        ){
        
        glBindAttribLocation(program, *i, (*j).c_str());
    }
    glUseProgram(0);

    // Clean Up
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    //Add Program to RenderResources
    renderResources->programs[renderResources->num_programs].programID = program;

    addProgram(renderResources);

    return 0;
}

void deleteShader(Program* program){

}