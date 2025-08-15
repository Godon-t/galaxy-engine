#pragma once

#include "engine/data/Transform.hpp"
#include "engine/types/Render.hpp"

#include <list>

namespace Galaxy {
// TODO: Search how to make it correctly
class CameraManager {
private:
    std::unordered_map<camID, Transform*> m_registeredCams;
    std::list<camID> m_activeCamsHistory;
    std::stack<camID> m_freeIds;

    static CameraManager* s_instance;

public:
    CameraManager();
    camID registerCam(Transform* transformRef);
    void unregisterCam(camID id);

    void updateCurrent(camID id, bool state);

    mat4 getCurrentCamTransform();

    static mat4 processViewMatrix(mat4& transform);
    static inline CameraManager& getInstance() { return *s_instance; }
};
}
