#include "client.h"

/*Resources
    1. https://developer.valvesoftware.com/wiki/Latency_Compensating_Methods_in_Client/Server_In-game_Protocol_Design_and_Optimization
*/

int loadResources(RenderResources* renderResources);
int addEntities(Engine::WindowState* windowState, RenderState* renderState);    //convert to loadStates()

int main(){

	//Constants
	const int MAX_CAMERAS = 10;
    const int MAX_PLAYERS = 5;
	const int MAX_OBJECTS = 10;
	const int MAX_TEXTS = 10;
	
	//Structures
	Engine::WindowState windowState = {};

	Engine::OpenGLState openGLState = {};
    Engine::NetworkState networkState = {};   //We think this over writes memory somehow
    createQueue(&networkState.input_queue, 64);
	RenderResources renderResources = {}; 
	RenderState renderState = RenderState(MAX_CAMERAS, MAX_PLAYERS, MAX_OBJECTS, MAX_TEXTS);
	
	//Setup
	windowState.isFullscreen = false;
	openGLState.isCulling = true;
	openGLState.isWireframe = false;
	openGLState.clear_Color[2] = 1.0f;
    renderState.clientState = CONNECT_TO_SERVER;       //Initial State
    openGLState.updatesPerSecond = 60.0;

    //Network
    networkState.address = L"192.168.1.3";    //Using Ipconfig
    networkState.server_port = 8081;

	//Engine
	if(Engine::initEngine(&windowState, &openGLState, &renderResources, &renderState)){return 1;};

	//Load
	if(loadResources(&renderResources)){return 1;}
    //loadStates()                                           
	if(addEntities(&windowState, &renderState)){return 1;}    

	//Engine Loop
	Engine::loop(&windowState, &openGLState, &renderResources, &renderState, &networkState);
	//Engine CleanUp
	Engine::destroyEngine(&windowState, &renderResources, &renderState);

    return 0;
}

int loadResources(RenderResources* renderResources){

	//Load Textures
	if(loadTexture("Textures\\Test\\water.jpg", renderResources)){
        std::cout << "Failed to load Texture." << std::endl;
        return 1;
    }
	if(loadTexture("Textures\\Fonts\\Candara.png", renderResources)){
        std::cout << "Failed to load Texture." << std::endl;
        return 1;
    }

	//Load VAOs
	loadStaticObjectVAO(loadModel("", renderResources), renderResources);
    loadDynamicTextVAO(renderResources);

	//Load Shaders
	if(loadShader("../Shaders/basic.vert","../Shaders/basic.frag", renderResources)){
        std::cout << "Failed to load Shader Program." << std::endl;
        return 1;
    }
    if(loadShader("../Shaders/text2d.vert","../Shaders/text2d.frag", renderResources)){
        std::cout << "Failed to load Shader Program." << std::endl;
        return 1;
    }

	//Load Font Files
	if(loadFontFile("Textures\\Fonts\\Candara.fnt", renderResources)){
        std::cout << "Failed to load Font." << std::endl;
        return 1;
    }

	std::cout << "------------Render Resources---------------" << std::endl;
    std::cout << "Num_Programs  \t=\t" << renderResources->num_programs << std::endl;
    std::cout << "Num_VAOs      \t=\t" << renderResources->num_vaos << std::endl;
    std::cout << "Num_VBOs      \t=\t" << renderResources->num_vbos << std::endl;
    std::cout << "Num_Textures  \t=\t" << renderResources->num_textures << std::endl;
    std::cout << "Num_FontFiles \t=\t" << renderResources->num_fontFiles << std::endl;
	std::cout << "-------------------------------------------" << std::endl;

	return 0;
}
int addEntities(Engine::WindowState* windowState, RenderState* renderState){

    //Scene
    Camera* camera0 = addCamera(renderState);
    camera0->pos = glm::vec3(static_cast<float>(windowState->width) / 2.0f, static_cast<float>(windowState->height) / 2.0f, 0.0f);
    camera0->projection = glm::ortho<float>(0.0f, static_cast<float>(windowState->width), 0.0f,  static_cast<float>(windowState->height),  -1.0f, 1.0f);
    camera0->view = glm::translate(camera0->view, camera0->pos);

    //Create Objects
	// Object* object = addObject(renderState);
    // object->program_index = 0;
    // object->vao_index = 0;
    // object->texture_index = 0;
    // object->camera_index = 0;
    // object->offset = glm::vec3(0.0f, 0.0f, 0.0f);
    // object->pos = glm::vec3(0.5f, 0.5f, 0.0f);
    // object->scale = glm::vec3(800.0f, 800.0f, 1.0f);
    // object->rotate = glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f));
    // object->transformation = glm::scale(object->transformation, object->scale);// Scale
    // object->transformation = glm::rotate(object->transformation, object->rotate.x, glm::vec3(1.0f, 0.0f, 0.0f));// Rotate X
    // object->transformation = glm::rotate(object->transformation, object->rotate.y, glm::vec3(0.0f, 1.0f, 0.0f));// Rotate Y  
    // object->transformation = glm::rotate(object->transformation, object->rotate.z, glm::vec3(0.0f, 0.0f, 1.0f));// Rotate Z  
    // object->transformation = glm::translate(object->transformation, object->pos);// Translate


    //Create Texts    
    // Text* text = addText(renderState);
    // text->texture_index = 1;
    // text->vao_index = 1;
    // text->vbo_index = 0;
    // text->program_index = 1;
    // text->fontFile_index = 0;
    // text->camera_index = 0;
    // text->text = "Text is awesome!";
    // text->f_color = glm::vec3(0.0f, 0.0f, 0.0f);
    // text->c_width = 0.47f;
    // text->c_edge = 0.2f;
    // text->pos = glm::vec3(200.0f, 200.0f, 0.0f);
    // text->scale = glm::vec3(0.5f, 0.5f, 0.5f);
    // text->rotate = glm::vec3(glm::radians(0.0f),glm::radians(0.0f),glm::radians(0.0f));
    // text->transformation = glm::scale(text->transformation, text->scale);// Scale
    // text->transformation = glm::rotate(text->transformation, text->rotate.x, glm::vec3(1.0f, 0.0f, 0.0f));// Rotate X
    // text->transformation = glm::rotate(text->transformation, text->rotate.y, glm::vec3(0.0f, 1.0f, 0.0f));// Rotate Y  
    // text->transformation = glm::rotate(text->transformation, text->rotate.z, glm::vec3(0.0f, 0.0f, 1.0f));// Rotate Z  
    // text->transformation = glm::translate(text->transformation, text->pos);// Translate

    std::cout << "--------------Render State-----------------" << std::endl;
    std::cout << "Num_Cameras   \t=\t" << renderState->num_cameras << std::endl;
    std::cout << "Num_Objects   \t=\t" << renderState->num_objects << std::endl;
    std::cout << "Num_Texts     \t=\t" << renderState->num_texts << std::endl;
    std::cout << "-------------------------------------------" << std::endl;

	return 0;
}


