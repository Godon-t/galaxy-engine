#include <iostream>

#include <Engine.hpp>

class ExampleLayer : public Galaxy::Layer {
public:
    ExampleLayer()
        : Layer("Example")
    {
    }

    void onUpdate() override
    {
        // GLX_INFO("Update");
    }
};

class Sandbox : public Galaxy::Application {
public:
    Sandbox()
    {
        pushLayer(new ExampleLayer());
        pushOverlay(new Galaxy::ImGuiLayer());
    }
    ~Sandbox() { }
};

Galaxy::Application* Galaxy::createApplication()
{
    return new Sandbox();
}
