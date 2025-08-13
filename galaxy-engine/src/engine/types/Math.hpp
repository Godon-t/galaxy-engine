#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace math {
using vec4 = glm::vec4;
using vec3 = glm::vec3;
using vec2 = glm::vec2;
using mat4 = glm::mat4;
using quat = glm::quat;

using glm::degrees;
using glm::eulerAngles;
using glm::length;
using glm::lookAt;
using glm::perspective;
using glm::radians;
using glm::rotate;
using glm::scale;
using glm::toMat4;
using glm::translate;
} // namespace math
