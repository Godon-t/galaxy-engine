#include <iostream>

#include <Engine.hpp>

class Sandbox : public Galaxy::Application {
public:
    Sandbox(){}
    ~Sandbox(){}
};

Galaxy::Application* Galaxy::createApplication(){
    return new Sandbox();
}
