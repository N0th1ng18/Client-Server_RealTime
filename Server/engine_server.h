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

//Used due to the lack of rendering the frame causing query performance counter to be called to fast.
#include <thread>         
#include <chrono>        

#define MAX_SEND_BUF_SIZE 1024
#define MAX_RECV_BUF_SIZE 1024

#define MAX_CLIENTS 5

//Protocol
const int PROTOCOL_ID_LEN = 5;
const char PROTOCOL_ID[5] = {'T','i','t','a','n'};
#define MINIMUM_PACKET_SIZE 6	//Protocol_ID(5) + MessageType(1)
#define FAILED_PROTOCOL -1
#define CONNECTION_REQUEST 1
#define CONNECTION_ACCEPTED 2



namespace Engine_Server
{
//Structures

struct ServerLoopState{
    //Loop
    bool isRunning = true;
    double updatesPerSecond = 60.0;
};

struct NetworkState{
	//Setup
	int result, error;
	//Server
	bool is_socket_open = false;
	PCWSTR address;
	unsigned short server_port;
	SOCKET server_socket;
	sockaddr_in server_address;
	int server_address_len = sizeof(server_address);
	//Client Connection Slots
	int slot_id[MAX_CLIENTS];
	sockaddr_in slot_address[MAX_CLIENTS];
	//Network
	sockaddr_in client_address;
	int client_address_len = sizeof(client_address);
	int start_index_ptr;								//stores the next open space to write msg && The total size of msg written
	char send_buffer[MAX_SEND_BUF_SIZE];
	int recv_msg_len;
	char recv_buffer[MAX_RECV_BUF_SIZE];
};


struct MasterGameState{

};

//Server Functions
void server_Loop(NetworkState* networkState, ServerLoopState* serverLoopState);
int update(NetworkState* networkState, double time);

//Note: 1400 bytes is the max bytes per message

//Slot Functions
//bool hasSlot(NetworkState* networkState, ULONG address, USHORT port);
//int getNextAvailableSlot()
//void setSlot(int id, sockaddr_in* address)

//Network Functions
int udpInit(NetworkState* networkState);
int udpServerBind(NetworkState* networkState);
int udpConnect(NetworkState* networkState);
int udpSend_server(NetworkState* networkState, sockaddr_in* address);
int udpReceive_server(NetworkState* networkState);
int udpDisconnect(NetworkState* networkState);
int udpCleanup(NetworkState* networkState);
void package_msg(char* msg, int size, int start_index, NetworkState* networkState);

//Protocol Functions
int checkProtocol(char* buffer, int buffer_len);

}

#endif