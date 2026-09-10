#include "Backend.hpp"

#include "Helper.hpp"
#include "Log.hpp"
#include "gl_headers.hpp"
#include "pch.hpp"
#include "rendering/OpenglHelper.hpp"

namespace Galaxy {
Backend::Backend(size_t maxSize)
    : m_visualInstances(maxSize)
    , m_textureInstances(maxSize * 2)
    , m_materialInstances(maxSize)
    , m_cubemapInstances(maxSize)
    , m_frameBufferInstances(maxSize)
    , m_cubemapFrameBufferInstances(maxSize)
    , m_uboInstances(maxSize)
    , m_activeProgram(&m_mainProgram)
{
    GLenum error = glGetError();

    checkOpenGLErrors("error before glewInit");
    glewExperimental    = true; // Needed for core profile
    int glewInitialized = glewInit();
    GLX_CORE_ASSERT(glewInitialized == GLEW_OK, "Failed to initialize GLEW")

    checkOpenGLErrors("known error after glewInit");

    glClearColor(1.f, 0.f, 0.2f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    // glDisable(GL_CULL_FACE);

    // Shader construction uses OpenGL entry points, so it must happen after GLEW.
    // Move assignment is safe here: Program is move-only and releases any old ID.
    m_mainProgram                = ProgramPBR(engineRes("shaders/base.glsl"));
    m_skyboxProgram              = ProgramSkybox(engineRes("shaders/skybox.glsl"));
    m_irradianceProgram          = ProgramSkybox(engineRes("shaders/filters/irradiance.glsl"));
    m_textureProgram             = ProgramTexture(engineRes("shaders/texture.glsl"));
    m_unicolorProgram            = ProgramUnicolor(engineRes("shaders/unicolor.glsl"));
    m_postProcessingProbeProgram = ProgramPostProc(engineRes("shaders/post_processing.glsl"));
    m_postProcessingSSGIProgram  = ProgramPostProcSSGI(engineRes("shaders/ssgi.glsl"));
    m_shadowProgram              = ProgramShadow(engineRes("shaders/shadow_depth.glsl"));
    m_computeOctahedralProgram   = ProgramComputeOctahedral(engineRes("shaders/compute_octahedral.glsl"));
    m_debugLinesProgram          = ProgramDebugLines(engineRes("shaders/debug/line_draw.glsl"));

    m_debugLines.init();

    checkOpenGLErrors("Renderer constructor");
}

Backend::~Backend()
{
    destroy();
}

renderID Backend::instantiateUBO(unsigned int dataSize)
{
    if (!m_uboInstances.canAddInstance())
        return 0;

    renderID uboID = m_uboInstances.createResourceInstance();

    m_uboInstances.get(uboID)->init(dataSize);

    return uboID;
}

void Backend::destroy()
{
    // Pending CPU-resource callbacks use a weak copy of this token.
    m_lifetimeToken.reset();
    m_cubemapUploadLifetimes.clear();
    m_debugLines.destroy();

    auto visualNotifications  = std::move(m_visualDestroyNotifications);
    auto textureNotifications = std::move(m_textureDestroyNotifications);
    auto materialNotifications = std::move(m_materialDestroyNotifications);
    m_visualDestroyNotifications.clear();
    m_textureDestroyNotifications.clear();
    m_materialDestroyNotifications.clear();
    for (auto& [id, notify] : visualNotifications)
        notify();
    for (auto& [id, notify] : textureNotifications)
        notify();
    for (auto& [id, notify] : materialNotifications)
        notify();

    // Framebuffers may borrow textures/cubemaps, so release them first.
    m_frameBufferInstances.removeAll();
    m_cubemapFrameBufferInstances.removeAll();
    m_materialInstances.removeAll();
    m_uboInstances.removeAll();
    m_visualInstances.removeAll();
    m_textureInstances.removeAll();
    m_cubemapInstances.removeAll();

    m_debugLinesProgram.destroy();
    m_computeOctahedralProgram.destroy();
    m_shadowProgram.destroy();
    m_postProcessingSSGIProgram.destroy();
    m_postProcessingProbeProgram.destroy();
    m_unicolorProgram.destroy();
    m_textureProgram.destroy();
    m_irradianceProgram.destroy();
    m_skyboxProgram.destroy();
    m_mainProgram.destroy();
    m_activeProgram = nullptr;
}

void Backend::setActiveProgram(ProgramType program)
{
    if (program == SKYBOX)
        m_activeProgram = &m_skyboxProgram;
    else if (program == PBR)
        m_activeProgram = &m_mainProgram;
    else if (program == TEXTURE)
        m_activeProgram = &m_textureProgram;
    else if (program == UNICOLOR)
        m_activeProgram = &m_unicolorProgram;
    else if (program == POST_PROCESSING_PROBE)
        m_activeProgram = &m_postProcessingProbeProgram;
    else if (program == FILTER_IRRADIANCE)
        m_activeProgram = &m_irradianceProgram;
    else if (program == SHADOW_DEPTH)
        m_activeProgram = &m_shadowProgram;
    else if (program == COMPUTE_OCTAHEDRAL)
        m_activeProgram = &m_computeOctahedralProgram;
    else if (program == POST_PROCESSING_SSGI)
        m_activeProgram = &m_postProcessingSSGIProgram;
    else
        GLX_CORE_ASSERT(false, "unknown asked program!");

    m_activeProgram->use();
}

renderID Backend::instanciateMesh(std::vector<Vertex>& vertices, std::vector<short unsigned int>& indices, std::function<void()> destroyCallback)
{
    if (!m_visualInstances.canAddInstance())
        return 0;

    renderID meshID = m_visualInstances.createResourceInstance();
    m_visualInstances.get(meshID)->init(vertices, indices);

    if (destroyCallback) {
        m_visualDestroyNotifications[meshID] = destroyCallback;
    }

    return meshID;
}

Sphere& Backend::getMeshBoundingVolume(renderID meshID)
{
    return m_visualInstances.get(meshID)->getBoundingVolume();
}

renderID Backend::instanciateMesh(ResourceHandle<Mesh> mesh, int surfaceIdx)
{
    renderID subMeshID = mesh.getResource().getVisualID(surfaceIdx);
    if (subMeshID != 0) {
        m_visualInstances.increaseCount(subMeshID);
        return subMeshID;
    }
    if (!m_visualInstances.canAddInstance())
        return 0;

    renderID visualID = m_visualInstances.createResourceInstance();
    mesh.getResource().setVisualID(surfaceIdx, visualID);
    const std::weak_ptr<int> backendLifetime = m_lifetimeToken;

    mesh.getResource().onLoaded([this, backendLifetime, mesh, visualID, surfaceIdx] {
        if (backendLifetime.expired() || mesh.getResource().getVisualID(surfaceIdx) != visualID)
            return;

        const auto& meshRes = mesh.getResource();
        auto* visualInstance = m_visualInstances.tryGet(visualID);
        if (visualInstance == nullptr)
            return;
        visualInstance->init(
            meshRes.getVertices(surfaceIdx),
            meshRes.getIndices(surfaceIdx));
    });

    m_visualDestroyNotifications[visualID] = [surfaceIdx, mesh, visualID] { mesh.getResource().notifyGpuInstanceDestroyed(surfaceIdx, visualID); };

    return visualID;
}

void Backend::clearMesh(renderID meshID)
{
    if (!m_visualInstances.tryRemove(meshID))
        return;

    auto it = m_visualDestroyNotifications.find(meshID);
    if (it != m_visualDestroyNotifications.end()) {
        it->second();
        m_visualDestroyNotifications.erase(meshID);
    }
}

renderID Backend::instantiateTexture(ResourceHandle<Image> image)
{
    renderID existingID = image.getResource().getTextureID();
    if (existingID != 0) {
        m_textureInstances.increaseCount(existingID);
        return existingID;
    }
    if (!m_textureInstances.canAddInstance())
        return 0;

    renderID textureID = m_textureInstances.createResourceInstance();
    image.getResource().setTextureID(textureID);
    const std::weak_ptr<int> backendLifetime = m_lifetimeToken;

    image.getResource().onLoaded([this, backendLifetime, image, textureID] {
        if (backendLifetime.expired() || image.getResource().getTextureID() != textureID)
            return;

        auto& imgRes = image.getResource();
        auto* texture = m_textureInstances.tryGet(textureID);
        if (texture == nullptr)
            return;
        texture->init(imgRes.getData(), imgRes.getWidth(), imgRes.getHeight(), imgRes.getNbChannels());
        imgRes.freeCpuData();
    });

    m_textureDestroyNotifications[textureID] = [image, textureID] { image.getResource().notifyGpuInstanceDestroyed(textureID); };

    return textureID;
}

renderID Backend::instantiateTexture(TextureFormat format, vec2 size)
{
    if (!m_textureInstances.canAddInstance())
        return 0;

    renderID textureID = m_textureInstances.createResourceInstance(format, static_cast<int>(size.x), static_cast<int>(size.y));
    checkOpenGLErrors("Instantiate texture");
    return textureID;
}

void Backend::clearTexture(renderID textureID)
{
    if (!m_textureInstances.tryRemove(textureID))
        return;

    auto it = m_textureDestroyNotifications.find(textureID);
    if (it != m_textureDestroyNotifications.end()) {
        it->second();
        m_textureDestroyNotifications.erase(textureID);
    }
}

void Backend::frameReset()
{
    Texture::resetStaticActivationInt();
    Texture::clearReservedActivationInts();
    
    auto texturesPtrs = m_textureInstances.getAll();
    for(auto texture : texturesPtrs){
        texture->resetActivationInt();
    }
}

renderID Backend::instanciateMaterial(ResourceHandle<Material> material)
{

    renderID existingID = material.getResource().getRenderID();
    if (existingID) {
        m_materialInstances.increaseCount(existingID);
        return existingID;
    }
    if (!m_materialInstances.canAddInstance())
        return 0;

    renderID materialID = m_materialInstances.createResourceInstance();
    material.getResource().setRenderID(materialID);
    m_materialDestroyNotifications[materialID] = [material, materialID] {
        material.getResource().notifyGpuInstanceDestroyed(materialID);
    };
    const std::weak_ptr<int> backendLifetime = m_lifetimeToken;

    material.getResource().onLoaded([this, backendLifetime, material, materialID] {
        if (backendLifetime.expired() || material.getResource().getRenderID() != materialID)
            return;

        auto matInstance = m_materialInstances.tryGet(materialID);
        if (matInstance == nullptr)
            return;
        const auto& matResource = material.getResource();

        auto setupTexture = [this, &matInstance, &matResource](TextureType type) {
            matInstance->useImage[type] = matResource.canUseImage(type);
            if (matInstance->useImage[type]) {
                matInstance->images[type] = instantiateTexture(matResource.getImage(type));
            }
        };

        setupTexture(ALBEDO);
        setupTexture(METALLIC);
        setupTexture(ROUGHNESS);
        setupTexture(NORMAL);
        setupTexture(AO);

        matInstance->albedo       = matResource.getAlbedo();
        matInstance->metallic     = matResource.getMetallic();
        matInstance->ambient      = matResource.getAmbient();
        matInstance->roughness    = matResource.getRoughness();
        matInstance->transparency = matResource.getTransparency();

        updateMaterial(materialID, material);
    });

    return materialID;
}

void Backend::updateMaterial(renderID materialID, ResourceHandle<Material> material)
{
    auto& matResource = material.getResource();
    m_materialInstances.get(materialID)->transparency = matResource.getTransparency();
    
    if (m_materialUpdateCallback) {
        m_materialUpdateCallback(materialID, matResource.isUsingTransparency());
    }
}

void Backend::clearMaterial(renderID materialID)
{
    const bool removed = m_materialInstances.tryRemove(materialID, [this](MaterialInstance& materialInstance) {
        for (size_t type = 0; type < TextureType::COUNT; ++type) {
            if (materialInstance.useImage[type])
                clearTexture(materialInstance.images[type]);
        }
    });

    if (!removed)
        return;

    auto notification = m_materialDestroyNotifications.find(materialID);
    if (notification != m_materialDestroyNotifications.end()) {
        notification->second();
        m_materialDestroyNotifications.erase(notification);
    }
}

void Backend::processCommands(const std::vector<RenderCommand>& commands)
{
    for (const auto& command : commands) {
        std::visit([this](auto&& cmd) {
            processCommand(cmd);
        },
            command);
        checkOpenGLErrors("Process command");
    }
}

renderID Backend::generateCube(float dimmension, bool inward, std::function<void()> destroyCallback)
{
    std::vector<Vertex> vertices;
    std::vector<short unsigned int> indices;

    vec3 half(dimmension / 2);
    vertices.resize(8);
    for (int i = 0; i < 8; ++i) {
        vertices[i].position = vec3(
            (i & 1 ? half.x : -half.x),
            (i & 2 ? half.y : -half.y),
            (i & 4 ? half.z : -half.z));
        vertices[i].normal   = vec3();
        vertices[i].texCoord = vec2();
    }

    std::array<unsigned int, 36> baseIndices = {
        // +X
        1, 5, 7, 1, 7, 3,
        // -X
        0, 2, 6, 0, 6, 4,
        // +Y
        2, 3, 7, 2, 7, 6,
        // -Y
        0, 4, 5, 0, 5, 1,
        // +Z
        4, 6, 7, 4, 7, 5,
        // -Z
        0, 1, 3, 0, 3, 2
    };

    indices.reserve(36);
    for (size_t i = 0; i < baseIndices.size(); i += 3) {
        if (inward) {
            indices.push_back(baseIndices[i]);
            indices.push_back(baseIndices[i + 1]);
            indices.push_back(baseIndices[i + 2]);
        } else {
            indices.push_back(baseIndices[i]);
            indices.push_back(baseIndices[i + 2]);
            indices.push_back(baseIndices[i + 1]);
        }
    }

    return instanciateMesh(vertices, indices, destroyCallback);
}

renderID Backend::generateQuad(vec2 dimmensions, std::function<void()> destroyCallback)
{
    vec2 half = dimmensions / 2.f;

    std::vector<Vertex> vertices;
    Vertex v1, v2, v3, v4;
    v1.position = vec3(-half.x, half.y, 0);
    v1.texCoord = vec2(0, 1);

    v2.position = vec3(half.x, half.y, 0);
    v2.texCoord = vec2(1, 1);

    v3.position = vec3(-half.x, -half.y, 0);
    v3.texCoord = vec2(0, 0);

    v4.position = vec3(half.x, -half.y, 0);
    v4.texCoord = vec2(1, 0);

    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);
    vertices.push_back(v4);

    std::vector<short unsigned int> indices;
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(1);

    indices.push_back(3);
    indices.push_back(1);
    indices.push_back(2);

    return instanciateMesh(vertices, indices, destroyCallback);
}

renderID Backend::generatePyramid(float baseSize, float height, std::function<void()> destroyCallback)
{
    std::vector<Vertex> vertices;
    std::vector<short unsigned int> indices;

    float half = baseSize / 2.0f;

    // top
    Vertex apex;
    apex.position = vec3(0, 0, height);
    apex.normal   = vec3(0, 0, 1);
    apex.texCoord = vec2(0.5f, 0.5f);
    vertices.push_back(apex); // index 0

    // base
    Vertex base1, base2, base3, base4;
    base1.position = vec3(-half, -half, 0);
    base1.normal   = vec3(0, 0, -1);
    base1.texCoord = vec2(0, 0);
    vertices.push_back(base1); // index 1

    base2.position = vec3(half, -half, 0);
    base2.normal   = vec3(0, 0, -1);
    base2.texCoord = vec2(1, 0);
    vertices.push_back(base2); // index 2

    base3.position = vec3(half, half, 0);
    base3.normal   = vec3(0, 0, -1);
    base3.texCoord = vec2(1, 1);
    vertices.push_back(base3); // index 3

    base4.position = vec3(-half, half, 0);
    base4.normal   = vec3(0, 0, -1);
    base4.texCoord = vec2(0, 1);
    vertices.push_back(base4); // index 4

    // lateral faces triangles (apex to each base edge)
    // Front face (towards -Y)
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    // Right face (towards +X)
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);
    // Back face (towards +Y)
    indices.push_back(0);
    indices.push_back(3);
    indices.push_back(4);
    // Left face (towards -X)
    indices.push_back(0);
    indices.push_back(4);
    indices.push_back(1);

    // Base (two triangles)
    indices.push_back(1);
    indices.push_back(4);
    indices.push_back(3);
    indices.push_back(1);
    indices.push_back(3);
    indices.push_back(2);

    return instanciateMesh(vertices, indices, destroyCallback);
}

void Backend::clearCubemap(renderID cubemapID)
{
    if (m_cubemapInstances.tryRemove(cubemapID))
        m_cubemapUploadLifetimes.erase(cubemapID);
}

renderID Backend::instanciateFrameBuffer(unsigned int width, unsigned int height, FramebufferTextureFormat format, unsigned int colorCount, unsigned int depthLayerCount)
{
    if (!m_frameBufferInstances.canAddInstance())
        return 0;

    renderID frameBufferID = m_frameBufferInstances.createResourceInstance(width, height, format, colorCount, depthLayerCount);
    m_frameBufferInstances.get(frameBufferID)->unbind();
    checkOpenGLErrors("Instantiate frameBuffer");
    return frameBufferID;
}

renderID Backend::instantiateCubemapFrameBuffer(unsigned int resolution, unsigned int colorCount)
{
    if (!m_cubemapFrameBufferInstances.canAddInstance())
        return 0;

    renderID frameBufferID = m_cubemapFrameBufferInstances.createResourceInstance(resolution, colorCount);
    m_cubemapFrameBufferInstances.get(frameBufferID)->unbind();
    checkOpenGLErrors("Instantiate frameBuffer");
    return frameBufferID;
}

void Backend::clearFrameBuffer(renderID frameBufferID)
{
    m_frameBufferInstances.tryRemove(frameBufferID);
    checkOpenGLErrors("Clear frameBuffer");
}

void Backend::resizeFrameBuffer(renderID frameBufferID, unsigned int width, unsigned int height, unsigned int depthLayerCount)
{
    auto* framebuffer = m_frameBufferInstances.tryGet(frameBufferID);
    if (framebuffer == nullptr) {
        GLX_CORE_ERROR("Trying to resize an unknown framebuffer: {0}", frameBufferID);
        return;
    }
    framebuffer->resize(width, height, depthLayerCount);
}

void Backend::resizeCubemapFrameBuffer(renderID frameBufferID, unsigned int size)
{
    auto* framebuffer = m_cubemapFrameBufferInstances.tryGet(frameBufferID);
    if (framebuffer == nullptr) {
        GLX_CORE_ERROR("Trying to resize an unknown cubemap framebuffer: {0}", frameBufferID);
        return;
    }
    framebuffer->resize(size);
}

FramebufferTextureFormat Backend::getFramebufferFormat(renderID id)
{
    auto* framebuffer = m_frameBufferInstances.tryGet(id);
    if (framebuffer == nullptr) {
        GLX_CORE_ERROR("Trying to inspect an unknown framebuffer: {0}", id);
        return FramebufferTextureFormat::None;
    }
    return framebuffer->getFormat();
}

unsigned int Backend::getFrameBufferTextureID(renderID frameBufferID)
{
    auto* framebuffer = m_frameBufferInstances.tryGet(frameBufferID);
    if (framebuffer == nullptr) {
        GLX_CORE_ERROR("Trying to inspect an unknown framebuffer: {0}", frameBufferID);
        return 0;
    }
    return framebuffer->getColorTextureID();
}

unsigned int Backend::getFrameBufferDepthTextureID(renderID frameBufferID)
{
    auto* framebuffer = m_frameBufferInstances.tryGet(frameBufferID);
    if (framebuffer == nullptr) {
        GLX_CORE_ERROR("Trying to inspect an unknown framebuffer: {0}", frameBufferID);
        return 0;
    }
    return framebuffer->getDepthTextureID();
}

void Backend::setCullMode(renderID visualInstanceID, CullMode mode)
{
    m_visualInstances.get(visualInstanceID)->setCullMode(mode);
}

void Backend::initDebugCallback()
{
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback([](GLenum source, GLenum type, GLuint id,
                               GLenum severity, GLsizei length,
                               const GLchar* message, const void* userParam) {
        (void)source;
        (void)type;
        (void)id;
        (void)length;
        (void)userParam;
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
            return;

        const char* severityChr;
        switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            severityChr = "High";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            severityChr = "Medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            severityChr = "Low";
            break;
        default:
            severityChr = "Unknown";
            break;
        }
        GLX_CORE_WARN("GL DEBUG (severity={0}): {1}", severityChr, message);

        GLX_CORE_ASSERT(severity != GL_DEBUG_SEVERITY_HIGH, "GL DEBUG SEVERITY HIGH encountered");
    },
        nullptr);
}

renderID Backend::instanciateCubemap(std::array<ResourceHandle<Image>, 6> faces)
{
    renderID cubemapID = instanciateCubemap();
    if (cubemapID == 0)
        return 0;
    auto uploadLifetime = std::make_shared<int>(0);
    m_cubemapUploadLifetimes[cubemapID] = uploadLifetime;
    const std::weak_ptr<int> weakUploadLifetime = uploadLifetime;
    const std::weak_ptr<int> backendLifetime = m_lifetimeToken;

    for (int i = 0; i < 6; i++) {
        faces[i].getResource().onLoaded([this, backendLifetime, weakUploadLifetime, faces, cubemapID, i] {
            if (backendLifetime.expired() || weakUploadLifetime.expired())
                return;

            auto* cubemapInstance = m_cubemapInstances.tryGet(cubemapID);
            if (cubemapInstance == nullptr)
                return;
            int w = faces[0].getResource().getWidth();
            int h = faces[0].getResource().getHeight();
            cubemapInstance->resize(w);

            auto& faceResource = faces[i].getResource();
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapInstance->getId());

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, w, w, 0, GL_RGB, GL_UNSIGNED_BYTE, faceResource.getData());
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        });
    }

    return cubemapID;
}

renderID Backend::instanciateCubemap(int resolution)
{
    if (!m_cubemapInstances.canAddInstance())
        return 0;

    renderID cubemapID = m_cubemapInstances.createResourceInstance();
    m_cubemapInstances.get(cubemapID)->resize(resolution);
    return cubemapID;
}

void Backend::processCommand(const ClearCommand& clearCommand)
{
    auto& clearColor = clearCommand.color;
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Backend::processCommand(const DepthMaskCommand& command)
{
    if (command.state)
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);
}

void Backend::processCommand(const SetViewCommand& setViewCommand)
{
    m_mainProgram.use();
    m_mainProgram.updateViewMatrix(setViewCommand.view);

    m_textureProgram.use();
    m_textureProgram.updateViewMatrix(setViewCommand.view);

    m_unicolorProgram.use();
    m_unicolorProgram.updateViewMatrix(setViewCommand.view);

    m_skyboxProgram.use();
    m_skyboxProgram.updateViewMatrix(setViewCommand.view);

    m_irradianceProgram.use();
    m_irradianceProgram.updateViewMatrix(setViewCommand.view);

    m_postProcessingProbeProgram.use();
    m_postProcessingProbeProgram.updateViewMatrix(setViewCommand.view);
    m_postProcessingProbeProgram.updateInverseViewMatrix(inverse(setViewCommand.view));

    m_postProcessingSSGIProgram.use();
    m_postProcessingSSGIProgram.updateViewMatrix(setViewCommand.view);
    m_postProcessingSSGIProgram.updateInverseViewMatrix(inverse(setViewCommand.view));

    m_debugLinesProgram.use();
    m_debugLinesProgram.updateViewMatrix(setViewCommand.view);

    m_activeProgram->use();
}
void Backend::setProjectionMatrix(const mat4& projectionMatrix)
{
    // TODO: bug prone
    m_mainProgram.use();
    m_mainProgram.updateProjectionMatrix(projectionMatrix);

    m_textureProgram.use();
    m_textureProgram.updateProjectionMatrix(projectionMatrix);

    m_unicolorProgram.use();
    m_unicolorProgram.updateProjectionMatrix(projectionMatrix);

    m_skyboxProgram.use();
    m_skyboxProgram.updateProjectionMatrix(projectionMatrix);

    m_irradianceProgram.use();
    m_irradianceProgram.updateProjectionMatrix(projectionMatrix);

    m_postProcessingProbeProgram.use();
    m_postProcessingProbeProgram.updateProjectionMatrix(projectionMatrix);
    m_postProcessingProbeProgram.updateInverseProjectionMatrix(inverse(projectionMatrix));

    m_postProcessingSSGIProgram.use();
    m_postProcessingSSGIProgram.updateProjectionMatrix(projectionMatrix);
    m_postProcessingSSGIProgram.updateInverseProjectionMatrix(inverse(projectionMatrix));

    m_debugLinesProgram.use();
    m_debugLinesProgram.updateProjectionMatrix(projectionMatrix);

    m_activeProgram->use();
}
void Backend::processCommand(const SetProjectionCommand& command)
{
    setProjectionMatrix(command.projection);
}

void Backend::processCommand(const SetActiveProgramCommand& command)
{
    setActiveProgram(command.program);
}

void Backend::processCommand(const DrawCommand& command)
{
    auto& modelMatrix = command.model;
    m_activeProgram->updateModelMatrix(modelMatrix);
    m_visualInstances.get(command.instanceId)->draw();
}

void Backend::processCommand(const RawDrawCommand& command)
{
    m_visualInstances.get(command.instanceID)->draw();
}

void Backend::processCommand(const UseTextureCommand& command)
{
    auto uniLoc = glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName.c_str());
    Texture* texture = m_textureInstances.get(command.instanceID);
    texture->activate(uniLoc);
    if(command.important)
        texture->reserveActivationInt();
    checkOpenGLErrors("Bind texture");
}

void Backend::processCommand(const UseCubemapCommand& command)
{
    auto uniLoc   = glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName.c_str());
    auto& cubemap = *m_cubemapInstances.get(command.instanceID);
    cubemap.activate(uniLoc);
    checkOpenGLErrors("Bind cubemap");
}

void Backend::processCommand(const AttachTextureToFramebufferCommand& command)
{
    auto& framebuffer = *m_frameBufferInstances.get(command.framebufferID);
    auto& texture     = *m_textureInstances.get(command.textureID);
    if (command.attachmentIdx < 0)
        framebuffer.attachDepthTexture(texture);
    else
        framebuffer.attachColorTexture(texture, command.attachmentIdx);

    checkOpenGLErrors("Attach texture to framebuffer");
}

void Backend::processCommand(const AttachCubemapToFramebufferCommand& command)
{
    // TODO: Beware of memory handling !!!
    if (command.colorIdx < 0)
        m_cubemapFrameBufferInstances.get(command.framebufferID)->attachDepthCubemap(*m_cubemapInstances.get(command.cubemapID));
    else
        m_cubemapFrameBufferInstances.get(command.framebufferID)->attachColorCubemap(*m_cubemapInstances.get(command.cubemapID), command.colorIdx);
}

void Backend::processCommand(const BindMaterialCommand& command)
{
    if (m_activeProgram->type() != ProgramType::PBR) {
        GLX_CORE_ERROR("PBR Program not active, activating it");
        setActiveProgram(ProgramType::PBR);
    }

    MaterialInstance& material = *m_materialInstances.get(command.materialRenderID);
    std::array<Texture*, TextureType::COUNT> materialTextures {};
    auto addTexture = [&material, &materialTextures, this](TextureType type) {
        if (material.useImage[type]) {
            materialTextures[type] = m_textureInstances.get(material.images[type]);
        }
    };
    addTexture(ALBEDO);
    addTexture(NORMAL);
    addTexture(METALLIC);
    addTexture(ROUGHNESS);
    addTexture(AO);

    static_cast<ProgramPBR*>(m_activeProgram)->updateMaterial(material, materialTextures);
    checkOpenGLErrors("Binding material");
}

void Backend::processCommand(const BindFrameBufferCommand& command)
{
    if (command.bind)
        if (command.cubemapFaceIdx >= 0)
            m_cubemapFrameBufferInstances.get(command.frameBufferID)->bind(command.cubemapFaceIdx);
        else
            m_frameBufferInstances.get(command.frameBufferID)->bind(command.depthLayerIdx);
    else {
        if (command.cubemapFaceIdx >= 0)
            m_cubemapFrameBufferInstances.get(command.frameBufferID)->unbind();
        else
            m_frameBufferInstances.get(command.frameBufferID)->unbind();
    }

    checkOpenGLErrors("Binding framebuffer");
}

void Backend::processCommand(const SetUniformCommand& command)
{
    if (command.type == SetValueTypes::BOOL) {
        glUniform1i(glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName.c_str()), command.valueBool ? GL_TRUE : GL_FALSE);
    } else if (command.type == SetValueTypes::FLOAT) {
        glUniform1f(glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName.c_str()), command.valueFloat);
    } else if (command.type == SetValueTypes::INT) {
        glUniform1i(glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName.c_str()), command.valueInt);
    } else if (command.type == SetValueTypes::VEC3) {
        glUniform3f(glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName.c_str()),
            command.valueVec3.x, command.valueVec3.y, command.valueVec3.z);
    } else if (command.type == SetValueTypes::IVEC3) {
        glUniform3i(glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName.c_str()),
            command.valueIVec3.x, command.valueIVec3.y, command.valueIVec3.z);
    } else if (command.type == SetValueTypes::VEC2) {
        glUniform2f(glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName.c_str()),
            command.valueVec2.x, command.valueVec2.y);
    } else if (command.type == SetValueTypes::MAT4) {
        glUniformMatrix4fv(glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName.c_str()), 1, GL_FALSE, &command.matrixValue[0][0]);
    }

    checkOpenGLErrors("Set uniform");
}

void Backend::processCommand(const SetViewportCommand& command)
{
    glViewport((int)command.position.x, (int)command.position.y, (int)command.size.x, (int)command.size.y);
}

void Backend::processCommand(const UpdateTextureCommand& command)
{
    if (command.newFormat == TextureFormat::NONE)
        m_textureInstances.get(command.targetID)->resize(command.width, command.height);
    else
        m_textureInstances.get(command.targetID)->setFormat(command.newFormat);

    checkOpenGLErrors("Update texture");
}

void Backend::processCommand(const UpdateCubemapCommand& command)
{
    m_cubemapInstances.get(command.targetID)->resize(command.resolution);
    checkOpenGLErrors("Update cubemap");
}

void Backend::processCommand(const SetFramebufferAsTextureUniformCommand& command)
{
    auto uniLoc       = glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName.c_str());
    if(!command.aboutCubemap){
        auto& framebuffer = *m_frameBufferInstances.get(command.framebufferID);
        framebuffer.setAsTextureUniform(uniLoc, command.textureIdx);
    } else {
        auto& cubemapFB = *m_cubemapFrameBufferInstances.get(command.framebufferID);
        cubemapFB.setAsCubemapUniform(uniLoc, command.textureIdx);
    }
    checkOpenGLErrors("Bind framebuffer texture as uniform");
}

void Backend::processCommand(const UpdateUBOCommand& command)
{
    m_uboInstances.get(command.uboID)->update(command.data.data(), command.data.size());
}

void Backend::processCommand(const BindUBOCommand& command)
{
    m_uboInstances.get(command.uboID)->bind(command.idx);
}

void Backend::processCommand(const DebugMsgCommand& command)
{
    GLX_CORE_TRACE(command.msg);
}

void Backend::processCommand(const DrawDebugLineCommand& command)
{
    m_debugLines.addLine(command.start, command.end, vec3(0, 1, 0));
}

void Backend::processCommand(const SaveFrameBufferCommand& command)
{
    m_frameBufferInstances.get(command.frameBufferID)->savePPM(command.path);
}

void Backend::debugDraw()
{
    m_debugLinesProgram.use();
    m_debugLines.draw();
}

} // namespace Galaxy
