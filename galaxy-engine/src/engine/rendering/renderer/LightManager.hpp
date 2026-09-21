#pragma once

#include "engine/data/Transform.hpp"
#include "engine/nodes/Node.hpp"
#include "engine/nodes/rendering/lighting/SpotLight.hpp"

#include "engine/types/Math.hpp"
#include "engine/types/Render.hpp"
#include "rendering/renderer/resources/GpuResourceHandle.hpp"
#include "rendering/renderer/commands/RenderCommand.hpp"

namespace Galaxy {
struct RenderCameraTransform;

const unsigned int maxLightCount = 32;

enum LightType {
    SPOTLIGHT = 0,
    POINTLIGHT,
};
struct LightData {
    LightType type;
    int idx;
    math::mat4 transformationMatrix;
    int shadowMapLayer;
    vec3 color;
    float intensity;
    float range;

    LightData()
        : idx(-1)
        , shadowMapLayer(0)
        , color(1)
        , intensity(0.5)
        , range(1.0)
    {
    }
    LightData(int lightIdx, math::mat4& matrix)
        : idx(lightIdx)
        , transformationMatrix(matrix)
        , shadowMapLayer(0)
        , color(1)
        , intensity(0.5)
        , range(1.0)
    {
    }
};

struct GPULightData {
    glm::vec4 positions[maxLightCount];
    glm::vec4 colors[maxLightCount];
    glm::vec4 params[maxLightCount];

    struct alignas(16) ShadowLayer {
        int layer;
        int pad0, pad1, pad2;
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
    void updateLightTransform(lightID id, math::mat4 transform);
    void updateLightColor(lightID id, math::vec3 color);
    void updateLightIntensity(lightID id, float intensity);
    void updateLightRange(lightID id, float range);
    void unregisterLight(int id);
    void shadowPass(Node* sceneRoot);
    unsigned int getShadowMapLayer(lightID light) { return m_lights[light].shadowMapLayer; }

    unsigned int getProbesRadianceTexture();

    unsigned int getLightCount() const {return m_currentLightCount;}

    void updateProbeField();
    void updateBias(float newValue);
    void resizeProbeFieldGrid(unsigned int width, unsigned int height, unsigned int depth, float spaceBetween = 10.f, unsigned int probeResolution = 512, vec3 probeFieldCenter = vec3(0));
    void debugDraw();
    std::vector<vec3> getProbePositions();

    bool isDirty() const {return m_dirty;}
    UpdateUBOCommand getLightUboUpdate();
    std::vector<std::unique_ptr<RenderCameraTransform>> getLightsDevices();

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

    FramebufferHandle m_shadowMapFramebuffer;

    unsigned int getCellCoord(unsigned int x, unsigned int y, unsigned int z);
    vec2 getProbeTexCoord(unsigned int probeGridIdx);

    GeometryHandle m_fullQuad;
    CubemapFramebufferHandle m_cubemapFramebuffer;
    FramebufferHandle m_probesFramebuffer;

    BufferHandle m_lightsUBO;
    GPULightData m_lightUniformData;

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
