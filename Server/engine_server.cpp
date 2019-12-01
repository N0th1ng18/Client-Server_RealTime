#include "engine_server.h"
/*
    Nicholas Frey

    Server

    To do:
        * Naive Simple Connection Protocol using slots
            - Implement non-blocking sockets.
        * Could improve connection protocol security but is outside the scope of this project

*/

//Private Functions
void resetNetForce(glm::vec3* netforce);
void addForce2D(glm::vec3* netforce, float force, float theta);
void addForce3D(glm::vec3* netforce, float force, float theta, float phi);
void addForceVec(glm::vec3* netforce, float x, float y, float z);
float degreesToRadians(float degrees);

/***************************************** SERVER FUNCTIONS *******************************************/
void Engine_Server::server_Loop(NetworkState* networkState, ServerLoopState* serverLoopState, MasterGameState* masterGameState){

    double time = 0.0;
    double dt = 1000000.0 / serverLoopState->updatesPerSecond;
    

    double accumulator = 0.0;
	double alpha = 0.0;
    
    LARGE_INTEGER frequency;

    LARGE_INTEGER newCycles;
    LARGE_INTEGER oldCycles;
    LARGE_INTEGER frameTime;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&oldCycles);

    //FPS Counter
    unsigned int updateCounter = 0;
    LARGE_INTEGER fpsStartTime;
    LARGE_INTEGER fpsEndTime;
    QueryPerformanceCounter(&fpsStartTime);

    while(serverLoopState->isRunning){
        QueryPerformanceCounter(&newCycles);
        frameTime.QuadPart = ((newCycles.QuadPart - oldCycles.QuadPart) * 1000000LL) / frequency.QuadPart;
        oldCycles.QuadPart = newCycles.QuadPart;

        //Spiral of Death
        if(frameTime.QuadPart > 2500000LL){ //2.5 seconds
            frameTime.QuadPart = 2500000LL;
        }

        accumulator += (double)(frameTime.QuadPart);

        while(accumulator >= dt){
            if(Engine_Server::update(networkState, masterGameState, time)){
                serverLoopState->isRunning = false;
                break;
            }
            updateCounter++;

            time += dt / 1000000.0;
            accumulator -= dt;
        }

        //Used due to QueryPerformanceCounter being used to fast
        std::this_thread::sleep_for (std::chrono::microseconds(1));

        //UPS
        QueryPerformanceCounter(&fpsEndTime);
        if((((fpsEndTime.QuadPart - fpsStartTime.QuadPart) * 1000000LL) / frequency.QuadPart) >= 1000000LL){
            QueryPerformanceCounter(&fpsStartTime);
            std::cout << "\tUPS:\t" << updateCounter << "\tTime:\t" << time << "\n";
            updateCounter = 0;
        }

    }

}

int Engine_Server::update(NetworkState* networkState, MasterGameState* masterGameState, double time){

    //Process All Received Messages
    do{
        if(!Engine_Server::udpReceive_server(networkState, (char*)&networkState->receive_p, sizeof(Receive_P))){
            //Check if message length is greater than 0
            if(networkState->recv_msg_len > 0){
                //Check if message contains protocol ID
                switch(checkProtocol(networkState, networkState->recv_msg_len))
                {
                    case CONNECTION_REQUEST:
                    {
                        //Check for open slot
                        int client_id = Engine_Server::getAvailSlot(networkState);
                        if(client_id < 0){
                            //Server is full -> send CONNECTION_DECLINED
                            networkState->connect_response_P.MSG_TYPE = CONNECTION_DECLINED;
                            if(Engine_Server::udpSend_server(networkState, &networkState->client_address, (char*)&networkState->connect_response_P, sizeof(Connect_Response_P))){
                                std::cout << "Error: Failed to send Server Full message." << std::endl;
                                break;
                            }

                        }

                        //Add client to slot
                        networkState->slot_address[client_id].sin_addr.S_un.S_addr = networkState->client_address.sin_addr.S_un.S_addr;
                        networkState->slot_address[client_id].sin_port = networkState->client_address.sin_port;
                        networkState->is_occupied[client_id] = true;
                        //Slot is available -> send CONNECTION_ACCEPTED
                        networkState->connect_response_P.MSG_TYPE = CONNECTION_ACCEPTED;
                        if(Engine_Server::udpSend_server(networkState, &networkState->client_address, (char*)&networkState->connect_response_P, sizeof(Connect_Response_P))){
                            std::cout << "Error: Failed to send Connection Accepted message." << std::endl;
                            break;
                        }

                        //Spawn Connected Player
                        if(!masterGameState->isSpawned[client_id]){
                            
                            Player* player = addPlayer(masterGameState);
                            player->pos = glm::vec3(0.0f, 0.0f, 0.0f);
                            player->vel = glm::vec3(0.0f, 0.0f, 0.0f);
                            player->acc = glm::vec3(0.0f, 0.0f, 0.0f);
                            
                            masterGameState->isSpawned[client_id] = true;
                        }

                        std::cout << "---------------Slots---------------" << std::endl;
                        for(int i=0; i < MAX_CLIENTS; i++){
                            std::cout << i << "\t";
                            if(networkState->is_occupied[i]){
                                std::cout << "\t" << networkState->slot_address[i].sin_addr.S_un.S_addr << ":" << networkState->slot_address[i].sin_port;
                            }
                            std::cout << std::endl;
                        }
                        std::cout << "---------------Slots---------------" << std::endl << std::endl;

                        //std::cout << "CONNECTION_REQUEST" << std::endl;
                        break;
                    }
                    case INPUT_PACKET:
                    {
                        //Incoming input packets -> Update MASTER State

                        int clientID = getClientID(networkState);
                        if(clientID < 0){   //Sender needs to prove that they are connected
                            //Sender is not connected -> drop packet
                            break;
                        }
                        //Save Button Key Presses
                        char keyPresses = networkState->receive_p.buttons;
                        //W
                        if(keyPresses & (1UL << 3)){
                            masterGameState->key_W[clientID] = true;
                        }else{
                            masterGameState->key_W[clientID] = false;
                        }
                        //A
                        if(keyPresses & (1UL << 2)){
                            masterGameState->key_A[clientID] = true;
                        }else{
                            masterGameState->key_A[clientID] = false;
                        }
                        //S
                        if(keyPresses & (1UL << 1)){
                            masterGameState->key_S[clientID] = true;
                        }else{
                            masterGameState->key_S[clientID] = false;
                        }
                        //D
                        if(keyPresses & (1UL << 0)){
                            masterGameState->key_D[clientID] = true;
                        }else{
                            masterGameState->key_D[clientID] = false;
                        }
                        break;
                    }
                    case FAILED_PROTOCOL:
                    {
                        //Drop packet
                        //std::cout << "FAILED_PROTOCOL" << std::endl;
                        break;
                    }
                }
                
            }
        }

    }while(networkState->recv_msg_len >= 0 /* && receiveMessageTimeout*/);

    //Step MasterGameState
    for(int i=0; i < MAX_CLIENTS; i++){
        if(masterGameState->slotlist_players[i]){ //if player is active

            Engine_Server::Player* player = &masterGameState->players[i];
            bool w = masterGameState->key_W[i];
            bool a = masterGameState->key_A[i];
            bool s = masterGameState->key_S[i];
            bool d = masterGameState->key_D[i];
            //No movement
            if((!w && !a && !s && !d)               
                        || (w && !a && s && !d)
                        || (!w && a && !s && d)
                        || (w && a && s && d)){
                    player->mov_force = 0.0f;
                    //std::cout << "No Movement" << std::endl;
            //Up
            }else if((w && !a && !s && !d)
                        || (w && a && !s && d)){
                    player->theta = 90.0f;
                    player->mov_force = player->mov_acc;
                    //std::cout << "Up" << std::endl;
            //Left
            }else if((!w && a && !s && !d)
                        || (w && a && s && !d)){
                    player->theta = 180.0f;
                    player->mov_force = player->mov_acc;
                    //std::cout << "Left" << std::endl;
            //Down
            }else if((!w && !a && s && !d)
                        || (!w && a && s && d)){
                    player->theta = 270.0f;
                    player->mov_force = player->mov_acc;
                    //std::cout << "Down" << std::endl;
            //Right
            }else if((!w && !a && !s && d)
                        || (w && !a && s && d)){
                    player->theta = 270.0f;
                    player->mov_force = player->mov_acc;
                    //std::cout << "Right" << std::endl;
            //Up-Right
            }else if((w && !a && !s && d)){
                    player->theta = 45.0f;
                    player->mov_force = player->mov_acc;
                    //std::cout << "Up-Right" << std::endl;
            //Up-Left
            }else if((w && a && !s && !d)){
                    player->theta = 135.0f;
                    player->mov_force = player->mov_acc;
                    //std::cout << "Up-Left" << std::endl;
            //Bottom-Left
            }else if((!w && a && s && !d)){
                    player->theta = 225.0f;
                    player->mov_force = player->mov_acc;
                    //std::cout << "Bottom-Left" << std::endl;
            //Bottom-Right
            }else if((!w && !a && s && d)){
                    player->theta = 315.0f;
                    player->mov_force = player->mov_acc;
                    //std::cout << "Bottom-Right" << std::endl;
            }

            //Force
            resetNetForce(&player->netforce);
            addForce2D(&player->netforce, player->mov_force,player->theta);
            addForceVec(&player->netforce, -player->vel.x * player->mov_friction, -player->vel.y * player->mov_friction, -player->vel.z * player->mov_friction);

            //Integration
            player->acc.x = player->netforce.x / player->mass;
            player->acc.y = player->netforce.y / player->mass;
            player->acc.z = player->netforce.z / player->mass;
            player->vel.x = player->vel.x + player->acc.x;
            player->vel.y = player->vel.y + player->acc.y;
            player->vel.z = player->vel.z + player->acc.z;
            player->pos.x = player->pos.x + player->vel.x;
            player->pos.y = player->pos.y + player->vel.y;
            player->pos.z = player->pos.z + player->vel.z;

        }
    }

    //Collisions?

    //Send game state to all active clients 20 times per second
    // networkState->start_index_ptr = 0;
    // Engine_Server::package_msg((char *)PROTOCOL_ID, 5, networkState);
    // Engine_Server::package_msg((char *)MSG_GAME_PACKET, 1, networkState);
    //Number of Clients
    // char buffer[1]; //Max clients is 5 using 1 char
    // sprintf_s(buffer, "%d", masterGameState->num_players);
    // Engine_Server::package_msg(buffer, 1, networkState);

    // for(int i=0; i < MAX_CLIENTS; i++){
    //     if(masterGameState->slotlist_players[i]){ //if player is active

    //         //send gamestate to active clients
    //         //Package msg
    //         //Send

    //     }
    // }

    //Client Timeout

    return 0;
}

void Engine_Server::destroyEngine_Server(MasterGameState* masterGameState){
    destoryMasterGameState(masterGameState);
}
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SERVER FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/**************************************** NETWORK FUNCTIONS ********************************************/
int Engine_Server::udpInit(NetworkState* networkState){
    //Init WINSOCK
    WSAData wsaData;
    WORD dllVersion = MAKEWORD(2, 1);
    networkState->result = WSAStartup(dllVersion, &wsaData);
    if(networkState->result  != NO_ERROR){
        std::cout << "Error: failed to init WINSOCK" << std::endl;
        return 1;
    }

    //Socket
    std::cout << "Network: Opening Socket..." << std::endl;
    networkState->server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(networkState->server_socket == INVALID_SOCKET){
        networkState->error = WSAGetLastError();
        std::cout << "Error " << networkState->error <<": failed to create socket." << std::endl;
        return 1;
    }

    //Set Socket to Non Blocking
    unsigned long int noblock = 1;
    ioctlsocket(networkState->server_socket, FIONBIO, &noblock);

    return 0;
}
int Engine_Server::udpServerBind(NetworkState* networkState){
    networkState->server_address.sin_family = AF_INET;
    networkState->server_address.sin_addr.S_un.S_addr = htons(INADDR_ANY);
    networkState->server_address.sin_port = htons(networkState->server_port);


    std::cout << "Network: Binding..." << std::endl;
    networkState->result = bind(networkState->server_socket, (struct sockaddr*) &networkState->server_address, sizeof(networkState->server_address));
    if(networkState->result != 0){
        networkState->error = WSAGetLastError();
        std::cout << "Error " << networkState->error << ": failed to bind socket." << std::endl;
        return 1;
    }

    networkState->is_socket_open = true;
    return 0;
}
int Engine_Server::udpConnect(NetworkState* networkState){
    //Server Address
    networkState->server_address.sin_family = AF_INET;
    InetPtonW(AF_INET, networkState->address, &networkState->server_address.sin_addr.S_un.S_addr);
    networkState->server_address.sin_port = htons(networkState->server_port);
    networkState->server_address_len = sizeof(networkState->server_address);

    return 0;
}
int Engine_Server::udpSend_server(NetworkState* networkState, sockaddr_in* address, char* buffer, int size){

    if(networkState->is_socket_open == false){
        return 1;
    }

    //Send
    std::cout << "Network: Sending DataGram..." << std::endl;
    networkState->result = sendto(networkState->server_socket, buffer, size, 0, (struct sockaddr*) address, sizeof(*address));
    if(networkState->result == SOCKET_ERROR){
        networkState->error = WSAGetLastError();
        std::cout << "Error " << networkState->error << ": failed to send message." << std::endl;
        return 1;
    }
    return 0;
}
int Engine_Server::udpReceive_server(NetworkState* networkState, char* buffer, int size){

    if(networkState->is_socket_open == false){
        return 1;
    }
    
    networkState->client_address_len = sizeof(networkState->client_address);

    //std::cout << "Network: Receiving DataGram..." << std::endl;
    networkState->recv_msg_len = recvfrom(networkState->server_socket, buffer, size, 0, (struct sockaddr*) &networkState->client_address, &networkState->client_address_len);
    if(networkState->recv_msg_len == SOCKET_ERROR){
        networkState->error = WSAGetLastError();
        if(networkState->error == WSAEWOULDBLOCK){
            //Empty Socket -> continue without blocking
        }else{
            std::cout << "Error " << networkState->error << ": failed to receive message." << std::endl;
            return 1;
        }
    }
    return 0;
}
int Engine_Server::udpDisconnect(NetworkState* networkState){
        //Close Socket
    std::cout << "Network: Closing Socket..." << std::endl;
    networkState->result = closesocket(networkState->server_socket);
    if(networkState->result == SOCKET_ERROR){
        networkState->error = WSAGetLastError();
        std::cout << "Error " << networkState->error << ": failed to close socket." << std::endl;
        return 1;
    }

    networkState->is_socket_open = false;

    return 0;
}
int Engine_Server::udpCleanup(NetworkState* networkState){
    WSACleanup();
    return 0;
}

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ NETWORK FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/**************************************** MasterGameState FUNCTIONS *************************************************/

Engine_Server::Player* Engine_Server::addPlayer(MasterGameState* masterGameState){

    //Check Max
    if(masterGameState->num_players > MAX_CLIENTS){
        return NULL;
    }

    //Mark slot in slotlist
    masterGameState->slotlist_players[masterGameState->num_players] = true;

    //Return index and then increment num_cameras
    return &masterGameState->players[masterGameState->num_players++];

}
void Engine_Server::removePlayer(MasterGameState* masterGameState, int id){

    //Boundary Check
    if(id >= masterGameState->num_players || id < 0){
        return;
    }

    //Mark slot as free
    masterGameState->slotlist_players[id] = false;

    //Decrement count
    masterGameState->num_players--;
}

void Engine_Server::destoryMasterGameState(MasterGameState* masterGameState){
    delete[] masterGameState->players;

    delete[] masterGameState->slotlist_players;
}


/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ MasterGameState FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/**************************************** PROTOCOL FUNCTIONS **********************************************/
                /*
                               Connection Request
                        ________________________________
                       |          Protocol ID           |
                       |--------------------------------|
                       |       Connection Request       |
                       |________________________________|
                       

                              Connection Accepted
                        ________________________________
                       |          Protocol ID           |
                       |--------------------------------|
                       |      Connection Accepted       |
                       |________________________________|
                */

char Engine_Server::checkProtocol(NetworkState* networkState, int buffer_len){

    //Check buffer length and discard packets that are too small
    if(buffer_len < MINIMUM_PACKET_SIZE){
        //std::cout << "Packet Too Small" << std::endl;
        return -1;
    }

    //Check if protocol ID is equal
    for(int i=0; i < PROTOCOL_ID_LEN; i++){
        if(networkState->receive_p.PROTOCOL_ID[i] != PROTOCOL_ID[i]){
            //std::cout << "buffer != Protocol_id" << i << std::endl;
            return -1;
        }
    }

    //return message type
    return networkState->receive_p.MSG_TYPE;
}

/*
    Returns next open slot or -1 if no slots are available.
*/
int Engine_Server::getAvailSlot(NetworkState* networkState){

    int avail_id = -1;
    for(int i=0; i < MAX_CLIENTS; i++){
        //Check if slot is being used
        if(!networkState->is_occupied[i]){
            avail_id = i;
            return avail_id;    //Return open slot with new client_id
        //Check if address and port are equal
        }else if(networkState->slot_address[i].sin_addr.S_un.S_addr == networkState->client_address.sin_addr.S_un.S_addr 
                    && networkState->slot_address[i].sin_port == networkState->client_address.sin_port){
            avail_id = i;
            return avail_id;    //Return existing client_id
            
        }
    }

    return avail_id;    //Return -1 "no slots available"

}

int Engine_Server::getClientID(NetworkState* networkState){

    for(int i=0; i < MAX_CLIENTS; i++){
        //Check if slot is being used
        if(networkState->is_occupied[i] 
                    && networkState->slot_address[i].sin_addr.S_un.S_addr == networkState->client_address.sin_addr.S_un.S_addr 
                    && networkState->slot_address[i].sin_port == networkState->client_address.sin_port){
            return i;
        }
    }

    return -1;
}
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ PROTOCOL FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/**************************************** PRIVATE FUNCTIONS **********************************************/
 
void resetNetForce(glm::vec3* netforce){
    netforce->x = 0.0f;
    netforce->y = 0.0f;
    netforce->z = 0.0f;
}
void addForce2D(glm::vec3* netforce, float force, float theta){
    //Degrees to Radians
    theta = degreesToRadians(theta);

    netforce->x = netforce->x + (force * sin(theta));
    netforce->y = netforce->y + (force * cos(theta));
}
void addForce3D(glm::vec3* netforce, float force, float theta, float phi){
    //Degrees to Radians
    theta = degreesToRadians(theta);
    phi = degreesToRadians(phi);

    netforce->x = netforce->x + (force * sin(theta) * sin(phi));
    netforce->y = netforce->y + (force * cos(theta));
    netforce->z = netforce->z + (force * sin(theta) * cos(phi));
}

void addForceVec(glm::vec3* netforce, float x, float y, float z){

    netforce->x = netforce->x + x;
    netforce->y = netforce->y + y;
    netforce->z = netforce->z + z;
}
float degreesToRadians(float degrees){
    return (degrees  * (float)(PI / 180.0));
}

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ PRIVATE FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/