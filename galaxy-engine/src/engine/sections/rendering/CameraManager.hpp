#pragma once

#include "engine/data/Transform.hpp"
#include "engine/types/Render.hpp"

namespace Galaxy {
class CameraManager {
private:
    std::unordered_map<camID, std::pair<Transform*, bool>> m_registeredCamTransforms;
    std::stack<camID> m_activeCamsHistory;
    std::stack<camID> m_freeIds;

    static CameraManager* s_instance;

public:
    CameraManager();
    camID registerCam(Transform* transformRef);
    void unregisterCam(camID id);

    void setCurrent(camID id);

    mat4 getCurrentCamTransform();

    static mat4 processViewMatrix(mat4& transform);
    static inline CameraManager& getInstance() { return *s_instance; }
};
}
