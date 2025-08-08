#pragma once

#include "engine/data/Transform.hpp"
#include "engine/types/Render.hpp"

class CameraManager {
private:
    std::unordered_map<camID, std::pair<Transform*, bool>> m_registeredCamTransforms;
    std::stack<camID> m_activeCamsHistory;
    std::stack<camID> m_freeIds;

    mat4 getCurrentCamTransform();

public:
    CameraManager();
    camID registerCam(Transform* transformRef);
    void unregisterCam(camID id);

    void setCurrent(camID id);

    mat4 getViewMatrix();
};
