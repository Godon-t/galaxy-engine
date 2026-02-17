#pragma once

#include "types/Math.hpp"

using namespace math;

namespace Galaxy
{
    struct Camera {
        vec3 position{0,0,0};
        vec3 forward {0,0,1};
        vec3 up      {0,1,0};
        vec3 right   {1,0,0};

        float zNear     = 0.1;
        float zFar      = 999.0;
        vec2 dimmensions{256, 256};
    };
    
} // namespace Galaxy
