#include "engine_server.h"
/*
    Nicholas Frey

    Server

    To do:
        * Naive Simple Connection Protocol using slots
            - Implement non-blocking sockets.
        * Could improve connection protocol security but is outside the scope of this project

/*

/***************************************** SERVER FUNCTIONS *******************************************/
void Engine_Server::server_Loop(NetworkState* networkState, ServerLoopState* serverLoopState){

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
            if(Engine_Server::update(networkState, time)){
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

int Engine_Server::update(NetworkState* networkState, double time){

    //Process All Received Messages
    do{
        if(Engine_Server::udpReceive_server(networkState)){return 1;}else{
            //Check if message length is greater than 0
            if(networkState->recv_msg_len > 0){
                //Check if message contains protocol ID
                switch(checkProtocol(networkState->recv_buffer, networkState->recv_msg_len))
                {
                    case CONNECTION_REQUEST:
                    {
                        //Check for open slot
                        int avail_slot = Engine_Server::getAvailSlot(networkState);
                        if(avail_slot < 0){
                            //Server is full -> send CONNECTION_DECLINED
                            networkState->start_index_ptr = 0;
                            Engine_Server::package_msg((char *)PROTOCOL_ID, 5, networkState);
                            Engine_Server::package_msg((char *)MSG_CONNECTION_DECLINED, 1, networkState);
                            if(Engine_Server::udpSend_server(networkState, &networkState->client_address)){
                                std::cout << "Error: Failed to send Server Full message." << std::endl;
                                break;
                            }

                        }

                        //Add client to slot
                        networkState->slot_address[avail_slot].sin_addr.S_un.S_addr = networkState->client_address.sin_addr.S_un.S_addr;
                        networkState->slot_address[avail_slot].sin_port = networkState->client_address.sin_port;
                        networkState->is_occupied[avail_slot] = true;
                        //Slot is available -> send CONNECTION_ACCEPTED
                        networkState->start_index_ptr = 0;
                        Engine_Server::package_msg((char *)PROTOCOL_ID, 5, networkState);
                        Engine_Server::package_msg((char *)MSG_CONNECTION_ACCEPTED, 1, networkState);

                        if(Engine_Server::udpSend_server(networkState, &networkState->client_address)){
                            std::cout << "Error: Failed to send Connection Accepted message." << std::endl;
                            break;
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
                    case GAME_PACKET:
                    {
                        //Incoming input packets -> Update Server State
                        
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


    //While(messages exist to be read)
        //Read messages from socket
        //Process messages
        //update state

    //Step Physics!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    //Send game state to each client x times per second or every X ms

    return 0;
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
int Engine_Server::udpSend_server(NetworkState* networkState, sockaddr_in* address){

    if(networkState->is_socket_open == false){
        return 1;
    }

    //Send
    std::cout << "Network: Sending DataGram..." << std::endl;
    networkState->result = sendto(networkState->server_socket, networkState->send_buffer, networkState->start_index_ptr, 0, (struct sockaddr*) address, sizeof(*address));
    if(networkState->result == SOCKET_ERROR){
        networkState->error = WSAGetLastError();
        std::cout << "Error " << networkState->error << ": failed to send message." << std::endl;
        return 1;
    }
    return 0;
}
int Engine_Server::udpReceive_server(NetworkState* networkState){

    if(networkState->is_socket_open == false){
        return 1;
    }
    
    networkState->client_address_len = sizeof(networkState->client_address);

    //std::cout << "Network: Receiving DataGram..." << std::endl;
    networkState->recv_msg_len = recvfrom(networkState->server_socket, networkState->recv_buffer, MAX_RECV_BUF_SIZE, 0, (struct sockaddr*) &networkState->client_address, &networkState->client_address_len);
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

void Engine_Server::package_msg(char* msg, int size,NetworkState* networkState){
    //Check if start_index is outside the bounds of the send buffer
    if(networkState->start_index_ptr < 0 || networkState->start_index_ptr + size > MAX_SEND_BUF_SIZE){
        std::cout << "ERROR: OutOfBounceException" << std::endl;
        return;
    }

    //Write message to send_buffer
    for(int i=networkState->start_index_ptr, j = 0; i < networkState->start_index_ptr + size; i++, j++){
        networkState->send_buffer[networkState->start_index_ptr + j] = msg[j];
    }

    //Save start_index for future packages
    networkState->start_index_ptr += size;
}
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ NETWORK FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/**************************************** SLOT FUNCTIONS *************************************************/
//https://gafferongames.com/post/client_server_connection/
// bool Engine_Server::hasSlot(NetworkState* networkState, ULONG address, USHORT port){


// }

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SLOT FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



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

int Engine_Server::checkProtocol(char* buffer, int buffer_len){

    //Check buffer length and discard packets that are too small
    if(buffer_len < MINIMUM_PACKET_SIZE){
        //std::cout << "Packet Too Small" << std::endl;
        return -1;
    }

    //Check if protocol ID is equal
    for(int i=0; i < PROTOCOL_ID_LEN; i++){
        if(buffer[i] != PROTOCOL_ID[i]){
            //std::cout << "buffer != Protocol_id" << i << std::endl;
            return -1;
        }
    }

    //return message type
    return buffer[PROTOCOL_ID_LEN] - 48;
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
            return avail_id;
        //Check if address and port are equal
        }else if(networkState->slot_address[i].sin_addr.S_un.S_addr == networkState->client_address.sin_addr.S_un.S_addr 
                    && networkState->slot_address[i].sin_port == networkState->client_address.sin_port){
            avail_id = i;
            return avail_id;
            
        }
    }

    return avail_id;

}
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ PROTOCOL FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/