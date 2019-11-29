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
        1) Go into engine loop and wait for connections. then start game...etc.
    */

    std::cout << "--------------Server--------------" << std::endl;

    //Structures
    Engine_Server::ServerLoopState serverLoopState = {};
    Engine_Server::NetworkState networkState = {};

    //Setup
    serverLoopState.updatesPerSecond = 1.0;

    //Network
    networkState.server_port = 8081;


    //Server
    if(Engine_Server::udpInit(&networkState)){return 1;}
    if(Engine_Server::udpServerBind(&networkState)){return 1;}

    //Run Server
    Engine_Server::server_Loop(&networkState, &serverLoopState);

    //Terminate Server
    if(Engine_Server::udpDisconnect(&networkState)){return 1;}
    if(Engine_Server::udpCleanup(&networkState)){return 1;}

    return 0;
}