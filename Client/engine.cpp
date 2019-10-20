#include "engine.h"

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
        *   - Engine Setup                                                                                  -Done       (Clean up destruction of engine)
        *   - Window Setup                                                                                  -Done
        *   - OpenGL Setup                                                                                  -Done
        *   - Renderer Setup                                                                                -Done
        *   - Model Loader                                                                                  -Done       (File import from blender not supported)
        *   - Program Loader                                                                                -Done       (Clean up auto uniform detection)
        *   - Texture Loader                                                                                -Done
        *   - Process Inputs                                                                                -Done
        *   - Resource, State, input Files
        *   - Render Cameras(Projection and view matrix uniforms)                   (1 day)                 -Done       (Scene defines when different types of cameras are used and when certain things are rendered)
        *   - Add and Remove Objects, Cameras, etc.                                 (1 day)                 -Done       (Need to remove Texts)
        *   - Distance Field Text                                                   (2 week)                            (Fixed max string length) (Dynamic Array Size for Text) (Delete Dynamic Text https://stackoverflow.com/questions/936687/how-do-i-declare-a-2d-array-in-c-using-new)           
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
        *   - Setup Client-Side Prediction                                          (1 week)
        *   - Setup Server Reconciliation                                           (1 week)
        *   - Setup Entity Interpolation                                            (1 day)
        *   - Setup Lag Compensation                                                (1 week)
        * Water/Fluid Dynamics
        *   - Simulation On Client-Side                                             (tbd)
        *   - Simulation Over Client-Server                                         (tbd)
        *   - Simulation with Prediction and Lag Compensation                       (tbd)
    
        


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

    while(openGLState->isRunning && !glfwWindowShouldClose(windowState->window)){
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

    // std::cout << "W: " << windowState->key_W << std::endl;
    // std::cout << "A: " << windowState->key_A << std::endl;
    // std::cout << "S: " << windowState->key_S << std::endl;
    // std::cout << "D: " << windowState->key_D << std::endl;
    // std::cout << "Escape: " << windowState->key_Escape << std::endl;

}

void Engine::update(){

}

void Engine::render(WindowState* windowState, RenderResources* renderResources, RenderState* renderState){

    updateViewport(windowState);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //render state needs to have a main_menu, settings, join_server, ect. (Maps) -> can be loaded from file.
    renderCameras(renderResources, renderState);
    renderObjects(renderResources, renderState);
    renderTexts(renderResources, renderState, windowState->width, windowState->height);


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
    if(openGLState->isWireframe){
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }else{
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }
    glFrontFace(GL_CCW);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

int Engine::loadRenderer(WindowState* windowState, RenderResources* renderResources, RenderState* renderState){

    //Load Textures - Create Texture Loader
    // if(loadTexture("Textures\\Test\\water.jpg", renderResources)){
    //     std::cout << "Failed to load Texture." << std::endl;
    //     return 1;
    // }
    if(loadTexture("Textures\\Fonts\\Candara.png", renderResources)){
        std::cout << "Failed to load Texture." << std::endl;
        return 1;
    }

    //Load VAOs - VAO loader from blender
    //createStaticObject_VAO(loadModel("", renderResources), renderResources);
    createDynamic2DText_VAO_VBO(renderResources);
    
    //Load Shaders          https://www.glfw.org/docs/latest/quick.html
    // if(loadShader("../Shaders/basic.vert","../Shaders/basic.frag", renderResources)){
    //     std::cout << "Failed to load Shader Program." << std::endl;
    //     return 1;
    // }
    if(loadShader("../Shaders/text2d.vert","../Shaders/text2d.frag", renderResources)){
        std::cout << "Failed to load Shader Program." << std::endl;
        return 1;
    }

    //Load Font Files
    if(loadFontFile("Textures\\Fonts\\Candara.fnt", renderResources)){
        std::cout << "Failed to load Font." << std::endl;
        return 1;
    }

    //Create Cameras (UI Camera, Other Cameras)
    addCamera(renderState, windowState->width, windowState->height);

    //Create Objects
    //addObject(renderState);

    //Create Fonts      (read from file)  
    addText(renderState);

    std::cout << "------------Render Resources---------------" << std::endl;
    std::cout << "Num_Programs  \t=\t" << renderResources->num_programs << std::endl;
    std::cout << "Num_VAOs      \t=\t" << renderResources->num_vaos << std::endl;
    std::cout << "Num_VBOs      \t=\t" << renderResources->num_vbos << std::endl;
    std::cout << "Num_Textures  \t=\t" << renderResources->num_textures << std::endl;
    std::cout << "Num_FontFiles \t=\t" << renderResources->num_fontFiles << std::endl;
    std::cout << "--------------Render State-----------------" << std::endl;
    std::cout << "Num_Cameras   \t=\t" << renderState->num_cameras << std::endl;
    std::cout << "Num_Objects   \t=\t" << renderState->num_objects << std::endl;
    std::cout << "Num_Texts     \t=\t" << renderState->num_texts << std::endl;
    std::cout << "--------------------------------------------" << std::endl;


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