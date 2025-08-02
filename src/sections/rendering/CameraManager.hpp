#pragma once

#include <vector>
#include <stack>
#include <unordered_map>

#include <src/types/Render.hpp>
#include <src/data/Transform.hpp>

class CameraManager {
private:
    std::unordered_map<camID, std::pair<Transform*, bool>> registeredCamTransforms;
    std::stack<camID> activeCamsHistory;
    std::stack<camID> freeIds;

    mat4 getCurrentCamTransform();
public:
    CameraManager();
    camID registerCam(Transform* transformRef);
    void unregisterCam(camID id);

    void setCurrent(camID id);

    mat4 getViewMatrix();
};
