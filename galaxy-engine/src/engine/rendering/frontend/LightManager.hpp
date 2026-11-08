#pragma once

#include <queue>
#include <unordered_map>
#include <vector>

#include "engine/data/Transform.hpp"
#include "engine/nodes/Node.hpp"
#include "engine/nodes/rendering/lighting/SpotLight.hpp"
#include "engine/types/Math.hpp"
#include "engine/types/Render.hpp"
#include "rendering/GPUInstances/GpuResourceHandle.hpp"
#include "rendering/Commands.hpp"


namespace Galaxy {
const unsigned int maxLightCount = 32;

enum LightType {
    POINTLIGHT = 0,
    SPOTLIGHT = 1,
    DIRECTIONLIGHT = 2,
};
struct LightData {
    LightType type = POINTLIGHT;
    int idx = -1;
    math::mat4 transformationMatrix { 1.0f };
    int shadowMapLayer = -1;
    vec3 color { 1.0f };
    float intensity = 0.5f;
    float range = 1.0f;
    float innerCutoff = 22.5f;
    float outerCutoff = 22.5f;
    bool castShadow = false;

    mat4 getProjection(const vec2& viewportDimmension) const;

    LightData() = default;
    LightData(int lightIdx, math::mat4& matrix)
        : idx(lightIdx)
        , transformationMatrix(matrix)
    {
    }
};

struct GPULightData {
    glm::vec4 positions[maxLightCount];
    glm::vec4 directions[maxLightCount];
    glm::vec4 colors[maxLightCount];
    glm::vec4 params[maxLightCount];

    struct alignas(16) ShadowLayer {
        int layer;
        int type; 
        int castShadow;
        int pad0;
    };
    ShadowLayer shadowMapLayers[maxLightCount];

    mat4 lightMatrices[maxLightCount];
    int count;
    int pad0, pad1, pad2;
};

class LightManager {
public:
    LightManager();
    ~LightManager();

    void init();

    int registerLight(LightData desc);
    // Direction is the direction in which rays travel in world space (local -Z).
    // Directional lights share the light budget; range/cutoffs are unused and shadows are unsupported.
    int registerDirectionalLight(const vec3& direction, const vec3& color = vec3(1.0f), float intensity = 1.0f);
    void updateLightDirection(lightID id, const vec3& direction);
    void updateLightTransform(lightID id, math::mat4 transform);
    void updateLightColor(lightID id, math::vec3 color);
    void updateLightIntensity(lightID id, float intensity);
    void updateLightRange(lightID id, float range);
    void updateLightCutoffs(lightID id, float innerCutoff, float outerCutoff);
    void updateLightCastShadow(lightID id, bool state);

    void unregisterLight(int id);
    int getShadowMapLayer(lightID light) const { return m_lights.at(light).shadowMapLayer; }

    unsigned int getProbesRadianceTexture();

    unsigned int getLightCount() const {return m_lights.size();}

    void updateProbeField();
    void updateBias(float newValue);
    void resizeProbeFieldGrid(unsigned int width, unsigned int height, unsigned int depth, float spaceBetween = 10.f, unsigned int probeResolution = 512, vec3 probeFieldCenter = vec3(0));
    void debugDraw();
    std::vector<vec3> getProbePositions();

    bool isDirty() const {return m_dirty;}
    UpdateUBOCommand getLightUboUpdate();
    [[nodiscard]] BufferHandle getLightUboHandle()const { return m_lightsUBO; }
    const std::vector<LightData> getLightsData() const;

private:
    LightData* findLight(lightID id);
    struct ProbeCell {
        // from 0 to 1 with order x, y, z
        unsigned int probes[8];
    };
    struct ProbeData {
        unsigned int probeCoord;
        vec3 position;
    };

    std::unordered_map<lightID, LightData> m_lights;
    std::priority_queue<lightID, std::vector<lightID>, std::greater<lightID>> m_availableIDs; 
    int m_maxLightCount = maxLightCount;

    FramebufferHandle m_shadowMapFramebuffer;

    unsigned int getCellCoord(unsigned int x, unsigned int y, unsigned int z);
    vec2 getProbeTexCoord(unsigned int probeGridIdx);

    GeometryHandle m_fullQuad;
    CubemapFramebufferHandle m_cubemapFramebuffer;
    FramebufferHandle m_probesFramebuffer;

    BufferHandle m_lightsUBO;
    GPULightData m_lightUniformData {};

    bool m_dirty;

    unsigned int m_probeResolution;
    unsigned int m_textureWidth;
    unsigned int m_textureHeight;
    unsigned int m_gridDimX;
    unsigned int m_gridDimY;
    unsigned int m_gridDimZ;
    float m_probeDistance;
    vec3 m_probeFieldStart;

    std::vector<ProbeData> m_probeGrid;

    Transform m_debugStartTransform, m_debugEndTransform;
    GeometryHandle m_debugStartGeometry;
    GeometryHandle m_debugEndGeometry;
};
} // namespace Galaxy
