#include "engine.h"

#include <bitset>   //Delete this
/*
    Nicholas Frey

    Notes:
        *   - Server Max players (maybe 4)
        *   - Multiplayer Only - first thing is connect to a server.
        *   - Menu Loader         
        *   - Dynamic VBOs vs Static VBOs (All Static Objects in 1 vbo)(All dynamic objects in 1 vbo) - Optimization
        *   - Integration and Update 
        *   - Clean up Program uniform structure


    To do:
        * Client-Side Setup
        *   - Engine Setup                                                                                  -Done       (Clean up destruction of engine)
        *   - Window Setup                                                                                  -Done
        *   - OpenGL Setup                                                                                  -Done
        *   - Renderer Setup                                                                                -Done
        *   - Model Loader                                                                                  -Done       (File import from 3rd party modeling program not supported)
        *   - Program Loader                                                                                -Done       (Clean up auto uniform detection)
        *   - Texture Loader                                                                                -Done
        *   - Process Inputs                                                                                -Done
        *   - Cameras(Projection and view matrix uniforms)                                                  -Done       (Scene defines when different types of cameras are used and when certain things are rendered)
        *   - Add and Remove Objects, Cameras, etc.                                                         -Done       (Need to remove Texts)
        *   - Distance Field Text                                                                           -Done       (Alignment and Line Length)  
        *   - Make Resources and State seperate from engine                                                 -Done     
        *   - Scene States                                                                                  -Not in project scope 
        *   - Buttons                                                                                       -Not in project scope    
        *   - First Page - Play Online, Settings, Quit                                                      -Not in project scope
        *   - Play Online - Host Server, Join By Ip, Back                                                   -Not in project scope 
        *   - Settings - Video, Audio, Back                                                                 -Not in project scope 
        *   - Quit - Exits                                                                                  -Not in project scope 
        *   - Menu Page Loader                                                                              -Not in project scope            
        * Client-Server Setup and test
        *   - Setup Basic Connection                                                                        -Done       (Questions to Answer: Create a timer,threads, How to message ping time)              
        *   - Setup Ping Delay and Random Drop for testing                          (1 day)
        *   - Setup Dumb Client                                                                             -Done       (send timestamp to limit out of order packets)
        *   - Setup Client-Side Prediction                                          (2 day)
        *   - Setup Server Reconciliation                                           (2 day)
        *   - Setup Entity Interpolation                                            (1 day)
        *   - Setup Lag Compensation                                                (2 day)
        * Water/Fluid Dynamics
        *   - Simulation On Client-Side                                             (tbd)                   -Not in project scope 
        *   - Simulation Over Client-Server                                         (tbd)                   -Not in project scope 
        *   - Simulation with Prediction and Lag Compensation                       (tbd)                   -Not in project scope 
    
        


*/

//Private Functions
void resetNetForce(glm::vec3* netforce);
void addForce2D(glm::vec3* netforce, float force, float theta);
void addForce3D(glm::vec3* netforce, float force, float theta, float phi);
void addForceVec(glm::vec3* netforce, float x, float y, float z);
float degreesToRadians(float degrees);

/***************************************** CLIENT FUNCTIONS *******************************************/

int Engine::initEngine(WindowState* windowState, OpenGLState* openGLState, RenderResources* renderResources, RenderState* renderState){

    //Initialize and load all states
	if (!glfwInit()){std::cout << "Failed to initialize GLFW\n"; return 1;}
    createWindow(windowState);
    loadWindowState(windowState);
    loadInputState(windowState, Engine::input);
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {std::cout << "Failed to initialize GLEW\n"; return 1;}
    loadOpenGLState(openGLState);

    return 0;
}

void Engine::loop(WindowState* windowState, OpenGLState* openGLState, RenderResources* renderResources, RenderState* renderState, NetworkState* networkState){

    double time = 0.0;
    double dt = 1000000.0 / openGLState->updatesPerSecond;
    
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
    unsigned int renderCounter = 0;
    LARGE_INTEGER fpsStartTime;
    LARGE_INTEGER fpsEndTime;
    QueryPerformanceCounter(&fpsStartTime);

    while(openGLState->isRunning && !glfwWindowShouldClose(windowState->window)){
        QueryPerformanceCounter(&newCycles);
        frameTime.QuadPart = ((newCycles.QuadPart - oldCycles.QuadPart) * 1000000LL) / frequency.QuadPart;
        oldCycles.QuadPart = newCycles.QuadPart;

        //Spiral of Death
        if(frameTime.QuadPart > 2500000LL){ //2.5 seconds
            frameTime.QuadPart = 2500000LL;
        }

        accumulator += (double)(frameTime.QuadPart);

        //Input
        glfwPollEvents();

        while(accumulator >= dt){
            //Update
            Engine::update(time, windowState, renderState, networkState);
            updateCounter++;

            time += dt / 1000000.0;
            accumulator -= dt;
        }

        //Render
        alpha = accumulator / dt;
        Engine::render(windowState, renderResources, renderState);
        renderCounter++;

        //FPS
        QueryPerformanceCounter(&fpsEndTime);
        if((((fpsEndTime.QuadPart - fpsStartTime.QuadPart) * 1000000LL) / frequency.QuadPart) >= 1000000LL){
            QueryPerformanceCounter(&fpsStartTime);
            std::cout << "FPS:\t" << renderCounter << "\tUPS:\t" << updateCounter << "\tTime:\t" << time << "\n";
            updateCounter = 0;
            renderCounter = 0;
        }

    }

}

void Engine::input(GLFWwindow* window, int key, int scancode, int action, int mods){

    //Get window Data from window
    void* data = glfwGetWindowUserPointer(window);
    WindowState* windowState = static_cast<WindowState*>(data);

    switch(key){
        case GLFW_KEY_W:
        {
            if(action == GLFW_PRESS){
                windowState->key_W = true;
            }else if(action == GLFW_RELEASE){
                windowState->key_W = false;
            }
            break;
        }
        case GLFW_KEY_A:
        {
            if(action == GLFW_PRESS){
                windowState->key_A = true;
            }else if(action == GLFW_RELEASE){
                windowState->key_A = false;
            }
            break;
        }
        case GLFW_KEY_S:
        {
            if(action == GLFW_PRESS){
                windowState->key_S = true;
            }else if(action == GLFW_RELEASE){
                windowState->key_S = false;
            }
            break;
        }
        case GLFW_KEY_D:
        {
            if(action == GLFW_PRESS){
                windowState->key_D = true;
            }else if(action == GLFW_RELEASE){
                windowState->key_D = false;
            }
            break;
        }
        case GLFW_KEY_ESCAPE:
        {
            if(action == GLFW_PRESS){
                windowState->key_Escape = true;
            }else if(action == GLFW_RELEASE){
                windowState->key_Escape = false;
            }
            break;
        }
    }

    // std::cout << "W: " << windowState->key_W << std::endl;
    // std::cout << "A: " << windowState->key_A << std::endl;
    // std::cout << "S: " << windowState->key_S << std::endl;
    // std::cout << "D: " << windowState->key_D << std::endl;
    // std::cout << "Escape: " << windowState->key_Escape << std::endl;

}

void Engine::update(double time, WindowState* windowState, RenderState* renderState, NetworkState* networkState){

    switch(renderState->clientState){
        case CONNECT_TO_SERVER:
        {
            //Initialize
            if(Engine::udpInit(networkState)){
                renderState->clientState = FAILED_TO_CONNECT; 
                break;
            }
            //Connect
            if(Engine::udpConnect(networkState)){
                renderState->clientState = FAILED_TO_CONNECT; 
                break;
            }

            //Set Connect Timer
            networkState->connect_timer = time;

            //Packet Setup
            networkState->connect_p.time = time;

            //Send Inital message to implicitly bind the socket (sendto)
            std::cout << "Client: Connection Request" << std::endl;
            Engine::udpSend_client(networkState, (char*)&networkState->connect_p, sizeof(Connect_P));

            //Set State to Connecting
            renderState->clientState = CONNECTING;
            break; 
        }
        case CONNECTING:
        {

            //Check connection timout
            if(time - networkState->connect_timer > CONNECT_TIMOUT){
                renderState->clientState = FAILED_TO_CONNECT;
                break;
            }

            //Resend Connection Request
            if(time - networkState->connect_timer > CONNECT_RESEND_TIME){
                networkState->connect_timer = time;

                networkState->connect_p.time = time;
                std::cout << "Client: Connection Request" << std::endl;
                Engine::udpSend_client(networkState, (char*)&networkState->connect_p, sizeof(Connect_P));
            }

            Engine::udpReceive_client(networkState, (char*)&networkState->receive_p, sizeof(Receive_P));
            //Check if message length is greater than 0
            if(networkState->recv_msg_len > 0){
                switch(checkProtocol(networkState, networkState->recv_msg_len))
                {
                    case CONNECTION_ACCEPTED:
                    {
                        std::cout << "Client: Connection Accepted" << std::endl;
                        renderState->client_id = networkState->receive_p.client_id;
                        renderState->clientState = CONNECTED;
                        break;
                    }
                    case CONNECTION_DECLINED:
                    {
                        std::cout << "Client: Connection Failed" << std::endl;
                        renderState->clientState = FAILED_TO_CONNECT;
                        break;
                    }
                    case FAILED_PROTOCOL:
                    {
                        //Drop Packet
                        break;
                    }
                }
            }
            break;
        }
        case FAILED_TO_CONNECT:
        {
            std::cout << "Client: Connection Failed" << std::endl;
            break;
        }
        case CONNECTED:
        {
            //Send Input Packet to server
            getButtonsBitset(windowState, networkState);
            networkState->input_p.time = time;
            Engine::udpSend_client(networkState, (char*)&networkState->input_p, sizeof(Input_P));

            //Create Move from input packet
            Move input;
            input.time = networkState->input_p.time;
            input.key_W = windowState->key_W;
            input.key_A = windowState->key_A;
            input.key_S = windowState->key_S;
            input.key_D = windowState->key_D;
            //Predict Input State
            predictClientState(&input, renderState, networkState);
            //Add Move to queue
            networkState->input_queue.enqueue(input);

            //Read packets from server and update render state
            Engine::udpReceive_client(networkState, (char*)&networkState->receive_p, sizeof(Receive_P));
            //Check if message length is greater than 0
            if(networkState->recv_msg_len > 0){
                switch(checkProtocol(networkState, networkState->recv_msg_len))
                {
                    case GAME_PACKET:
                    {
                        //Drop late Packets
                        if(networkState->receive_p.time >= networkState->cur_server_time){
                            networkState->cur_server_time = networkState->receive_p.time;

                            //Update RenderState
                            for(int i=0; i < MAX_CLIENTS; i++){
                                Engine::Client_MS_P* client_s = &networkState->receive_p.client_p[i];
                                Player* player = &renderState->players[i];

                                //Update players
                                if(networkState->receive_p.client_p[i].isActive){
                                    //Check if renderstate has this slot active
                                    if(renderState->slotlist_players[i]){
                                        //Update Player
                                        player->pos = client_s->pos;
                                    }else{
                                        //Add Player
                                        Player* newPlayer = addPlayer(renderState);
                                        newPlayer->program_index = 0;
                                        newPlayer->vao_index = 0;
                                        newPlayer->texture_index = 0;
                                        newPlayer->camera_index = 0;
                                        newPlayer->offset = glm::vec3(0.0f, 0.0f, 0.0f);
                                        newPlayer->pos = glm::vec3(client_s->pos.x, client_s->pos.y, client_s->pos.z);
                                        newPlayer->scale = glm::vec3(20.0f, 20.0f, 1.0f);
                                        newPlayer->rotate = glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f));
                                    }
                                }else{
                                    if(renderState->slotlist_players[i]){
                                        //Remove Player
                                        removePlayer(renderState, i);
                                    }
                                }
                            }

                            //Debug
                            // std::cout << "-------------Players-------------" << std::endl;
                            // for(int i=0; i < renderState->MAX_PLAYERS; i++){
                            //     std::cout << i << ": " << renderState->slotlist_players[i];
                            //     if(renderState->slotlist_players[i]){
                            //         std::cout << renderState->players[i].pos.x 
                            //         << "\t|\t"<< renderState->players[i].pos.y 
                            //         << "\t|\t"<< renderState->players[i].pos.z << std::endl;
                            //     }else{
                            //         std::cout << std::endl;
                            //     }
                            // }
                        }
                        break;
                    }
                }
            }

            //time-out if server hasnt sent update packet after x seconds & Disconnect Packets

            //std::cout << "CONNECTED" << std::endl;
            break;
        }
            
    }



    //Connect to server (Render Different Scenes menu, connection screen, loading screen, game)
        // Sample user input (mouse, keyboard, joystick)
    // Package up and send movement command using simulation time
    // Read any packets from the server from the network system
    // Use packets to determine visible objects and their state
        // Render Scene
        // Sample clock to find end time
        // End time minus start time is the simulation time for the next frame

}

void Engine::render(WindowState* windowState, RenderResources* renderResources, RenderState* renderState){

    updateViewport(windowState);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //render state needs to have a main_menu, settings, join_server, ect. (Maps) -> can be loaded from file.
    renderPlayers(renderResources, renderState);
    renderObjects(renderResources, renderState);
    renderTexts(renderResources, renderState, windowState->width, windowState->height);


    glfwSwapBuffers(windowState->window);
}

void Engine::destroyEngine(WindowState* windowState, RenderResources* renderResources, RenderState* renderState){

    destroyRenderState(renderState);
    //Do RenderResources Next

    //Go through renderer and do clean up
    //Render State Clean Up
    for(int i=0; i < renderResources->num_fontFiles; i++){
        deleteFontFile(&(renderResources->fontFiles[i]));
    }
    renderResources->num_fontFiles = 0;
    //Window Clean Up
    glfwDestroyWindow(windowState->window);
    glfwTerminate();
}

void Engine::predictClientState(Move* move, RenderState* renderState, NetworkState* networkState){

    //Predict next position of all players (This clients player will get input applied to it)
    for(int i=0; i < MAX_CLIENTS; i++){
        if(renderState->slotlist_players[i]){
            Player* player = &renderState->players[i];
            if(renderState->client_id == i){
                //Client Player -> Apply input
                bool w = move->key_W;
                bool a = move->key_A;
                bool s = move->key_S;
                bool d = move->key_D;
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
                        player->theta = 0.0f;
                        player->mov_force = player->mov_acc;
                        //std::cout << "Up" << std::endl;
                //Left
                }else if((!w && a && !s && !d)
                            || (w && a && s && !d)){
                        player->theta = 270.0f;
                        player->mov_force = player->mov_acc;
                        //std::cout << "Left" << std::endl;
                //Down
                }else if((!w && !a && s && !d)
                            || (!w && a && s && d)){
                        player->theta = 180.0f;
                        player->mov_force = player->mov_acc;
                        //std::cout << "Down" << std::endl;
                //Right
                }else if((!w && !a && !s && d)
                            || (w && !a && s && d)){
                        player->theta = 90.0f;
                        player->mov_force = player->mov_acc;
                        //std::cout << "Right" << std::endl;
                //Up-Right
                }else if((w && !a && !s && d)){
                        player->theta = 45.0f;
                        player->mov_force = player->mov_acc;
                        //std::cout << "Up-Right" << std::endl;
                //Up-Left
                }else if((w && a && !s && !d)){
                        player->theta = 315.0f;
                        player->mov_force = player->mov_acc;
                        //std::cout << "Up-Left" << std::endl;
                //Bottom-Left
                }else if((!w && a && s && !d)){
                        player->theta = 225.0f;
                        player->mov_force = player->mov_acc;
                        //std::cout << "Bottom-Left" << std::endl;
                //Bottom-Right
                }else if((!w && !a && s && d)){
                        player->theta = 135.0f;
                        player->mov_force = player->mov_acc;
                        //std::cout << "Bottom-Right" << std::endl;
                }
            }else{
                //Other players -> Reset attributes
                player->mov_force = 0.0f;
                player->theta = 0.0f;
            }

            //Step integration
            //Force
            resetNetForce(&player->netforce);
            addForce2D(&player->netforce, player->mov_force, player->theta);
            addForceVec(&player->netforce, -player->vel.x * player->mov_friction, -player->vel.y * player->mov_friction, -player->vel.z * player->mov_friction);

            //Integration to update RenderState
            player->acc = player->netforce / player->mass;
            player->vel = player->vel + player->acc;
            player->pos = player->pos + player->vel;

            //Save state to move
            move->players[i].isActive = true;
            move->players[i].vel = player->vel;
            move->players[i].pos = player->pos;

        }else{
            //Save state to move
            move->players[i].isActive = false;
        }
    }
}

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ CLIENT FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/





/***************************************** SERVER FUNCTIONS *******************************************/



/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SERVER FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/










/************************************ INIT ENGINE FUNCTIONS *****************************************/

int Engine::createWindow(WindowState* windowState){

	//Create OpenGL Mode
	windowState->mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Create OpenGL Window
	if(windowState->isFullscreen == true){
		windowState->window = glfwCreateWindow(windowState->mode->width, windowState->mode->height, "Project", glfwGetPrimaryMonitor(), NULL);
	}else{
		windowState->window = glfwCreateWindow(windowState->width, windowState->height, "Project", NULL, NULL);
	}

	if (windowState->window == NULL) {
		std::cout << "Failed to open GLFW window.\n";
		return 1;
	}

	//Set window to Current OpenGL Context
	glfwMakeContextCurrent(windowState->window);

	return 0;
}

void Engine::loadWindowState(WindowState* windowState){

    bool fullscreen = glfwGetWindowMonitor(windowState->window) != nullptr;

    if(windowState->isFullscreen && !fullscreen){
        glfwSetWindowMonitor(windowState->window, glfwGetPrimaryMonitor(), 0, 0, windowState->mode->width, windowState->mode->height, windowState->mode->refreshRate);
    }else if(!windowState->isFullscreen && fullscreen){
        glfwSetWindowMonitor( windowState->window, nullptr,  windowState->pos[0], windowState->pos[1], windowState->width, windowState->height, 0);
    }
}

void Engine::loadInputState(WindowState* windowState, GLFWkeyfun glfwkeyfun){

    //Set glfwSetWindowUserPoint
    glfwSetWindowUserPointer(windowState->window, windowState);

    glfwSetInputMode(windowState->window, GLFW_STICKY_KEYS, GL_TRUE);
	//glfwSetInputMode(windowState->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//glfwSetCursorPos(windowState->window, windowState->mode->width / 2, windowState->mode->height / 2);
    glfwSetKeyCallback(windowState->window, glfwkeyfun);

}

void Engine::loadOpenGLState(OpenGLState* openGLState){

    glClearColor(openGLState->clear_Color[0], openGLState->clear_Color[1],openGLState->clear_Color[2],openGLState->clear_Color[3]);
    if(openGLState->isCulling){
        glEnable(GL_CULL_FACE);
    }else{
        glDisable(GL_CULL_FACE);
    }
    if(openGLState->isWireframe){
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }else{
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }
    glFrontFace(GL_CCW);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ INIT ENGINE FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


















/**************************************** OPENGL FUNCTIONS *************************************************/

void Engine::updateViewport(WindowState* windowState){

    if (windowState->updateViewport){
        glfwGetFramebufferSize(windowState->window, &windowState->viewPortSize[0], &windowState->viewPortSize[1] );
        glViewport( 0, 0, windowState->viewPortSize[0], windowState->viewPortSize[1] );
        windowState->updateViewport = false;
    }
}

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ OPENGL FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/**************************************** NETWORK FUNCTIONS *************************************************/
int Engine::udpInit(NetworkState* networkState){

    //Init WINSOCK
    WSAData wsaData;
    WORD dllVersion = MAKEWORD(2, 1);
    networkState->result = WSAStartup(dllVersion, &wsaData);
    if(networkState->result != NO_ERROR){
        std::cout << "Error: failed to init WINSOCK" << std::endl;
        return 1;
    }

    //Socket
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
int Engine::udpConnect(NetworkState* networkState){
    //Server Address
    networkState->server_address.sin_family = AF_INET;
    InetPtonW(AF_INET, networkState->address, &networkState->server_address.sin_addr.S_un.S_addr);
    networkState->server_address.sin_port = htons(networkState->server_port);
    networkState->isConnected = false;

    return 0;
}
void Engine::udpSend_client(NetworkState* networkState, char* buffer, int size){
    networkState->result = sendto(networkState->server_socket, buffer, size, 0, (struct sockaddr*) &networkState->server_address, sizeof(networkState->server_address));
    if(networkState->result == SOCKET_ERROR){
        networkState->error = WSAGetLastError();
        std::cout << "Error " << networkState->error << ": failed to send message." << std::endl;
    }
}
void Engine::udpReceive_client(NetworkState* networkState, char* buffer, int size){

    networkState->server_address_len = sizeof(networkState->server_address);

    networkState->recv_msg_len = recvfrom(networkState->server_socket, buffer, size, 0, (struct sockaddr*) &networkState->server_address, &networkState->server_address_len);
    if(networkState->recv_msg_len == SOCKET_ERROR){
        networkState->error = WSAGetLastError();
        if(networkState->error = WSAEWOULDBLOCK){
            //Empty Socket -> continue without blocking
        }else{
            std::cout << "Error " << networkState->error << ": failed to receive message." << std::endl;
        }
    }
}
int Engine::udpDisconnect(NetworkState* networkState){

    networkState->result = closesocket(networkState->server_socket);
    if(networkState->result == SOCKET_ERROR){
        networkState->error = WSAGetLastError();
        std::cout << "Error " << networkState->error << ": failed to close socket." << std::endl;
        return 1;
    }
    networkState->isConnected = false;
    return 0;
}
int Engine::udpCleanup(NetworkState* networkState){
    WSACleanup();
    return 0;
}

void Engine::package_msg(char* msg, int size,NetworkState* networkState){
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
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ NETWORK FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/**************************************** PROTOCOL FUNCTIONS **********************************************/
char Engine::checkProtocol(NetworkState* networkState, int buffer_len){

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
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ PROTOCOL FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/







/**************************************** HELPER FUNCTIONS *************************************************/

glm::mat4 FPS_ViewMatrix( glm::vec3 pos, float pitch, float yaw )
{
    float cosPitch = cos(pitch);
    float sinPitch = sin(pitch);
    float cosYaw = cos(yaw);
    float sinYaw = sin(yaw);
 
    glm::vec3 xaxis = { cosYaw, 0.0f, -sinYaw };
    glm::vec3 yaxis = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
    glm::vec3 zaxis = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };
 
    glm::mat4 viewMatrix = {
        glm::vec4(       xaxis.x,            yaxis.x,            zaxis.x,      0.0f ),
        glm::vec4(       xaxis.y,            yaxis.y,            zaxis.y,      0.0f ),
        glm::vec4(       xaxis.z,            yaxis.z,            zaxis.z,      0.0f ),
        glm::vec4( -glm::dot( xaxis, pos ), -glm::dot( yaxis, pos ), -glm::dot( zaxis, pos ), 1.0f )
    };
     
    return viewMatrix;
}
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ HELPER FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/**************************************** BITWISE FUNCTIONS *************************************************/

void Engine::getButtonsBitset(WindowState* windowState, NetworkState* networkState){

    //W
    if(windowState->key_W){
        networkState->input_p.buttons |= 1UL << 3;
    }else{
        networkState->input_p.buttons &= ~(1UL << 3);
    }

    //A
    if(windowState->key_A){
        networkState->input_p.buttons |= 1UL << 2;
    }else{
        networkState->input_p.buttons &= ~(1UL << 2);
    }
    //S
    if(windowState->key_S){
        networkState->input_p.buttons |= 1UL << 1;
    }else{
        networkState->input_p.buttons &= ~(1UL << 1);
    }
    //D
    if(windowState->key_D){
        networkState->input_p.buttons |= 1UL;
    }else{
        networkState->input_p.buttons &= ~(1UL);
    }

}

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ BITWISE FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

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