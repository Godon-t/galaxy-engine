#include "EditorLayer.hpp"
#include "Engine.hpp"
#include "editor/NodeListPanel.hpp"

#include <imgui.h>

namespace Galaxy {
std::unordered_map<Galaxy::uuid, std::string> ResourceAccess::paths;

EditorLayer::EditorLayer(const char* projectPath)
    : Layer("Editor layer")
    , m_mode(EditorMode::Edit)
    , m_showAllNodes(false)
{
    if (!Project::load(projectPath)) {
        GLX_ERROR("Can't load project '{0}', creating one", projectPath);
        Project::create(projectPath);
    } else {
        ResourceAccess::paths = Project::getPaths(ProjectPathTypes::RESOURCE);
    }
}

EditorLayer::~EditorLayer()
{
}

void EditorLayer::onAttach()
{
    m_rootEditorNode = std::make_shared<Node>("Editor");
    m_editorCamera   = std::make_shared<Camera>();
    m_editorCamera->setCurrent(false);
    m_rootEditorNode->addChild(m_editorCamera);
    m_rootEditorNode->activate();

    m_fileDialog.SetDirectory(Project::getProjectRootPath());

    m_rootSceneNode = std::make_shared<Node>("RootScene");
    m_rootSceneNode->disable();

    std::shared_ptr<Node> rootNode = std::make_shared<Node>("Root");
    rootNode->addChild(m_rootEditorNode);
    rootNode->addChild(m_rootSceneNode);

    Application::getInstance().setRootNode(rootNode);

    auto& renderer  = Renderer::getInstance();
    m_viewportFrame = new FrameBuffer(320, 180, FramebufferTextureFormat::DEPTH24STENCIL8);

    InputManager::addAction(Action(GLX_KEY_ESCAPE, "editor_exit"));
    InputManager::addAction(Action(GLX_KEY_W, "editor_forward"));
    InputManager::addAction(Action(GLX_KEY_S, "editor_backward"));
    InputManager::addAction(Action(GLX_KEY_A, "editor_right"));
    InputManager::addAction(Action(GLX_KEY_D, "editor_left"));

    InputManager::addAction(Action(GLX_KEY_Q, "editor_down"));
    InputManager::addAction(Action(GLX_KEY_E, "editor_up"));

    Renderer::getInstance().changeUsedProgram(BaseProgramEnum::PBR);
    mat4 proj = CameraManager::processProjectionMatrix(vec2(320, 180));
    Renderer::getInstance().setProjectionMatrix(proj);
}

void EditorLayer::onDetach()
{
    m_viewportFrame->destroy();
}

void EditorLayer::onUpdate()
{
    m_viewportFrame->bind();
    if (Project::isSceneValid(m_selectedSceneId)) {
        m_selectedScene = &Project::loadScene(m_selectedSceneId);
    } else {
        m_selectedScene = nullptr;
        m_viewportFrame->unbind();
        return;
    }

    if (m_selectedScene->getNodePtr()) {
        auto& renderer = Renderer::getInstance();
        mat4 cameraTransform;
        if (m_mode == EditorMode::Run) {
            cameraTransform = CameraManager::getInstance().getCurrentCamTransform();
        } else {
            cameraTransform = m_editorCamera->getTransform()->getGlobalModelMatrix();
            updateCamera();
        }
        renderer.beginSceneRender(cameraTransform);

        m_selectedScene->getNodePtr()->draw();

        renderer.endSceneRender();
        renderer.renderFrame();
    }
    m_viewportFrame->unbind();
}

void EditorLayer::displayMenuBar(bool validScene)
{
    static bool newScene = false;
    ImGui::BeginMenuBar();
    if (ImGui::BeginMenu("File")) {
        if (validScene && ImGui::MenuItem("Save project")) {
            Project::save();
        }
        if (ImGui::MenuItem("New scene"))
            newScene = true;
        if (ImGui::BeginMenu("Load")) {
            int i = 0;
            for (auto& scene : Project::getPaths(ProjectPathTypes::SCENE)) {
                if (ImGui::MenuItem(std::string(scene.second + "##" + std::to_string(i++)).c_str())) {
                    m_selectedScene   = &Project::loadScene(scene.first);
                    m_selectedSceneId = m_selectedScene->getUuid();
                    m_rootSceneNode->clearChilds();
                    m_rootSceneNode->addChild(m_selectedScene->getNodePtr());
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Import file"))
            m_fileDialog.Open();
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();

    m_fileDialog.Display();

    if (m_fileDialog.HasSelected()) {
        std::string filePath = Project::toRelativePath(m_fileDialog.GetSelected().string());

        if (ResourceImporter::import(filePath)) {
            ResourceAccess::paths = Project::getPaths(ProjectPathTypes::RESOURCE);
            GLX_INFO("File imported");
        } else {
            GLX_ERROR("Error importing '{0}'", filePath);
        }
        m_fileDialog.ClearSelected();
    }

    if (newScene) {
        ImGui::Begin("New Scene window");
        static char sceneName[128];
        ImGui::InputText("Scene's path: ", sceneName, IM_ARRAYSIZE(sceneName));
        if (ImGui::Button("OK")) {
            m_selectedScene   = &Project::createScene(std::string(sceneName));
            m_selectedSceneId = m_selectedScene->getUuid();
            newScene          = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("close")) {
            newScene = false;
        }
        ImGui::End();
    }
}

void EditorLayer::displayViewport(bool validScene)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 { 0, 0 });
    ImGui::Begin("Viewport");
    if (validScene && m_selectedScene->getNodePtr().get()) {
        if (ImGui::Button("Run")) {
            m_mode = EditorMode::Run;
            m_rootSceneNode->activate();
            m_rootEditorNode->disable();
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            m_mode = EditorMode::Edit;
            m_rootSceneNode->disable();
            m_rootEditorNode->activate();
        }
        if (!ImGui::IsWindowDocked()) {
            ImGui::PopStyleVar();
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 { 100, 1000 });
        }
        ImVec2 pannelSize = ImGui::GetContentRegionAvail();
        if (m_viewportSize != *(vec2*)&pannelSize) {
            m_viewportSize = { pannelSize.x, pannelSize.y };
            m_viewportFrame->resize(m_viewportSize.x, m_viewportSize.y);
        }
        ImGui::PopStyleVar();
        auto textureID = m_viewportFrame->getColorTextureID();
        ImGui::Image(reinterpret_cast<void*>(textureID), pannelSize, ImVec2 { 1, 1 }, ImVec2 { 0, 0 });
    } else {
        ImGui::PopStyleVar();
    }

    ImGui::End();
}

void EditorLayer::onImGuiRender()
{
    bool validScene           = Project::isSceneValid(m_selectedSceneId);
    static bool dockSpaceOpen = true;
    if (dockSpaceOpen) {
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
        ImGuiViewport* viewport      = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus; // | ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Dock", &dockSpaceOpen, windowFlags);
        ImGui::PopStyleVar(1);

        ImGuiID dockspaceID = ImGui::GetID("DockSpace");
        ImGui::DockSpace(dockspaceID);

        displayMenuBar(validScene);

        ImGui::End();
    }

    ImGui::Begin("App");

    double fps = 1.0 / Application::getInstance().getDelta();

    static std::vector<float> fpsHistory(300, 0.0f);
    static size_t offset = 0;

    fpsHistory[offset] = (float)fps;
    offset             = (offset + 1) % fpsHistory.size();

    ImGui::Text("FPS: %.1f", fps);
    ImGui::PlotHistogram("FPS history", fpsHistory.data(), fpsHistory.size(), offset, nullptr, 0.0f, 60.0f, ImVec2(0, 100));
    ImGui::Checkbox("Show all nodes", &m_showAllNodes);

    if (ImGui::Button("Test resource"))
        m_resourceAccess.show();
    if (m_resourceAccess.begin()) {
        GLX_INFO("Selected resource! '{0}'", m_resourceAccess.selectedResourcePath);
    }

    ImGui::End();

    displayViewport(validScene);

    ImGui::Begin("Scene");

    if (m_showAllNodes)
        m_nodeList.listNodes(*Application::getInstance().getRootNodePtr());
    else if (validScene)
        m_nodeList.listNodes(*m_selectedScene->getNodePtr().get());
    if (Node::nodeExists(m_nodeList.selectedNodeId)) {
        m_editNode.selectNode(*Node::getNode(m_nodeList.selectedNodeId).lock().get());
    }
    ImGui::End();
}

void EditorLayer::onEvent(Event& evt)
{
    if (evt.getEventType() == EventType::ActionInteract) {
        ActionEvent& actEvt = (ActionEvent&)evt;
        if (std::string(actEvt.getActionName()) == "editor_exit")
            Application::getInstance().terminate();
    } else if (evt.getEventType() == EventType::MouseScroll) {
        MouseScrollEvent& scrollEvt = (MouseScrollEvent&)evt;
        m_cameraSpeed += scrollEvt.getYOffset() * 0.001;
        m_cameraSpeed = m_cameraSpeed <= 0 ? 0.0001 : m_cameraSpeed;
    } else if (evt.getEventType() == EventType::MouseMotion && m_rightClickDown) {
        MouseMotionEvent& mouseMotion = (MouseMotionEvent&)evt;
        m_editorCamera->getTransform()->localRotateY(mouseMotion.getDeltaX() * 0.001);
        m_editorCamera->getTransform()->localRotateX(mouseMotion.getDeltaY() * 0.001);
    } else if (evt.getEventType() == EventType::MouseButtonInteract) {
        MouseButtonEvent& mouseBtn = (MouseButtonEvent&)evt;
        if (mouseBtn.getButton() == 1) {
            m_rightClickDown = mouseBtn.isPressed();
        }
    } else if (evt.getEventType() == EventType::WindowResize) {
        WindowResizeEvent& resize = (WindowResizeEvent&)evt;
        mat4 proj                 = CameraManager::processProjectionMatrix(vec2(resize.getWidth(), resize.getHeight()));
        Renderer::getInstance().setProjectionMatrix(proj);
    }
}
void EditorLayer::updateCamera()
{
    vec3 translation(0);
    vec3 forwardVec = m_editorCamera->getForward();
    vec3 leftVec    = cross(forwardVec, vec3(0, 1, 0));
    forwardVec *= m_cameraSpeed;
    leftVec *= m_cameraSpeed;
    if (InputManager::isActionPressed("editor_forward"))
        m_editorCamera->translate(-forwardVec);
    if (InputManager::isActionPressed("editor_backward"))
        m_editorCamera->translate(forwardVec);
    if (InputManager::isActionPressed("editor_right"))
        m_editorCamera->translate(-leftVec);
    if (InputManager::isActionPressed("editor_left"))
        m_editorCamera->translate(leftVec);
    if (InputManager::isActionPressed("editor_up"))
        m_editorCamera->translate(vec3(0, m_cameraSpeed, 0));
    if (InputManager::isActionPressed("editor_down"))
        m_editorCamera->translate(vec3(0, -m_cameraSpeed, 0));
}
}