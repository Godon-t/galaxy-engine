#pragma once

extern Galaxy::Application* Galaxy::createApplication(int argc, char const* argv[]);

int main(int argc, char const* argv[])
{
    Galaxy::Log::Init();

    auto app = Galaxy::createApplication(argc, argv);
    app->run();
    delete app;
}
