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
const double SERVER_OUTPUT_DELAY = 1.0 / 20.0;	//20 times a second
const int PROTOCOL_ID_LEN = 5;
const char PROTOCOL_ID[5] = {'T','i','t','a','n'};
const char MSG_CONNECTION_ACCEPTED[1] = {'2'};
const char MSG_CONNECTION_DECLINED[1] = {'3'};
const char MSG_GAME_PACKET[1] = {'5'};
#define MINIMUM_PACKET_SIZE 6	//Protocol_ID(5) + MessageType(1)
#define FAILED_PROTOCOL -1
#define CONNECTION_REQUEST 1
#define CONNECTION_ACCEPTED 2
#define INPUT_PACKET 4

#define PI 3.14159265359





namespace Engine_Server
{

//Structures

//Network Message Structures
struct UserInput{
	char PROTOCOL_ID[5] = {'T','i','t','a','n'};
	char MSG_INPUT_PACKET[1] = {'4'};
	char buttons;	//Byte: ---- WASD
};

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
	bool is_occupied[MAX_CLIENTS] = {false};
	sockaddr_in slot_address[MAX_CLIENTS] = {NULL};
	double send_timer;

	//Network
	sockaddr_in client_address;
	int client_address_len = sizeof(client_address);
	int start_index_ptr;								//stores the next open space to write msg && The total size of msg written
	char send_buffer[MAX_SEND_BUF_SIZE];
	int recv_msg_len;
	char recv_buffer[MAX_RECV_BUF_SIZE];
};

struct Player{
	float mass = 100.0f;
	float mov_friction = 0.2f;
	float mov_acc = 10.0f;
	float mov_force = 0.0f;
	float theta = 0.0f;
	float phi = 0.0f;

    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 vel = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 acc = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 netforce = glm::vec3(0.0f, 0.0f, 0.0f);

};

struct MasterGameState{

	MasterGameState(){
		//Save Size of Arrays
		//MAX_PLAYERS = MAX_CLIENTS

		//Allocate Entities on Heap
		players = new Player[MAX_CLIENTS];

		//Initialize
		for(int i=0; i < MAX_CLIENTS; i++){
			players[i] = {};
		}

		//Create slot arrays
		slotlist_players = new bool[MAX_CLIENTS];

		//Initialize
		for(int i=0; i < MAX_CLIENTS; i++){
			slotlist_players[i] = false;
		}

	}

	//input
	bool key_W[MAX_CLIENTS] = {false};
	bool key_A[MAX_CLIENTS] = {false};
	bool key_S[MAX_CLIENTS] = {false};
	bool key_D[MAX_CLIENTS] = {false};

	//Players
	bool isSpawned[MAX_CLIENTS] = {false};
	Player* players;
	bool* slotlist_players;
	int num_players = 0;

};

//Server Functions
void server_Loop(NetworkState* networkState, ServerLoopState* serverLoopState, MasterGameState* masterGameState);
int update(NetworkState* networkState, MasterGameState* masterGameState, double time);
void destroyEngine_Server(MasterGameState* masterGameState);

//Note: 1400 bytes is the max bytes per message

//Network Functions
int udpInit(NetworkState* networkState);
int udpServerBind(NetworkState* networkState);
int udpConnect(NetworkState* networkState);
int udpSend_server(NetworkState* networkState, sockaddr_in* address);
int udpReceive_server(NetworkState* networkState);
int udpDisconnect(NetworkState* networkState);
int udpCleanup(NetworkState* networkState);
void package_msg(char* msg, int size, NetworkState* networkState);

//Protocol Functions
int checkProtocol(char* buffer, int buffer_len);
int getAvailSlot(NetworkState* networkState);
int getClientID(NetworkState* networkState);

//MasterGameState Funcitons
Player* addPlayer(MasterGameState* masterGameState);
void removePlayer(MasterGameState* masterGameState, int id);

//Clean up
void destoryMasterGameState(MasterGameState* masterGameState);

}

#endif