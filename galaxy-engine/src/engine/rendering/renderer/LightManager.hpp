#pragma once

#include "engine/data/Transform.hpp"
#include "engine/nodes/Node.hpp"
#include "engine/nodes/rendering/lighting/SpotLight.hpp"

#include "engine/types/Math.hpp"
#include "engine/types/Render.hpp"

namespace Galaxy {
enum LightType {
    SPOTLIGHT = 0,
    POINTLIGHT,
};
struct LightData {
    LightType type;
    int idx;
    math::mat4 transformationMatrix;
    renderID shadowMapID;
    bool needUpdate = true;
    vec3 color;
    float intensity;

    LightData()
        : idx(-1)
        , shadowMapID(0)
        , color(1)
    {
    }
    LightData(int lightIdx, math::mat4& matrix)
        : idx(lightIdx)
        , transformationMatrix(matrix)
        , shadowMapID(0)
        , color(1)
    {
    }
};

class LightManager {
public:
    LightManager();
    ~LightManager();

    void init();

    int registerLight(LightData desc);
    void updateLightTransform(lightID id, math::mat4 transform);
    void updateLightColor(lightID id, math::vec3 color);
    void unregisterLight(int id);
    void shadowPass(Node* sceneRoot);
    renderID getShadowMapID(lightID light) { return m_lights[light].shadowMapID; }

    renderID getProbesRadianceTexture();

    void updateProbeField();
    void resizeProbeFieldGrid(unsigned int width, unsigned int height, unsigned int depth, float spaceBetween = 10.f);
    void debugDraw();

private:
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
    renderID m_colorRenderingCubemap;
    renderID m_normalRenderingCubemap;
    renderID m_depthRenderingCubemap;

    renderID m_probesFrameBuffer;
    renderID m_probeFieldColor;
    renderID m_probeFieldNormal;
    renderID m_probeFieldDepth;

    unsigned int m_probeResolution;
    unsigned int m_textureWidth;
    unsigned int m_textureHeight;
    renderID m_probeRadianceTexture;
    renderID m_probeDepthTexture;
    unsigned int m_gridDimX;
    unsigned int m_gridDimY;
    unsigned int m_gridDimZ;
    float m_probeDistance;

    std::vector<ProbeData> m_probeGrid;

    Transform m_debugStartTransform, m_debugEndTransform;
    renderID m_debugStartVisu, m_debugEndVisu;
};
} // namespace Galaxy
