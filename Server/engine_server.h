#ifndef ENGINE_SERVER_H
#define ENGINE_SERVER_H

#define WIN32_LEAN_AND_MEAN

//includes
#include <winsock2.h>       //Socket Programing
#include <Ws2tcpip.h>
#include "Windows.h"
#include <iostream>         //For console
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define MAX_SEND_BUF_SIZE 1024
#define MAX_RECV_BUF_SIZE 1024

namespace Engine_Server
{
//Structures

struct NetworkState{
	//Client
	bool isConnected = false;
	//Server
	PCWSTR address;
	unsigned short server_port;
	SOCKET server_socket;
	sockaddr_in server_address;
	int server_address_len = sizeof(server_address);
	sockaddr_in client_address;
	int client_address_len = sizeof(client_address);

	//Client Address array
	
	//Messages
	int start_index_ptr;					//stores the next open space to write msg && The total size of msg written
	char send_buffer[MAX_SEND_BUF_SIZE];
	int recv_msg_len;
	char recv_buffer[MAX_RECV_BUF_SIZE];
	
};

//Server Functions
//void server_InitEngine();
//void server_Loop();
//void server_destroyEngine();


//Network Functions
int udpInit(NetworkState* networkState);
int udpServerBind(NetworkState* networkState);
int udpConnect(NetworkState* networkState);
int udpSend_server(NetworkState* networkState, sockaddr_in* address);
int udpReceive_server(NetworkState* networkState);
int udpDisconnect(NetworkState* networkState);
int udpCleanup(NetworkState* networkState);
void package_msg(char* msg, int size, int start_index, NetworkState* networkState);

}

#endif