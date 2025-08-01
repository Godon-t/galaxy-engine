#include <src/sections/rendering/Renderer.hpp>

#include <iostream>

#include <backend/imgui_impl_glfw.h>
#include <imgui.h>
#include <backend/imgui_impl_opengl3.h>

#include <src/sections/rendering/VisualInstance.hpp>
#include <src/sections/rendering/OpenglHelper.hpp>


void Renderer::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // Camera::getInstance().view_width = float(width);
    // Camera::getInstance().view_height = float(height);

    // Camera::editor = !Camera::editor; 
    // Camera::getInstance().view_width = float(width);
    // Camera::getInstance().view_height = float(height);

    // Camera::editor = !Camera::editor;
    std::cout<<width<<" "<<height<<std::endl;
    glViewport(0, 0, width, height);
}

const size_t maxSize = 512;
Renderer::Renderer()
{
    visuInstances.resize(maxSize);
    
    for(size_t i=0; i<maxSize; i++){
        freeIds.push(i);
    }

    GLenum error = glGetError();
    
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        throw std::runtime_error("Failed glfw init");
    }

    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow( 1024, 768, "engine - GLFW", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        throw std::runtime_error("Failed window creation");
    }
    glfwMakeContextCurrent(window);
    
    
    checkOpenGLErrors("error before glewInit");
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        throw std::runtime_error("Failed glfw init");
    }
    checkOpenGLErrors("known error after glewInit");
    
    glfwSetWindowSizeCallback(window, Renderer::framebuffer_size_callback);
    

    glClearColor(1.f, 0.f, 0.2f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    glEnable(GL_CULL_FACE);

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();


    frameDuration = std::chrono::milliseconds(1000 / 60); // 60 fps


    mainProgram = std::move(Program("shaders/vertex.glsl", "shaders/fragment.glsl"));
    
    checkOpenGLErrors("Renderer constructor");
}

Renderer::~Renderer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

Renderer& Renderer::getInstance(){
    static Renderer renderer;
    return renderer;
}

void Renderer::renderFrame()
{
    // glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // float currentFrame = glfwGetTime();
    auto frameStart = std::chrono::high_resolution_clock::now();
    // deltaTime = currentFrame - lastFrame;
    // lastFrame = currentFrame;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




    mainProgram.use();

    for(int instanceIdx=0; instanceIdx<instanceCount; instanceIdx++){
        visuInstances[instanceIdx].draw();
    }


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers
    glfwSwapBuffers(window);
    // glfwPollEvents();

    auto frameEnd = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);
    if (elapsed < frameDuration)
        std::this_thread::sleep_for(frameDuration - elapsed);
}

renderID Renderer::instanciateMesh(std::vector<Vertex> &vertices, std::vector<short unsigned int> &indices)
{
    if(freeIds.size() == 0) return -1;

    VisualInstance meshInstance;
    meshInstance.init(vertices, indices);
    
    renderID meshID = freeIds.top();
    size_t listIdx = instanceCount;
    visuInstances[listIdx] = std::move(meshInstance);
    visuIdxToInstanceId[listIdx] = meshID;
    instanceIdToVisuIdx[meshID] = listIdx;
    
    freeIds.pop();
    instanceCount ++;
    return meshID;
}

void Renderer::clearMesh(renderID meshID)
{
    size_t idxToDelete = instanceIdToVisuIdx[meshID];

    instanceCount--;

    renderID movedMeshID = visuIdxToInstanceId[instanceCount];
    instanceIdToVisuIdx[movedMeshID] = idxToDelete;
    visuIdxToInstanceId[idxToDelete] = movedMeshID;

    visuInstances[idxToDelete] = std::move(visuInstances[instanceCount]);

    freeIds.emplace(meshID);
}
