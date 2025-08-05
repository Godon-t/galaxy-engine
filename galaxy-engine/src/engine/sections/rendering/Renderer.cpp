#include "pch.hpp"
#include "Renderer.hpp"

#include <backend/imgui_impl_glfw.h>
#include <imgui.h>
#include <backend/imgui_impl_opengl3.h>

#include "VisualInstance.hpp"
#include "OpenglHelper.hpp"
#include "engine/Helper.hpp"

#include "Core.hpp"



void Renderer::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // Camera::getInstance().view_width = float(width);
    // Camera::getInstance().view_height = float(height);

    // Camera::editor = !Camera::editor; 
    // Camera::getInstance().view_width = float(width);
    // Camera::getInstance().view_height = float(height);

    // Camera::editor = !Camera::editor;
    // std::cout<<width<<" "<<height<<std::endl;
    glViewport(0, 0, width, height);
}

const size_t maxSize = 512;
Renderer::Renderer()
{
    m_visuInstances.resize(maxSize);
    
    for(size_t i=0; i<maxSize; i++){
        m_freeIds.push(i);
    }

    GLenum error = glGetError();
    
    bool initialized = glfwInit();
    GLX_CORE_ASSERT(initialized, "Glfw init");

    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow( 1024, 768, "engine - GLFW", NULL, NULL);
    GLX_CORE_ASSERT(window != NULL, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version");

    glfwMakeContextCurrent(window);
    
    
    checkOpenGLErrors("error before glewInit");
    glewExperimental = true; // Needed for core profile
    int glewInitialized = glewInit();
    GLX_CORE_ASSERT(glewInitialized == GLEW_OK, "Failed to initialize GLEW")

    checkOpenGLErrors("known error after glewInit");
    
    glfwSetWindowSizeCallback(window, Renderer::framebuffer_size_callback);
    

    glClearColor(1.f, 0.f, 0.2f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    // glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();


    m_frameDuration = std::chrono::milliseconds(1000 / 60); // 60 fps


    m_mainProgram = std::move(Program(galaxy::engineRes("shaders/vertex.glsl"), galaxy::engineRes("shaders/fragment.glsl")));
    
    checkOpenGLErrors("Renderer constructor");
}

Renderer::~Renderer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(Renderer::getInstance().window);
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




    m_mainProgram.use();

    m_mainProgram.updateViewMatrix(m_camManager.getViewMatrix());

    for(int instanceIdx=0; instanceIdx<instanceCount; instanceIdx++){
        m_mainProgram.updateModelMatrix(m_visuInstances[instanceIdx].second->getGlobalModelMatrix());
        m_visuInstances[instanceIdx].first.draw();
    }


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers
    glfwSwapBuffers(window);

    auto frameEnd = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);
    if (elapsed < m_frameDuration)
        std::this_thread::sleep_for(m_frameDuration - elapsed);
}

camID Renderer::addCamera(Transform *transformRef)
{
    return m_camManager.registerCam(transformRef);
}
void Renderer::setCurrentCamera(camID id)
{
    m_camManager.setCurrent(id);
}
void Renderer::removeCamera(camID id)
{
    m_camManager.unregisterCam(id);
}
renderID Renderer::instanciateMesh(Transform *transformRef, std::vector<Vertex> &vertices, std::vector<short unsigned int> &indices)
{
    if(m_freeIds.size() == 0) return -1;

    VisualInstance meshInstance;
    meshInstance.init(vertices, indices);
    
    renderID meshID = m_freeIds.top();
    size_t listIdx = instanceCount;
    m_visuInstances[listIdx] = std::make_pair(std::move(meshInstance), transformRef);
    m_visuIdxToInstanceId[listIdx] = meshID;
    m_instanceIdToVisuIdx[meshID] = listIdx;
    
    m_freeIds.pop();
    instanceCount ++;
    return meshID;
}

void Renderer::clearMesh(renderID meshID)
{
    size_t idxToDelete = m_instanceIdToVisuIdx[meshID];

    instanceCount--;

    renderID movedMeshID = m_visuIdxToInstanceId[instanceCount];
    m_instanceIdToVisuIdx[movedMeshID] = idxToDelete;
    m_visuIdxToInstanceId[idxToDelete] = movedMeshID;

    m_visuInstances[idxToDelete] = std::move(m_visuInstances[instanceCount]);

    m_freeIds.emplace(meshID);
}
