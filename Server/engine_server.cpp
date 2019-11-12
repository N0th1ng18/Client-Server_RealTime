#include "engine_server.h"

/*
    Nicholas Frey

    Server

/*


/***************************************** SERVER FUNCTIONS *******************************************/
//void server_InitEngine();
//void server_Loop();
//void server_destroyEngine();
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SERVER FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/**************************************** NETWORK FUNCTIONS *************************************************/
int Engine_Server::udpInit(NetworkState* networkState){
    //Init WINSOCK
    WSAData wsaData;
    WORD dllVersion = MAKEWORD(2, 1);
    if( WSAStartup(dllVersion, &wsaData) != NO_ERROR){
        std::cout << "Error: failed to init WINSOCK" << std::endl;
        return 1;
    }

    //Socket
    std::cout << "Network: Opening Socket..." << std::endl;
    networkState->server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(networkState->server_socket == INVALID_SOCKET){
        std::cout << "Error: failed to create socket" << WSAGetLastError() << std::endl;
        return 1;
    }

    return 0;
}
int Engine_Server::udpServerBind(NetworkState* networkState){
    networkState->server_address.sin_family = AF_INET;
    networkState->server_address.sin_addr.S_un.S_addr = htons(INADDR_ANY);
    networkState->server_address.sin_port = htons(networkState->server_port);


    std::cout << "Network: Binding..." << std::endl;
    if( bind(networkState->server_socket, (struct sockaddr*) &networkState->server_address, sizeof(networkState->server_address)) != 0){
        std::cout << "Error: failed to bind socket " << WSAGetLastError() << std::endl;
        return 1;
    }

    return 0;
}
int Engine_Server::udpConnect(NetworkState* networkState){
    //Server Address
    networkState->server_address.sin_family = AF_INET;
    InetPtonW(AF_INET, networkState->address, &networkState->server_address.sin_addr.S_un.S_addr);
    networkState->server_address.sin_port = htons(networkState->server_port);
    networkState->server_address_len = sizeof(networkState->server_address);
    networkState->isConnected = false;

    return 0;
}
int Engine_Server::udpSend_server(NetworkState* networkState, sockaddr_in* address){

    std::cout << address->sin_addr.S_un.S_addr << ":" << address->sin_port << std::endl;

    //Send
    std::cout << "Network: Sending DataGram..." << std::endl;
    if(sendto(networkState->server_socket, networkState->send_buffer, networkState->start_index_ptr, 0, (struct sockaddr*) address, sizeof(*address)) == SOCKET_ERROR){
        std::cout << "Error: failed to send message " << WSAGetLastError() << std::endl;
        return 1;
    }
    return 0;
}
int Engine_Server::udpReceive_server(NetworkState* networkState){
    std::cout << "Network: Receiving DataGram..." << std::endl;
    networkState->recv_msg_len = recvfrom(networkState->server_socket, networkState->recv_buffer, MAX_RECV_BUF_SIZE, 0, (struct sockaddr*) &networkState->client_address, &networkState->client_address_len);
    if(networkState->recv_msg_len == SOCKET_ERROR){
        std::cout << "Error: failed to receive message " << WSAGetLastError() << std::endl;
        return 1;
    }
    return 0;
}
int Engine_Server::udpDisconnect(NetworkState* networkState){
        //Close Socket
    std::cout << "Network: Closing Socket..." << std::endl;
    if(closesocket(networkState->server_socket) == SOCKET_ERROR){
        std::cout << "Error: failed to close socket " << WSAGetLastError() << std::endl;
        return 1;
    }
    networkState->isConnected = false;
    return 0;
}
int Engine_Server::udpCleanup(NetworkState* networkState){
    WSACleanup();
    return 0;
}

void Engine_Server::package_msg(char* msg, int size, int start_index, NetworkState* networkState){
    //Check if start_index is outside the bounds of the send buffer
    if(start_index < 0 || start_index + size - 1 >= MAX_SEND_BUF_SIZE - 1){
        std::cout << "ERROR: OutOfBounceException" << std::endl;
        return;
    }

    //Write message to send_buffer
    for(int i=start_index; i < start_index + size - 1; i++){
        networkState->send_buffer[i + start_index] = msg[i];
    }

    //Save start_index for future packages
    networkState->start_index_ptr = start_index + size;
}
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ NETWORK FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
