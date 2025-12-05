#include "CameraManager.hpp"

#include "pch.hpp"
#include "types/Math.hpp"

using namespace math;

namespace Galaxy {
CameraManager* CameraManager::s_instance = new CameraManager();

const size_t maxIds = 64;
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

    m_registeredCams[id] = transformRef;
    return id;
}

void CameraManager::unregisterCam(camID id)
{
    m_registeredCams.erase(id);
    m_activeCamsHistory.remove(id);
    m_freeIds.emplace(id);
}

void CameraManager::updateCurrent(camID id, bool state)
{
    if (state) {
        m_activeCamsHistory.emplace_back(id);
    } else {
        m_activeCamsHistory.remove(id);
    }
}

mat4 CameraManager::getCurrentCamTransform()
{
    if (m_activeCamsHistory.size() == 0)
        return mat4(1);
    camID topId = m_activeCamsHistory.back();
    if (m_registeredCams.find(topId) != m_registeredCams.end()) {
        if (m_registeredCams[topId])
            return m_registeredCams[topId]->getGlobalModelMatrix();
        else {
            m_activeCamsHistory.pop_back();
            return getCurrentCamTransform();
        }
    }
    return mat4(1);
}

mat4 CameraManager::processProjectionMatrix(vec2 viewPortDimmensions)
{
    return perspective(radians(45.f), viewPortDimmensions.x / viewPortDimmensions.y, 0.1f, 999.f);
}

mat4 CameraManager::processViewMatrix(const mat4& transform)
{
    vec3 position = vec3(transform[3][0], transform[3][1], transform[3][2]);
    vec3 forward  = vec3(transform[2][0], transform[2][1], transform[2][2]);
    vec3 target   = position + forward;
    return lookAt(position, target, vec3(0, 1, 0));
}

mat4 CameraManager::processViewMatrix(mat4& transform)
{
    vec3 position = vec3(transform[3][0], transform[3][1], transform[3][2]);
    vec3 forward  = vec3(transform[2][0], transform[2][1], transform[2][2]);
    vec3 target   = position + forward;
    return lookAt(position, target, vec3(0, 1, 0));
}
}
