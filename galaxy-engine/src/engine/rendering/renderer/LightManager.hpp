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

    renderID getProbesRadianceTexture();

    void updateProbeField();
    void resizeProbeFieldGrid(unsigned int width, unsigned int height, unsigned int depth, float spaceBetween = 10.f);

private:
    struct LightData {
        int idx;
        math::mat4 transformationMatrix;
        renderID shadowMapID;

        LightData()
            : idx(-1)
            , shadowMapID(0)
        {
        }
        LightData(int lightIdx, math::mat4& matrix)
            : idx(lightIdx)
            , transformationMatrix(matrix)
            , shadowMapID(0)
        {
        }
    };

    struct ProbeCell {
        // from 0 to 1 with order x, y, z
        unsigned int probes[8];
    };
    struct ProbeData {
        unsigned int probeCoord;
        vec3 position;
    };

    std::unordered_map<lightID, LightData> m_lights;
    lightID m_nextLightID   = 0;
    int m_currentLightCount = 0;
    int m_maxLights         = 100;

    renderID m_shadowMapFrameBufferID;

    unsigned int getCellCoord(unsigned int x, unsigned int y, unsigned int z);
    vec2 getProbeTexCoord(unsigned int probeGridIdx);

    renderID m_fullQuad;
    renderID m_renderingCubemap;
    renderID m_probesFrameBuffer;
    unsigned int m_probeResolution;
    unsigned int m_textureWidth;
    unsigned int m_textureHeight;
    renderID m_probeRadianceTexture;
    unsigned int m_gridDimX;
    unsigned int m_gridDimY;
    unsigned int m_gridDimZ;
    float m_probeDistance;

    std::vector<ProbeData> m_probeGrid;
};
} // namespace Galaxy
