#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/ext.hpp>
namespace math {
    using vec3 = glm::vec3;
    using vec2 = glm::vec2;
    using mat4 = glm::mat4;
    using quat = glm::quat;
    
    using glm::length;
    using glm::eulerAngles;
    using glm::degrees;
    using glm::translate;
    using glm::rotate;
    using glm::scale;
    using glm::toMat4;
    using glm::radians;
    using glm::perspective;
    using glm::lookAt;
}
