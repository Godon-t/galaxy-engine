#include "CameraManager.hpp"

#include "engine/types/Math.hpp"
#include "pch.hpp"

using namespace math;

namespace Galaxy {
const size_t maxIds = 256;
CameraManager::CameraManager()
{
    for (size_t i = 0; i < maxIds; i++) {
        m_freeIds.push(i);
    }
}

camID CameraManager::registerCam(Transform* transformRef)
{
    camID id = m_freeIds.top();
    m_freeIds.pop();

    m_registeredCamTransforms[id] = std::make_pair(transformRef, false);
    return id;
}

void CameraManager::unregisterCam(camID id)
{
    m_registeredCamTransforms.erase(id);
    m_freeIds.emplace(id);
}

void CameraManager::setCurrent(camID id)
{
    m_registeredCamTransforms[id].second = true;
    m_activeCamsHistory.emplace(id);
}

mat4 CameraManager::getCurrentCamTransform()
{
    if (m_activeCamsHistory.size() == 0)
        return lookAt(vec3(0, 0, 0), vec3(0, 0, 1), vec3(0, 1, 0));
    camID topId = m_activeCamsHistory.top();
    if (m_registeredCamTransforms.find(topId) != m_registeredCamTransforms.end()) {
        if (m_registeredCamTransforms[topId].second)
            return m_registeredCamTransforms[topId].first->getGlobalModelMatrix();
        else {
            m_activeCamsHistory.pop();
            return getCurrentCamTransform();
        }
    }
    return lookAt(vec3(0, 0, 0), vec3(0, 0, 1), vec3(0, 1, 0));
}

mat4 CameraManager::getViewMatrix()
{
    mat4 tansMat = getCurrentCamTransform();
    vec3 position = vec3(tansMat[3][0], tansMat[3][1], tansMat[3][2]);
    vec3 forward = vec3(tansMat[2][0], tansMat[2][1], tansMat[2][2]);
    vec3 target = position + forward;
    return lookAt(position, target, vec3(0, 1, 0));
}
}
