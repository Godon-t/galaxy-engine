#pragma once

#include "Core.hpp"

namespace Galaxy {
    class API Application {
    public:
        Application();
        virtual ~Application();
    
        void run();
    };

    // Defined client side
    Application* createApplication();
}

