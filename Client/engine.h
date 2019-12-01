#ifndef ENGINE_H
#define ENGINE_H

#define WIN32_LEAN_AND_MEAN

//includes
#include <winsock2.h>       //Socket Programing
#include <Ws2tcpip.h>
#include "Windows.h"
#include <iostream>         //For console
#include <GL/glew.h>        //Determining which OpenGL extensions are supported on device
#include <GLFW/glfw3.h>     //Handle for the window
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "renderer.h"
#include "textureloader.h"
#include "modelloader.h"
#include "shaderloader.h"
#include "fontloader.h"

#define MAX_SEND_BUF_SIZE 1024
#define MAX_RECV_BUF_SIZE 1024

#define MAX_CLIENTS 5

//Protocol
const double CONNECT_RESEND_TIME = 3.0;	//Seconds
const double CONNECT_TIMOUT = 10.0;	//Seconds
const int PROTOCOL_ID_LEN = 5;
const char PROTOCOL_ID[5] = {'T','i','t','a','n'};
#define MINIMUM_PACKET_SIZE 6	//Protocol_ID(5) + MessageType(1)
#define FAILED_PROTOCOL '0'
#define CONNECTION_REQUEST '1'
#define CONNECTION_ACCEPTED '2'
#define CONNECTION_DECLINED '3'
#define INPUT_PACKET '4'
#define GAME_PACKET '5'



namespace Engine
{

//Network Message Structures
struct Connect_P{
	char PROTOCOL_ID[5] = {'T','i','t','a','n'};
	char MSG_TYPE = '1';
};
struct Input_P{
	char PROTOCOL_ID[5] = {'T','i','t','a','n'};
	char MSG_TYPE = '4';
	char buttons;//Byte: ---- WASD
};
//Receive
struct Client_MS_P{
	float pos_x;
	float pos_y;
	float pos_z;
};
struct Receive_P{
	char PROTOCOL_ID[5] = {'T','i','t','a','n'};
	char MSG_TYPE = '5';
	int client_id;
	int num_clients;
	Client_MS_P client_p[MAX_CLIENTS];
};

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
	//Setup
	int result, error;

	//Client
	bool isConnected = false;
	double connect_timer;

	//Packets
	Connect_P connect_p;
	Input_P input_p;
	Receive_P receive_p;

	//Server
	PCWSTR address;
	unsigned short server_port;
	SOCKET server_socket;
	sockaddr_in server_address;
	int server_address_len = sizeof(server_address);
	sockaddr_in client_address;
	int client_address_len = sizeof(client_address);
	
	//Messages
	int start_index_ptr;					//stores the next open space to write msg && The total size of msg written
	char send_buffer[MAX_SEND_BUF_SIZE];
	int recv_msg_len;
	char recv_buffer[MAX_RECV_BUF_SIZE];
	
};

//Client Functions
int initEngine(WindowState* windowState, OpenGLState* openGLState, RenderResources* renderResources, RenderState* renderState);
void loop(WindowState* windowState, OpenGLState* openGLState, RenderResources* renderResources, RenderState* renderState, NetworkState* networkState);
void input(GLFWwindow* window, int key, int scancode, int action, int mods);
void update(double time, WindowState* windowState, RenderState* renderState, NetworkState* networkState);
void render(WindowState* windowState, RenderResources* renderResources, RenderState* renderState);
void destroyEngine(WindowState* windowState, RenderResources* renderResources, RenderState* renderState);

//initEngine Functions
int createWindow(WindowState* windowState);
void loadWindowState(WindowState* windowState);
void loadInputState(WindowState* windowState, GLFWkeyfun glfwkeyfun);
void loadOpenGLState(OpenGLState* openGLState);

//OpenGL Functions
void updateViewport(WindowState* windowState);

//Network Functions
int udpInit(NetworkState* networkState);
int udpConnect(NetworkState* networkState);
void udpSend_client(NetworkState* networkState, char* buffer, int size);
void udpReceive_client(NetworkState* networkState, char* buffer, int size);
int udpDisconnect(NetworkState* networkState);
int udpCleanup(NetworkState* networkState);
void package_msg(char* msg, int size, NetworkState* networkState);

//Protocol Functions
char checkProtocol(NetworkState* networkState, int buffer_len);

//Bitwise Functions
void getButtonsBitset(WindowState* windowState, NetworkState* networkState);

}

#endif