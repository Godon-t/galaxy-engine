#include "Renderer.hpp"

#include "pch.hpp"

#include "Application.hpp"
#include "Core.hpp"
#include "gl_headers.hpp"
#include "rendering/CameraManager.hpp"
#include "rendering/GPUInstances/FrameBuffer.hpp"
#include "rendering/OpenglHelper.hpp"

namespace Galaxy {
Renderer::Renderer()
    : m_backend()
    , m_frontend(m_backend)
    , m_mainViewportSize(1024)
{
    m_backend.initDebugCallback();

    m_backend.onMaterialUpdated([this](MaterialHandle material, const MaterialRenderInfo& info) {
        m_frontend.notifyMaterialUpdated(material, info);
    });
    m_backend.onMaterialDestroyed([this](MaterialHandle material) {
        m_frontend.removeMaterialID(material);
    });
    

    m_sceneFramebuffer = m_frontend.getFinalFramebuffer();
}

Renderer::~Renderer()
{
    shutdown();
}

void Renderer::shutdown()
{
    m_backend.destroy();
}

Renderer& Renderer::getInstance()
{
    static Renderer renderer;
    return renderer;
}

void Renderer::init()
{
    m_frontend.getLightManager().init();
}

void Renderer::addMainCamera(const Camera& camera)
{
    Camera mainCamera = camera;
    mainCamera.viewportSize = m_mainViewportSize;
    mainCamera.viewportPosition = vec2(0);
    mainCamera.targetFramebuffer.reset();
    m_frontend.addCamera(std::move(mainCamera));
}

void Renderer::resize(unsigned int width, unsigned int height)
{
    m_mainViewportSize.x = width;
    m_mainViewportSize.y = height;
}

void Renderer::renderFrame()
{
    auto execution = m_frontend.buildFrameExecution();
    m_backend.frameReset();
    m_backend.execute(execution);
    
    m_frontend.clearContext();
}

// void Renderer::applyFilterOnCubemap(GeometryHandle skyboxMesh, CubemapHandle source, CubemapHandle target, FilterEnum filter)
// {
//     // switchCommandBuffer();
//     // m_commandBuffers[m_frontCommandBufferIdx].clear();

//     // std::function<void()> prgToUse;
//     // switch (filter) {
//     // case FilterEnum::IRRADIANCE:
//     //     prgToUse = [this]() {
//     //         m_frontend.changeUsedProgram(ProgramType::FILTER_IRRADIANCE);
//     //     };
//     //     break;
//     // default:
//     //     GLX_CORE_ERROR("Unknown filter applied!");
//     //     return;
//     // }

//     // Cubemap& targetCubemap = *m_backend.m_cubemapInstances.get(targetID);
//     // targetCubemap.useFloat = true;
//     // targetCubemap.resize(2048);
//     // CubemapFrameBuffer cubemapBuffer(targetCubemap);

//     // GLint viewport[4];
//     // glGetIntegerv(GL_VIEWPORT, viewport);
//     // glViewport(0, 0, targetCubemap.resolution, targetCubemap.resolution);

//     // vec2 dimmensions      = vec2(targetCubemap.resolution);
//     // auto projectionMatrix = CameraManager::processProjectionMatrix(vec2(dimmensions));

//     // mat4 baseProjection = m_frontend.getProjectionMatrix();
//     // mat4 projection     = perspective(radians(90.0f), 1.f, 0.001f, 999.f);
//     // m_frontend.setProjectionMatrix(projection);

//     // vec3 position = vec3(0, 0, 0);
//     // Transform transformTemp;
//     // for (int i = 0; i < 6; i++) {
//     //     auto viewMatrix = lookAt(camPosition, camPosition + camDirection, camUp);
//     //     m_frontend.beginCanva(viewMatrix, projectionMatrix, targetID, FramebufferTextureFormat::DEPTH24RGBA8, i);

//     //     beginSceneRender(position, m_cubemap_orientations[i], m_cubemap_ups[i], dimmensions);
//     //     prgToUse();
//     //     m_frontend.bindCubemap(sourceID, "skybox");
//     //     m_frontend.submit(skyboxMesh, transformTemp);
//     //     endSceneRender();
//     //     renderFrame();
//     // }

//     // m_frontend.setProjectionMatrix(baseProjection);
//     // glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
// }

}
