#pragma once

#include "engine/nodes/Node.hpp"
#include "engine/nodes/rendering/lighting/SpotLight.hpp"

#include "engine/types/Math.hpp"
#include "engine/types/Render.hpp"

namespace Galaxy {
class LightManager {
public:
    LightManager();
    ~LightManager();

    void init();

    int registerLight(const SpotLight* desc);
    void updateLightPos(int id, math::vec3 position);
    void updateLightTransform(lightID id, math::mat4 transform);
    void unregisterLight(int id);
    void shadowPass(Node* sceneRoot);
    renderID getShadowMapID(lightID light) { return m_lights[light].shadowMapID; }

private:
    struct LightData {
        int idx;
        math::mat4 lightSpaceMatrix;
        renderID shadowMapID;

        LightData()
            : idx(-1)
            , shadowMapID(0)
        {
        }
        LightData(int lightIdx, math::mat4& matrix)
            : idx(lightIdx)
            , lightSpaceMatrix(matrix)
            , shadowMapID(0)
        {
        }
    };

    std::unordered_map<lightID, LightData> m_lights;
    lightID m_nextLightID   = 0;
    int m_currentLightCount = 0;
    int m_maxLights         = 100;

    renderID m_shadowMapFrameBufferID;
};
} // namespace Galaxy
