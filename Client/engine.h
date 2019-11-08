#ifndef ENGINE_H
#define ENGINE_H

//includes
#include "Windows.h"
#include <iostream>         //For console
#include <winsock2.h>       //Socket Programing
#include <Ws2tcpip.h>
#include <GL/glew.h>        //Determining which OpenGL extensions are supported on device
#include <GLFW/glfw3.h>     //Handle for the window
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "renderer.h"
#include "textureloader.h"
#include "modelloader.h"
#include "shaderloader.h"
#include "fontloader.h"

namespace Engine
{

//Structures
struct WindowState{
	int pos[2] = {0, 0};
	int width = 800;
	int height = 600;
	int viewPortSize[2] = {0, 0};
	bool isFullscreen = true;
	bool updateViewport = true;
	const GLFWvidmode* mode;
	GLFWwindow* window;
	//Inputs
	bool key_Escape = false;
	bool key_W = false;
	bool key_A = false;
	bool key_S = false;
	bool key_D = false;
};
struct OpenGLState{
    //Loop
    bool isRunning = true;
    double updatesPerSecond = 60.0;
    //OpenGLInit
	float clear_Color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	bool depth_Test = false;
	GLenum depth_Test_Type = GL_LESS;
	bool isCulling = true;
	bool isWireframe = false;
};

struct NetworkState{
	//Server
	bool isConnected = false;
	SOCKET server_socket;
	sockaddr_in server_address;
	unsigned short server_port;
};

//Main Functions
int initEngine(WindowState* windowState, OpenGLState* openGLState, RenderResources* renderResources, RenderState* renderState);
void loop(WindowState* windowState, OpenGLState* openGLState, RenderResources* renderResources, RenderState* renderState);
void input(GLFWwindow* window, int key, int scancode, int action, int mods);
void update();
void render(WindowState* windowState, RenderResources* renderResources, RenderState* renderState);
void destroy_Engine(WindowState* windowState, RenderResources* renderResources, RenderState* renderState);

//initEngine Functions
int createWindow(WindowState* windowState);
void loadWindowState(WindowState* windowState);
void loadInputState(WindowState* windowState, GLFWkeyfun glfwkeyfun);
void loadOpenGLState(OpenGLState* openGLState);

//OpenGL Functions
void updateViewport(WindowState* windowState);

//Network Functions
int udpInit(NetworkState* networkState);
int udpConnect(char* address, NetworkState* networkState);
int udpSend(char* buffer, int buffer_size, NetworkState* networkState);
int udpReceive(NetworkState* networkState);
int udpDisconnect(NetworkState* networkState);
int udpCleanup(NetworkState* networkState);



}

#endif