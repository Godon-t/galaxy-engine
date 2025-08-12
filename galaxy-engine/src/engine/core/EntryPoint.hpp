#pragma once

extern Galaxy::Application* Galaxy::createApplication();

int main(int argc, char const* argv[])
{
    Galaxy::Log::Init();

    auto app = Galaxy::createApplication();
    app->run();
    delete app;
}
