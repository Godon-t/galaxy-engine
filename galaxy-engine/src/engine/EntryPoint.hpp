#pragma once

extern Galaxy::Application* Galaxy::createApplication();

int main(int argc, char const *argv[])
{
    Galaxy::Log::Init();

    int test = 4;
    GLX_CORE_ERROR("aie test={0}", test);

    auto app = Galaxy::createApplication();
    app->run();
    delete app;
}
