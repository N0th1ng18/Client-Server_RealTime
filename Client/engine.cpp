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
        *   - Setup Basic Connection                                                                        -Done                   
        *   - Setup Dumb Client                                                                             -Done       
        *   - Setup Client-Side Prediction                                                                  -Done
        *   - Setup Server Reconciliation                                                                   -Done
        *   - Setup Entity Interpolation                                            (1 day)
        *   - Setup Lag Compensation                                                

*/

//Private Functions
void resetNetForce(glm::vec3* netforce);
void addForce2D(glm::vec3* netforce, float force, float theta);
void addForce3D(glm::vec3* netforce, float force, float theta, float phi);
void addForceVec(glm::vec3* netforce, float x, float y, float z);
float degreesToRadians(float degrees);

/***************************************** CLIENT FUNCTIONS *******************************************/

int Engine::initEngine(WindowState* windowState, OpenGLState* openGLState, RenderResources* renderResources, RenderState* renderState, NetworkState* networkState){

    //Initialize and load all states
	if (!glfwInit()){std::cout << "Failed to initialize GLFW\n"; return 1;}
    createWindow(windowState);
    loadWindowState(windowState);
    loadInputState(windowState, Engine::input);
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {std::cout << "Failed to initialize GLEW\n"; return 1;}
    loadOpenGLState(openGLState);
    createQueue(&networkState->input_queue, networkState->input_queue_size);

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
            Engine::update(alpha, time, windowState, renderState, networkState);
            updateCounter++;

            time += dt / 1000000.0;
            accumulator -= dt;
        }
        //Render
        alpha = accumulator / dt;
        Engine::render(alpha, time, windowState, renderResources, renderState);
        renderCounter++;

        //FPS
        QueryPerformanceCounter(&fpsEndTime);
        if((((fpsEndTime.QuadPart - fpsStartTime.QuadPart) * 1000000LL) / frequency.QuadPart) >= 1000000LL){
            QueryPerformanceCounter(&fpsStartTime);
            std::cout << "FPS:\t" << renderCounter << "\tUPS:\t" << updateCounter << "\tTime:\t" << time << "\tInput_Queue:\t" << networkState->input_queue.count << "\n";
            updateCounter = 0;
            renderCounter = 0;
        }

        //Exit
        if(windowState->key_Escape){
            openGLState->isRunning = false;
        }
        if(windowState->key_F1){
            renderState->player_debug = true;
        }else{
            renderState->player_debug = false;   
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
                if(!windowState->key_Escape){
                    windowState->key_Escape = true;
                }else{
                    windowState->key_Escape = false;
                }
            }
            break;
        }
        case GLFW_KEY_F1:
        {
            if(action == GLFW_PRESS){
                if(!windowState->key_F1){
                    windowState->key_F1 = true;
                }else{
                    windowState->key_F1 = false;
                }
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

void Engine::update(double alpha, double time, WindowState* windowState, RenderState* renderState, NetworkState* networkState){

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
                        std::cout << "Client: Connection Accepted: " << networkState->receive_p.client_id << std::endl;
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
            //Debug
            // std::cout << "Sending Input Packet: " 
            // << "Input: " << windowState->key_W
            //              << " " << windowState->key_A
            //              << " " << windowState->key_S
            //              << " " << windowState->key_D
            // << std::endl << std::endl;


            //Send Input Packet to server
            getButtonsBitset(windowState, networkState);
            networkState->input_p.time = time;
            Engine::udpSend_client(networkState, (char*)&networkState->input_p, sizeof(Input_P));

            //Debug
            // std::cout << "Add Packet To Input Queue: " << std::endl;

            //Create Move from input packet
            Move* temp1 = NULL;
            Move* temp2 = NULL;
            Move temp;
            temp.time = networkState->input_p.time;
            temp.key_W = windowState->key_W;
            temp.key_A = windowState->key_A;
            temp.key_S = windowState->key_S;
            temp.key_D = windowState->key_D;
            for(int i=0; i < MAX_CLIENTS; i++){
                temp.players[i].isActive = false;
                temp.players[i].pos = glm::vec3(0.0f, 0.0f, 0.0f);
                temp.players[i].vel = glm::vec3(0.0f, 0.0f, 0.0f);
            }

            //Add Move to queue
            enqueue(&networkState->input_queue, &temp);

            //Debug
            // std::cout << "Receive Packet From Server: " << std::endl;
            bool skipPredictFinalStep = false;

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

                            //Add and Remove Players to and from renderState based on packet
                            for(int i=0; i < MAX_CLIENTS; i++){
                                
                                //Check if active
                                if(networkState->receive_p.client_p[i].isActive){
                                    //Active Player
                                    if(!renderState->slotlist_players[i]){
                                        //Add Player and update info
                                        Player* newPlayer = addPlayer(renderState);
                                        newPlayer->program_index = 0;
                                        newPlayer->vao_index = 0;
                                        newPlayer->texture_index = 0;
                                        newPlayer->camera_index = 0;
                                        newPlayer->offset = glm::vec3(0.0f, 0.0f, 0.0f);
                                        newPlayer->pos = glm::vec3(networkState->receive_p.client_p[i].pos.x, networkState->receive_p.client_p[i].pos.y, networkState->receive_p.client_p[i].pos.z);
                                        newPlayer->vel = glm::vec3(networkState->receive_p.client_p[i].vel.x, networkState->receive_p.client_p[i].vel.y, networkState->receive_p.client_p[i].vel.z);
                                        newPlayer->scale = glm::vec3(50.0f, 50.0f, 1.0f);
                                        newPlayer->rotate = glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f));


                                    }

                                    //Debug
                                    if(renderState->player_debug){
                                        if(!renderState->players[i].player_object){
                                            //Debug Player Object
                                            Object* object = addObject(renderState);
                                            object->program_index = 0;
                                            object->vao_index = 0;
                                            object->texture_index = 1;
                                            object->camera_index = 0;
                                            object->offset = glm::vec3(0.0f, 0.0f, 0.0f);
                                            object->pos = glm::vec3(networkState->receive_p.client_p[i].pos.x, networkState->receive_p.client_p[i].pos.y, networkState->receive_p.client_p[i].pos.z);
                                            object->vel = glm::vec3(networkState->receive_p.client_p[i].vel.x, networkState->receive_p.client_p[i].vel.y, networkState->receive_p.client_p[i].vel.z);
                                            object->scale = glm::vec3(50.0f, 50.0f, 1.0f);
                                            object->rotate = glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f));
                                            renderState->players[i].player_object = true;
                                        }
                                    }else{
                                        if(renderState->players[i].player_object){
                                            removeObject(renderState, i);
                                            renderState->players[i].player_object = false;
                                        }
                                    }

                                }else{
                                    //Not Active Player
                                    if(renderState->slotlist_players[i]){
                                        //Remove Player
                                        removePlayer(renderState, i);
                                    }

                                    //Debug
                                    if(renderState->player_debug && renderState->players[i].player_object == true){
                                            removeObject(renderState, i);
                                            renderState->players[i].player_object = false;
                                    }
                                }
                            }


                            //Debug
                            // std::cout << "Dequeue Acked inputs with input state time: " << networkState->receive_p.last_input_time << std::endl;

                            //Dequeue Acked inputs
                            if(networkState->input_queue.count > 1){

                                //get Starting input Move
                                peek(&networkState->input_queue, &temp1);

                                //Traverse input Queue dequeueing all packets before server's last_input_time
                                // std::cout << "DEBUG-> Count:" 
                                // << networkState->input_queue.count << " | front:"
                                // << networkState->input_queue.front << " | back:"
                                // << networkState->input_queue.back << " | "
                                // << " | " << networkState->receive_p.last_input_time << " > " << temp1->time 
                                // << std::endl;

                                while(networkState->receive_p.last_input_time > temp1->time){
                                    //remove input move as it has been acked
                                    dequeue(&networkState->input_queue);
                                    //get Starting input Move
                                    peek(&networkState->input_queue, &temp1);
                                }
                            }

                            //Debug
                            // std::cout << "Check if server state is different then Move: " << std::endl;

                            //Get Starting input move
                            peek(&networkState->input_queue, &temp1);
                            bool isEqual = true;
                            
                            //Check if Server Master State is different then temp1.state
                            for(int i=0; i < MAX_CLIENTS; i++){

                                //Check if Active:  Keep in mind that Server could have added or removed players
                                if(networkState->receive_p.client_p[i].isActive != temp1->players[i].isActive){
                                    isEqual = false;
                                    break;
                                }
                                //if Player is active check state of player
                                if(networkState->receive_p.client_p[i].isActive){
                                    Client_MS_P* player_server = &networkState->receive_p.client_p[i];

                                    if(player_server->pos != temp1->players[i].pos){
                                        isEqual = false;
                                        break;
                                    }
                                }
                            }
                            if(isEqual == true){
                                break; 
                            }


                            //Debug
                            // std::cout << "Set Starting input move to server's masterstate: " << std::endl;

                            //Get Starting input move
                            peek(&networkState->input_queue, &temp1);

                            //Set Starting move to Server master state
                            for(int i=0; i < MAX_CLIENTS; i++){
                                temp1->players[i].isActive = networkState->receive_p.client_p[i].isActive;
                                temp1->players[i].pos = networkState->receive_p.client_p[i].pos;
                                temp1->players[i].vel = networkState->receive_p.client_p[i].vel;
                            }

                            //Debug
                            // toString(&networkState->input_queue);
                            // std::cout << std::endl;

                            //Debug
                            // std::cout << "Fix Prediction Differences from Client and Server: " << std::endl;

                            if(networkState->input_queue.count == 1){
                                //Needs to goto setRenderState
                                skipPredictFinalStep = true;
                                break;
                            }else if(networkState->input_queue.count == 2){
                                //Nothing to Fix
                                break;
                            }else{

                                //Fix by prediction
                                for(int i=0; i < networkState->input_queue.count - 2; i++){
                                    peekIndex(&networkState->input_queue, &temp1, i);
                                    peekIndex(&networkState->input_queue, &temp2, i+1);

                                    predictToNextMove(&temp1, &temp2, renderState, networkState);
                                }
                            }
                        }
                        break;
                    }
                }

            }

            //Debug
            // std::cout << "Predict Final Step: " << std::endl;

            //Debug
            // toString(&networkState->input_queue);
            // std::cout << std::endl;

            //Predict Final Step
            if(!skipPredictFinalStep){

                if(networkState->input_queue.count == 1){
                    //use renderstate as A
                    peek(&networkState->input_queue, &temp1);
                    predictToNextMove_Render(&temp1, renderState, networkState);
                }else{
                    peekIndex(&networkState->input_queue, &temp1, networkState->input_queue.count - 2);
                    peekIndex(&networkState->input_queue, &temp2, networkState->input_queue.count - 1);
                    predictToNextMove(&temp1, &temp2, renderState, networkState);
                }
            }

            //Debug
            // toString(&networkState->input_queue);
            // std::cout << std::endl;


            //Debug
            // std::cout << "Set Render State: " << std::endl;

            //Move at index (count - 1) sets Render State
            peekIndex(&networkState->input_queue, &temp1, networkState->input_queue.count - 1);
            for(int i=0; i < MAX_CLIENTS; i++){
                if(renderState->slotlist_players[i]){
                    //Update Render State

                    if(renderState->client_id == i){
                        //Client's Player
                        glm::vec3 difference =  renderState->players[i].pos - renderState->players[i].prev_pos;
                        renderState->players[i].prev_pos = renderState->players[i].prev_pos + difference * (float)alpha;

                        renderState->players[i].pos = temp1->players[i].pos;
                        renderState->players[i].vel = temp1->players[i].vel;
                    }else{
                        //Other Players
                        renderState->players[i].prev_pos = renderState->players[i].pos;

                        renderState->players[i].pos = networkState->receive_p.client_p[i].pos;
                        renderState->players[i].vel = networkState->receive_p.client_p[i].vel;
                    }
                    
                    if(renderState->player_debug && renderState->players[i].player_object == true){
                        //Debug Player
                        renderState->objects[i].pos = networkState->receive_p.client_p[i].pos;
                    }

                }
            }


            //Debug
            // std::cout << "-------------RenderState-------------" << std::endl;
            // for(int i=0; i < renderState->MAX_PLAYERS; i++){
            //     std::cout << i << ": " << renderState->slotlist_players[i] << " | ";
            //     if(renderState->slotlist_players[i]){
            //         std::cout << "ID: " << renderState->client_id
            //         << "\t|\t" << renderState->players[i].pos.x 
            //         << "\t|\t"<< renderState->players[i].pos.y 
            //         << "\t|\t"<< renderState->players[i].pos.z << std::endl;
            //     }
            // }


            //time-out if server hasnt sent update packet after x seconds & Disconnect Packets

            //std::cout << "CONNECTED" << std::endl;
            break;
        }
            
    }

}

void Engine::render(double alpha, double  time, WindowState* windowState, RenderResources* renderResources, RenderState* renderState){

    updateViewport(windowState);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //render state needs to have a main_menu, settings, join_server, ect. (Maps) -> can be loaded from file.
    renderPlayers(alpha, time, renderResources, renderState);
    renderObjects(renderResources, renderState);
    renderTexts(renderResources, renderState, windowState->width, windowState->height);

    glfwSwapBuffers(windowState->window);
}

void Engine::destroyEngine(WindowState* windowState, RenderResources* renderResources, RenderState* renderState, NetworkState* networkState){

    destroyRenderState(renderState);
    //Do RenderResources Next

    //Go through renderer and do clean up
    //Render State Clean Up
    for(int i=0; i < renderResources->num_fontFiles; i++){
        deleteFontFile(&(renderResources->fontFiles[i]));
    }
    renderResources->num_fontFiles = 0;
    //NetworkState
    deleteQueue(&networkState->input_queue);
    udpDisconnect(networkState);
    udpCleanup(networkState);
    //Window Clean Up
    glfwDestroyWindow(windowState->window);
    glfwTerminate();
}

void Engine::predictToNextMove(Move** temp1, Move** temp2, RenderState* renderState, NetworkState* networkState){
    for(int i=0; i < MAX_CLIENTS; i++){
        if(renderState->slotlist_players[i]){
            Player* render_temp = &renderState->players[i]; //Only used for constants
            //temp1's state + temp2's input = temp2's state
            if(renderState->client_id == i){
                //Only predict for Client Player -> Apply input
                bool w = (*temp2)->key_W;
                bool a = (*temp2)->key_A;
                bool s = (*temp2)->key_S;
                bool d = (*temp2)->key_D;
                //No movement
                if((!w && !a && !s && !d)               
                            || (w && !a && s && !d)
                            || (!w && a && !s && d)
                            || (w && a && s && d)){
                        render_temp->mov_force = 0.0f;
                        //std::cout << "No Movement" << std::endl;
                //Up
                }else if((w && !a && !s && !d)
                            || (w && a && !s && d)){
                        render_temp->theta = 0.0f;
                        render_temp->mov_force = render_temp->mov_acc;
                        //std::cout << "Up" << std::endl;
                //Left
                }else if((!w && a && !s && !d)
                            || (w && a && s && !d)){
                        render_temp->theta = 270.0f;
                        render_temp->mov_force = render_temp->mov_acc;
                        //std::cout << "Left" << std::endl;
                //Down
                }else if((!w && !a && s && !d)
                            || (!w && a && s && d)){
                        render_temp->theta = 180.0f;
                        render_temp->mov_force = render_temp->mov_acc;
                        //std::cout << "Down" << std::endl;
                //Right
                }else if((!w && !a && !s && d)
                            || (w && !a && s && d)){
                        render_temp->theta = 90.0f;
                        render_temp->mov_force = render_temp->mov_acc;
                        //std::cout << "Right" << std::endl;
                //Up-Right
                }else if((w && !a && !s && d)){
                        render_temp->theta = 45.0f;
                        render_temp->mov_force = render_temp->mov_acc;
                        //std::cout << "Up-Right" << std::endl;
                //Up-Left
                }else if((w && a && !s && !d)){
                        render_temp->theta = 315.0f;
                        render_temp->mov_force = render_temp->mov_acc;
                        //std::cout << "Up-Left" << std::endl;
                //Bottom-Left
                }else if((!w && a && s && !d)){
                        render_temp->theta = 225.0f;
                        render_temp->mov_force = render_temp->mov_acc;
                        //std::cout << "Bottom-Left" << std::endl;
                //Bottom-Right
                }else if((!w && !a && s && d)){
                        render_temp->theta = 135.0f;
                        render_temp->mov_force = render_temp->mov_acc;
                        //std::cout << "Bottom-Right" << std::endl;
                }

                //Step integration
                //Force
                resetNetForce(&render_temp->netforce);
                addForce2D(&render_temp->netforce, render_temp->mov_force, render_temp->theta);
                addForceVec(&render_temp->netforce, -(*temp1)->players[i].vel.x * render_temp->mov_friction, -(*temp1)->players[i].vel.y * render_temp->mov_friction, -(*temp1)->players[i].vel.z * render_temp->mov_friction);

                //Integration to update RenderState
                render_temp->acc = render_temp->netforce / render_temp->mass;
                (*temp2)->players[i].vel = (*temp1)->players[i].vel + render_temp->acc;
                (*temp2)->players[i].pos = (*temp1)->players[i].pos + (*temp2)->players[i].vel;

            }
            (*temp2)->players[i].isActive = true;
        }else{
            //Non-Active slot
            (*temp2)->players[i].isActive = false;
        }
    }
}

void Engine::predictToNextMove_Render(Move** temp1, RenderState* renderState, NetworkState* networkState){
    for(int i=0; i < MAX_CLIENTS; i++){
        if(renderState->slotlist_players[i]){
            Player* render_temp = &renderState->players[i]; 
            //RenderState state + temp1's input = temp1's state
            if(renderState->client_id == i){
                //Only predict for client Player -> Apply input
                bool w = (*temp1)->key_W;
                bool a = (*temp1)->key_A;
                bool s = (*temp1)->key_S;
                bool d = (*temp1)->key_D;
                //No movement
                if((!w && !a && !s && !d)               
                            || (w && !a && s && !d)
                            || (!w && a && !s && d)
                            || (w && a && s && d)){
                        render_temp->mov_force = 0.0f;
                        //std::cout << "No Movement" << std::endl;
                //Up
                }else if((w && !a && !s && !d)
                            || (w && a && !s && d)){
                        render_temp->theta = 0.0f;
                        render_temp->mov_force = render_temp->mov_acc;
                        //std::cout << "Up" << std::endl;
                //Left
                }else if((!w && a && !s && !d)
                            || (w && a && s && !d)){
                        render_temp->theta = 270.0f;
                        render_temp->mov_force = render_temp->mov_acc;
                        //std::cout << "Left" << std::endl;
                //Down
                }else if((!w && !a && s && !d)
                            || (!w && a && s && d)){
                        render_temp->theta = 180.0f;
                        render_temp->mov_force = render_temp->mov_acc;
                        //std::cout << "Down" << std::endl;
                //Right
                }else if((!w && !a && !s && d)
                            || (w && !a && s && d)){
                        render_temp->theta = 90.0f;
                        render_temp->mov_force = render_temp->mov_acc;
                        //std::cout << "Right" << std::endl;
                //Up-Right
                }else if((w && !a && !s && d)){
                        render_temp->theta = 45.0f;
                        render_temp->mov_force = render_temp->mov_acc;
                        //std::cout << "Up-Right" << std::endl;
                //Up-Left
                }else if((w && a && !s && !d)){
                        render_temp->theta = 315.0f;
                        render_temp->mov_force = render_temp->mov_acc;
                        //std::cout << "Up-Left" << std::endl;
                //Bottom-Left
                }else if((!w && a && s && !d)){
                        render_temp->theta = 225.0f;
                        render_temp->mov_force = render_temp->mov_acc;
                        //std::cout << "Bottom-Left" << std::endl;
                //Bottom-Right
                }else if((!w && !a && s && d)){
                        render_temp->theta = 135.0f;
                        render_temp->mov_force = render_temp->mov_acc;
                        //std::cout << "Bottom-Right" << std::endl;
                }

                //Step integration
                //Force
                resetNetForce(&render_temp->netforce);
                addForce2D(&render_temp->netforce, render_temp->mov_force, render_temp->theta);
                addForceVec(&render_temp->netforce, -render_temp->vel.x * render_temp->mov_friction, -render_temp->vel.y * render_temp->mov_friction, -render_temp->vel.z * render_temp->mov_friction);

                //Integration to update RenderState
                render_temp->acc = render_temp->netforce / render_temp->mass;
                (*temp1)->players[i].vel = render_temp->vel + render_temp->acc;
                (*temp1)->players[i].pos = render_temp->pos + (*temp1)->players[i].vel;

            }
            (*temp1)->players[i].isActive = true;
        }else{
            //Non-Active slot
            (*temp1)->players[i].isActive = false;
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