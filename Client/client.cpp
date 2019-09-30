#include "client.h"

/*Resources
    1. https://developer.valvesoftware.com/wiki/Latency_Compensating_Methods_in_Client/Server_In-game_Protocol_Design_and_Optimization
*/

int main(){

	//Structures
	Engine::WindowState windowState = {};
	Engine::OpenGLState openGLState = {};
	RenderResources renderResources = {}; 
	RenderState renderState = {};

	//Setup
	windowState.isFullscreen = false;
	openGLState.isCulling = true;

	//Engine
	if(Engine::initEngine(&windowState, &openGLState, &renderResources, &renderState)){return 1;};
	Engine::loop(&windowState, &openGLState, &renderResources, &renderState);
	Engine::destroy_Engine(&windowState, &renderResources);

    return 0;
}

