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
const double MASTERSTATE_RESEND_TIME = 1.0 / 20.0;	//20 times a second
const int PROTOCOL_ID_LEN = 5;
const char PROTOCOL_ID[5] = {'T','i','t','a','n'};
#define MINIMUM_PACKET_SIZE 6	//Protocol_ID(5) + MessageType(1)
#define FAILED_PROTOCOL '0'
#define CONNECTION_REQUEST '1'
#define CONNECTION_ACCEPTED '2'
#define CONNECTION_DECLINED '3'
#define INPUT_PACKET '4'
#define GAME_PACKET '5'

//Math Stuff
#define PI 3.14159265359





namespace Engine_Server
{

//Network Message Structures
//Send
struct Connect_Response_P{
	char PROTOCOL_ID[5] = {'T','i','t','a','n'};
	char MSG_TYPE;
};
struct Client_MS_P{
	float pos_x;
	float pos_y;
	float pos_z;
};
struct MasterState_P{
	char PROTOCOL_ID[5] = {'T','i','t','a','n'};
	char MSG_TYPE = '5';
	int client_id;
	int num_clients;
	Client_MS_P client_p[MAX_CLIENTS];
};
//Receive
struct Receive_P{
	char PROTOCOL_ID[5];
	char MSG_TYPE;
	char buttons;//Byte: ---- WASD
};

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

	//Packets
	Connect_Response_P connect_response_P;
	MasterState_P masterstate_p;
	Receive_P receive_p;

	//Client Connection Slots
	bool is_occupied[MAX_CLIENTS] = {false};
	sockaddr_in slot_address[MAX_CLIENTS] = {NULL};
	double send_timer = 0.0;

	//Network
	sockaddr_in client_address;
	int client_address_len = sizeof(client_address);
	int recv_msg_len;
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
int udpSend_server(NetworkState* networkState, sockaddr_in* address, char* buffer, int size);
int udpReceive_server(NetworkState* networkState, char* buffer, int size);
int udpDisconnect(NetworkState* networkState);
int udpCleanup(NetworkState* networkState);
void package_msg(char* msg, int size, NetworkState* networkState);

//Protocol Functions
char checkProtocol(NetworkState* networkState, int buffer_len);
int getAvailSlot(NetworkState* networkState);
int getClientID(NetworkState* networkState);

//MasterGameState Funcitons
Player* addPlayer(MasterGameState* masterGameState);
void removePlayer(MasterGameState* masterGameState, int id);

//Clean up
void destoryMasterGameState(MasterGameState* masterGameState);

}

#endif