#include "server.h"
/*Resources
    1. https://developer.valvesoftware.com/wiki/Latency_Compensating_Methods_in_Client/Server_In-game_Protocol_Design_and_Optimization
*/


int main(){

    // Sample clock to find start time
    // Read client user input messages from network
    // Execute client user input messages
    // Simulate server-controlled objects using simulation time from last full pass
    // For each connected client, package up visible objects/world state and send to client
    // Sample clock to find end time
    // End time minus start time is the simulation time for the next frame




    std::cout << "--------------Server--------------" << std::endl;

    unsigned short port = 8081;

    int result;

    SOCKET server_socket;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int client_address_len = sizeof(client_address);

    char receive_buffer[1024];
    int receive_buffer_len = 1024;

    //Init WINSOCK
    WSAData wsaData;
    WORD dllVersion = MAKEWORD(2, 1);
    result = WSAStartup(dllVersion, &wsaData);
    if( result != NO_ERROR){
        std::cout << "Error: failed to init WINSOCK" << std::endl;
        return 1;
    }

    //Socket
    std::cout << "Server: Opening Socket..." << std::endl;
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(server_socket == INVALID_SOCKET){
        std::cout << "Error: failed to create socket" << WSAGetLastError() << std::endl;
        return 1;
    }

    //Bind
    server_address.sin_family = AF_INET;
    server_address.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);

    std::cout << "Server: Binding..." << std::endl;
    result = bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
    if( result != 0){
        std::cout << "Error: failed to bind socket " << WSAGetLastError() << std::endl;
        return 1;
    }

    //Send/Recv
    std::cout << "Server: Receiving DataGram..." << std::endl;
    result = recvfrom(server_socket, receive_buffer, receive_buffer_len, 0, (struct sockaddr*) &client_address, &client_address_len);
    if(result == SOCKET_ERROR){
        std::cout << "Error: failed to receive message " << WSAGetLastError() << std::endl;
        return 1;
    }

    //Print Results
    std::cout << "Server: Result: " << result << std::endl;
    for(int i=0; i < result; i++){
        std::cout << receive_buffer[i] << std::endl;
    }

    //Close Socket
    std::cout << "Server: Closing Socket..." << std::endl;
    result = closesocket(server_socket);
    if(result == SOCKET_ERROR){
        std::cout << "Error: failed to close socket " << WSAGetLastError() << std::endl;
        return 1;
    }

    //Terminate
    WSACleanup();
    std::cout << "Server: Terminated" << std::endl;

    return 0;
}