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


    /*
        To Do:
        1) Think about how to structure windowState, openGLState, NetworkState, RenderResources, and RenderState
    */

    std::cout << "--------------Server--------------" << std::endl;

    //Structures
    Engine_Server::NetworkState networkState = {};

    //Setup


    //Network
    networkState.server_port = 8081;


    //Should be called in update function in main loop
    if(Engine_Server::udpInit(&networkState)){return 1;}
    if(Engine_Server::udpServerBind(&networkState)){return 1;}
    if(Engine_Server::udpReceive_server(&networkState)){return 1;}else{
        
        //Receive Needs to save connection info

        //Print Results
        std::cout << "Server: Result: " << networkState.recv_msg_len << std::endl;
        for(int i=0; i < networkState.recv_msg_len; i++){
            std::cout << networkState.recv_buffer[i];
        }
        std::cout << std::endl;

        //Package Msg
        char* msg = "Server Saying: Ok!";
        int msg_len = 18;
        package_msg(msg, msg_len, 0, &networkState);
    }
    if(Engine_Server::udpSend_server(&networkState, &networkState.client_address)){return 1;}
    if(Engine_Server::udpDisconnect(&networkState)){return 1;}
    if(Engine_Server::udpCleanup(&networkState)){return 1;}

    return 0;
}