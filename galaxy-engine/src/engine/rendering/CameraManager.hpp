#pragma once

#include "data/Camera.hpp"
#include "types/Render.hpp"

#include <list>

namespace Galaxy {
// TODO: Search how to make it correctly
class CameraManager {
private:
    std::unordered_map<camID, std::shared_ptr<Camera>> m_registeredCams;
    std::list<camID> m_activeCamsHistory;
    std::stack<camID> m_freeIds;

    static CameraManager* s_instance;

public:
    CameraManager();
    camID registerCam(const std::shared_ptr<Camera> cameraData);
    void unregisterCam(camID id);

    void updateCurrent(camID id, bool state);

    bool hasCamera();
    const std::shared_ptr<Camera> getCurrentCamera();

    static mat4 processProjectionMatrix(vec2 viewPortDimmensions, float zNear = 0.1f, float zFar = 9999.0f, float outerCutoffAngle = 22.5f);
    static mat4 processViewMatrix(const mat4& transform);
    static mat4 processViewMatrix(std::shared_ptr<Camera> camera);
    static inline CameraManager& getInstance() { return *s_instance; }
};
}
