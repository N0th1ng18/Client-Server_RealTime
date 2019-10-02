#include "engine.h"

/*
    Nicholas Frey

    To do:
        * Multiplayer Only - first thing is connect to a server.
        * Menu Loader          
        *   - Process Inputs
        *   - Text                                                                  (2 week)                     
        *   - Buttons                                                               (1 day)
        *   - First Page - Play Online, Settings, Quit                              (less than 1 day)
        *   - Play Online - Host Server, Join By Ip, Back                           (less than 1 day)     
        *   - Settings - Video, Audio, Back                                         (less than 1 day)
        *   - Quit - Exits                                                          (less than 1 day)
        *   - Menu Page Loader                                                      (1 day)
        * Client-Server Setup and test
        *   - Setup Basic Connection                                                (1 day)
        *   - Setup Ping Delay for testing                                          (1 day)
        *   - Setup Dump Client                                                     (1 day)
        *   - Setup Client-Side Prediciton                                          (1 week)
        *   - Setup Server Reconciliation                                           (1 week)
        *   - Setup Entity Interpolation                                            (1 day)
        *   - Setup Lag Compensation                                                (1 week)
        * Water/Fluid Dynamics
        *   - Simulation On Client-Side                                             (tbd)
        *   - Simulation Over Client-Server                                         (tbd)
        *   - Simulation with Prediction and Lag Compensation                       (tbd)

    Server:                                                                         
        * Max Players Per Game is 4.
        


*/

/***************************************** MAIN FUNCTIONS *******************************************/

int Engine::initEngine(WindowState* windowState, OpenGLState* openGLState, RenderResources* renderResources, RenderState* renderState){

    //Initialize and load all states
	if (!glfwInit()){std::cout << "Failed to initialize GLFW\n"; return 1;}
    createWindow(windowState);
    loadWindowState(windowState);
    loadInputState(windowState, Engine::input);
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {std::cout << "Failed to initialize GLEW\n"; return 1;}
    loadOpenGLState(openGLState);
    loadRenderer(windowState, renderResources, renderState);

    return 0;
}

void Engine::loop(WindowState* windowState, OpenGLState* openGLState, RenderResources* renderResources, RenderState* renderState){
    //Connect to server (Render Different Scenes menu, connection screen, loading screen, game)

    // Sample clock to find start time
    // Sample user input (mouse, keyboard, joystick)
    // Package up and send movement command using simulation time
    // Read any packets from the server from the network system
    // Use packets to determine visible objects and their state
    // Render Scene
    // Sample clock to find end time
    // End time minus start time is the simulation time for the next frame

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

    while(openGLState->isRunning){
        QueryPerformanceCounter(&newCycles);
        frameTime.QuadPart = ((newCycles.QuadPart - oldCycles.QuadPart) * 1000000LL) / frequency.QuadPart;
        oldCycles.QuadPart = newCycles.QuadPart;

        //Spiral of Death
        if(frameTime.QuadPart > 2500000LL){ //2.5 seconds
            frameTime.QuadPart = 2500000LL;
        }

        accumulator += (double)(frameTime.QuadPart);

        while(accumulator >= dt){
            Engine::update();
            updateCounter++;

            time += dt / 1000000.0;
            accumulator -= dt;
        }
    
        alpha = accumulator / dt;
        Engine::render(windowState, renderResources, renderState);
        renderCounter++;

        glfwPollEvents();

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
    void * data = glfwGetWindowUserPointer(window);
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

    std::cout << "W: " << windowState->key_W << std::endl;
    std::cout << "A: " << windowState->key_A << std::endl;
    std::cout << "S: " << windowState->key_S << std::endl;
    std::cout << "D: " << windowState->key_D << std::endl;
    std::cout << "Escape: " << windowState->key_Escape << std::endl;

}

void Engine::update(){

}

void Engine::render(WindowState* windowState, RenderResources* renderResources, RenderState* renderState){

    updateViewport(windowState);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //render state needs to have a main_menu, settings, join_server, ect. (Maps) -> can be loaded from file.
    renderObjects(renderResources, renderState);
    renderTexts(renderResources, renderState);


    glfwSwapBuffers(windowState->window);
}

void Engine::destroy_Engine(WindowState* windowState, RenderResources* renderResources){

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

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ MAIN FUNCTIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

















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
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    if(openGLState->isCulling){
        glEnable(GL_CULL_FACE);
    }else{
        glDisable(GL_CULL_FACE);
    }
    glEnable(GL_BLEND);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

int Engine::loadRenderer(WindowState* windowState, RenderResources* renderResources, RenderState* renderState){

    //Load Textures - Create Texture Loader
    if(loadTexture("Textures\\Test\\water.jpg", renderResources)){
        std::cout << "Failed to load Texture." << std::endl;
        return 1;
    }
    // if(loadTexture("Build\\Textures\\Fonts\\Calibri_DF.png", renderResources)){
    //     std::cout << "Failed to load Texture." << std::endl;
    //     return 1;
    // }

    //Load VAOs - VAO loader from blender
    createStaticObject_VAO(loadModel("", renderResources), renderResources);
    createDynamic2DText_VAO_VBO(renderResources);
    
    //Load Shaders          https://www.glfw.org/docs/latest/quick.html
    if(loadShader("../Shaders/basic.vert","../Shaders/basic.frag", renderResources)){
        std::cout << "Failed to load Shader Program." << std::endl;
        return 1;
    }
    //LOAD SHADER FOR TEXT

    //Load Font Files
    if(loadFontFile("Textures\\Fonts\\Calibri_DF.fnt", renderResources)){
        std::cout << "Failed to load Font." << std::endl;
        return 1;
    }

    //Create Objects
    createObject(renderState, "", windowState->width, windowState->height);

    //Create Fonts      (read from file)  
    createText(renderState, "");

    return 0;
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