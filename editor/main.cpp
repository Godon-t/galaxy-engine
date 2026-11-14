#include <iostream>

#include <Engine.hpp>
#include <engine/core/EntryPoint.hpp>

#include "EditorLayer.hpp"

class Sandbox : public Galaxy::Application {
public:
    Sandbox(const char* projectPath = ".gproj")
    {
        pushOverlay(new Galaxy::EditorLayer(projectPath));
    }
    ~Sandbox() { }
};

Galaxy::Application* Galaxy::createApplication(int argc, char const* argv[])
{
    if (argc >= 2) {
        return new Sandbox(argv[1]);
    }
    return new Sandbox();
}
