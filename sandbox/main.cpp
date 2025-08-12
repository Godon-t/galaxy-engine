#include <iostream>

#include <Engine.hpp>
#include <engine/core/EntryPoint.hpp>

#include "EditorLayer.hpp"

class Sandbox : public Galaxy::Application {
public:
    Sandbox()
    {
        pushOverlay(new Galaxy::EditorLayer());
    }
    ~Sandbox() { }
};

Galaxy::Application* Galaxy::createApplication()
{
    return new Sandbox();
}
