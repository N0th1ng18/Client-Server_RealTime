#include "client.h"

/*Resources
    1. https://developer.valvesoftware.com/wiki/Latency_Compensating_Methods_in_Client/Server_In-game_Protocol_Design_and_Optimization
*/

int addResources(RenderResources* renderResources);
int addEntities(Engine::WindowState* windowState, RenderState* renderState);
int addStates();

int main(){

	//Constants
	const int MAX_CAMERAS = 10;
	const int MAX_OBJECTS = 10;
	const int MAX_TEXTS = 10;
	
	//Structures
	Engine::WindowState windowState = {};
	Engine::OpenGLState openGLState = {};
	RenderResources renderResources = {}; 
	RenderState renderState = RenderState(MAX_CAMERAS,MAX_OBJECTS,MAX_TEXTS);
	
	//Setup
	windowState.isFullscreen = false;
	openGLState.isCulling = true;
	openGLState.isWireframe = false;
	openGLState.clear_Color[0] = 1.0f;

	//Engine
	if(Engine::initEngine(&windowState, &openGLState, &renderResources, &renderState)){return 1;};

	//Load
	if(addResources(&renderResources)){return 1;}
	if(addEntities(&windowState, &renderState)){return 1;}
	if(addStates()){return 1;}

	//Engine Loop
	Engine::loop(&windowState, &openGLState, &renderResources, &renderState);
	//Engine CleanUp
	Engine::destroy_Engine(&windowState, &renderResources, &renderState);

    return 0;
}


int addResources(RenderResources* renderResources){

	//Load Textures
	// if(loadTexture("Textures\\Test\\water.jpg", renderResources)){
    //     std::cout << "Failed to load Texture." << std::endl;
    //     return 1;
    // }
	if(loadTexture("Textures\\Fonts\\Candara.png", renderResources)){
        std::cout << "Failed to load Texture." << std::endl;
        return 1;
    }

	//Load VAOs
	//loadStaticObjectVAO(loadModel("", renderResources), renderResources);
    loadDynamicTextVAO(renderResources);

	//Load Shaders
	// if(loadShader("../Shaders/basic.vert","../Shaders/basic.frag", renderResources)){
    //     std::cout << "Failed to load Shader Program." << std::endl;
    //     return 1;
    // }
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

	//Create Cameras (UI Camera, Other Cameras)
	/*
		Change RenderResources arrays and RenderState's arrays to dynamic aka pointers...!!!!!!!!!!!!!!!!!!
		Then adding entities will be much easier, but will have to manage deletion to avoid memory leaks.

		Should beable to define states and load the first state for main menu. 
		then input could be overrided to define how states change
	*/

/*	OUT OF SCOPE PROBLEM -> FIX IS TO USE NEW FOR ALL THESE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

    Camera* camera0 = addCamera(renderState);
    camera0->program_index = 0;
    camera0->pos.x = 20.0f;
    camera0->pos.y = 200.0f;
    camera0->pos.z = 0.0f;
    camera0->projection = glm::ortho<float>(0.0f, static_cast<float>(windowState->width), 0.0f,  static_cast<float>(windowState->height),  -1.0f, 1.0f);
    camera0->view = glm::translate(camera0->view, camera0->pos);

    //Create Objects
	// Object* object = addObject(renderState);
    // object->program_index = 0;
    // object->vao_index = 0;
    // object->texture_index = 0;
    // object->offset = glm::vec3(0.0f, 0.0f, 0.0f);
    // object->pos = glm::vec3(0.0f, 0.0f, 0.0f);
    // object->scale = glm::vec3(200.0f, 200.0f, 1.0f);
    // object->rotate = glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f));
    // object->transformation = glm::scale(renderState->objects[renderState->num_objects].transformation, renderState->objects[renderState->num_objects].scale);// Scale
    // object->transformation = glm::rotate(renderState->objects[renderState->num_objects].transformation, renderState->objects[renderState->num_objects].rotate.x, glm::vec3(1.0f, 0.0f, 0.0f));// Rotate X
    // object->transformation = glm::rotate(renderState->objects[renderState->num_objects].transformation, renderState->objects[renderState->num_objects].rotate.y, glm::vec3(0.0f, 1.0f, 0.0f));// Rotate Y  
    // object->transformation = glm::rotate(renderState->objects[renderState->num_objects].transformation, renderState->objects[renderState->num_objects].rotate.z, glm::vec3(0.0f, 0.0f, 1.0f));// Rotate Z  
    // object->transformation = glm::translate(renderState->objects[renderState->num_objects].transformation, renderState->objects[renderState->num_objects].pos);// Translate


    //Create Texts      (read from file)  
    Text* text = addText(renderState);
	text->texture_index = 0;
    text->vao_index = 0;
	text->vbo_index = 0;
    text->program_index = 0;
    text->fontFile_index = 0;
    text->text = "TTText is awesome!";
    text->f_color.x = 0.0f;
    text->f_color.y = 1.0f;
    text->f_color.z = 0.0f;
    text->c_width = 0.47f;
    text->c_edge = 0.2f;
    text->pos.x = 0.0f;
    text->pos.y = 0.0f;
    text->pos.z = 0.0f;
    text->scale.x = 0.5f;
    text->scale.y = 0.5f;
    text->scale.z = 0.5f;
    text->rotate.x = glm::radians(0.0f);
    text->rotate.y = glm::radians(0.0f);
    text->rotate.z = glm::radians(0.0f);
    text->transformation = glm::scale(text->transformation, text->scale);// Scale
    text->transformation = glm::rotate(text->transformation, text->rotate.x, glm::vec3(1.0f, 0.0f, 0.0f));// Rotate X
    text->transformation = glm::rotate(text->transformation, text->rotate.y, glm::vec3(0.0f, 1.0f, 0.0f));// Rotate Y  
    text->transformation = glm::rotate(text->transformation, text->rotate.z, glm::vec3(0.0f, 0.0f, 1.0f));// Rotate Z  
    text->transformation = glm::translate(text->transformation, text->pos);// Translate


    std::cout << "--------------Render State-----------------" << std::endl;
    std::cout << "Num_Cameras   \t=\t" << renderState->num_cameras << std::endl;
    std::cout << "Num_Objects   \t=\t" << renderState->num_objects << std::endl;
    std::cout << "Num_Texts     \t=\t" << renderState->num_texts << std::endl;
    std::cout << "-------------------------------------------" << std::endl;

	return 0;
}

int addStates(){

	return 0;
}


