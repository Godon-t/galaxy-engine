#include "pch.hpp"
#include "CameraManager.hpp"


#include "engine/types/Math.hpp"

using namespace math;

const size_t maxIds = 256;
CameraManager::CameraManager()
{
    for(size_t i=0; i<maxIds; i++){
        freeIds.push(i);
    }
}

camID CameraManager::registerCam(Transform *transformRef)
{
    camID id = freeIds.top();
    freeIds.pop();

    registeredCamTransforms[id] = std::make_pair(transformRef, false);
    return id;
}

void CameraManager::unregisterCam(camID id)
{
    registeredCamTransforms.erase(id);
    freeIds.emplace(id);
}

void CameraManager::setCurrent(camID id)
{
    registeredCamTransforms[id].second = true;
    activeCamsHistory.emplace(id);
}

mat4 CameraManager::getCurrentCamTransform()
{
    if(activeCamsHistory.size() == 0) return lookAt(vec3(0,0,0), vec3(0,0,1), vec3(0,1,0));
    camID topId = activeCamsHistory.top();
    if(registeredCamTransforms.find(topId) != registeredCamTransforms.end()){
        if(registeredCamTransforms[topId].second) 
            return registeredCamTransforms[topId].first->getGlobalModelMatrix();
        else {
            activeCamsHistory.pop();
            return getCurrentCamTransform();
        }
    }
    return lookAt(vec3(0,0,0), vec3(0,0,1), vec3(0,1,0));
}

mat4 CameraManager::getViewMatrix()
{
    mat4 tansMat = getCurrentCamTransform();
    vec3 position = vec3(tansMat[3][0], tansMat[3][1], tansMat[3][2]);
    vec3 forward = vec3(tansMat[2][0], tansMat[2][1], tansMat[2][2]);
    vec3 target = position + forward;
    return lookAt(position, target, vec3(0,1,0));
}
