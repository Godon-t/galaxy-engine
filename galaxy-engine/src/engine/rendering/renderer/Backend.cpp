#include "Backend.hpp"

#include "Helper.hpp"
#include "Log.hpp"
#include "gl_headers.hpp"
#include "pch.hpp"
#include "rendering/OpenglHelper.hpp"
#include "rendering/renderer/refactor/RenderGraphExecution.hpp"
#include "rendering/renderer/refactor/RenderGraph.hpp"

namespace Galaxy {
Backend::Backend(size_t maxSize)
    : m_programInstances(maxSize)
    , m_visualInstances(maxSize)
    , m_textureInstances(maxSize * 2)
    , m_materialInstances(maxSize)
    , m_cubemapInstances(maxSize)
    , m_frameBufferInstances(maxSize)
    , m_cubemapFrameBufferInstances(maxSize)
    , m_uboInstances(maxSize)
{
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

    checkOpenGLErrors("Renderer constructor");
}

Backend::~Backend()
{
    destroy();
}

BufferHandle Backend::instantiateUBO(unsigned int dataSize)
{
    if (!m_uboInstances.canCreate())
        return {};

    BufferHandle uboID = m_uboInstances.create();

    m_uboInstances.get(uboID)->init(dataSize);

    return uboID;
}

ProgramHandle Backend::loadShader(std::string path)
{
    if (!m_programInstances.canCreate())
        return {};

    const ProgramHandle handle = m_programInstances.create(path);
    Program* program = m_programInstances.get(handle);
    if (program == nullptr || !program->isLinked()) {
        (void)m_programInstances.release(handle);
        return {};
    }
    return handle;
}

void Backend::clearProgram(ProgramHandle program)
{
    if (m_activeProgram == program)
        m_activeProgram = {};

    (void)m_programInstances.release(program);
}

void Backend::destroy()
{
    // Pending CPU-resource callbacks use a weak copy of this token.
    m_lifetimeToken.reset();
    m_cubemapUploadLifetimes.clear();

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
    m_frameBufferInstances.clear();
    m_cubemapFrameBufferInstances.clear();
    m_framebufferAttachments.clear();
    m_cubemapFramebufferAttachments.clear();
    m_materialInstances.clear();
    m_uboInstances.clear();
    m_visualInstances.clear();
    m_textureInstances.clear();
    m_cubemapInstances.clear();

    m_activeProgram = {};
    m_programInstances.clear();
}

void Backend::setActiveProgram(ProgramHandle program)
{
    Program* instance = m_programInstances.tryGet(program);
    if (instance == nullptr) {
        GLX_CORE_ERROR("Cannot activate an invalid program handle (slot={0}, generation={1})",
            program.index(), program.generation());
        return;
    }

    m_activeProgram = program;
    instance->use();
}

int Backend::getUniformLocation(ProgramHandle program, const std::string& uniformName) const
{
    const Program* instance = m_programInstances.tryGet(program);
    if (instance == nullptr) {
        GLX_CORE_ERROR("Cannot inspect an invalid program handle");
        return -1;
    }
    return instance->getUniformLocation(uniformName);
}

Program* Backend::getActiveProgram()
{
    return m_programInstances.tryGet(m_activeProgram);
}

const Program* Backend::getActiveProgram() const
{
    return m_programInstances.tryGet(m_activeProgram);
}


void applyState(const RenderState& state){
    glDepthMask(state.depthWrite ? GL_TRUE : GL_FALSE);
    if(state.clear)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if(state.depthTest)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    // state.blend
    if(state.blend == BlendMode::Alpha){
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else if (state.blend == BlendMode::Disabled)
        glDisable(GL_BLEND);

}


void Backend::execute(RenderGraphExecution& execution){
    const auto& compiledGraph = execution.graph.compilation;

    GraphFramebufferHandle currentFramebuffer;

    auto unbind = [this](const GraphFramebufferHandle& framebuffer) {
        std::visit([this](const auto& typedHandle) {
            if (typedHandle)
                unbindFramebuffer(typedHandle);
        }, framebuffer);
    };
    auto resize = [this](const GraphFramebufferHandle& framebuffer, const ivec2& viewportSize) {
        std::visit([this, &viewportSize](const auto& typedHandle) {
            using Handle = std::decay_t<decltype(typedHandle)>;
            if constexpr (std::is_same_v<Handle, FramebufferHandle>) {
                resizeFrameBuffer(typedHandle, viewportSize.x, viewportSize.y);
            } else {
                if (viewportSize.x != viewportSize.y)
                    GLX_CORE_WARN("A cubemap framebuffer requires a square viewport; using width {0}", viewportSize.x);
                resizeCubemapFrameBuffer(typedHandle, viewportSize.x);
            }
        }, framebuffer);
    };
    auto bind = [this](const GraphFramebufferHandle& framebuffer, int targetLayer) {
        std::visit([this, targetLayer](const auto& typedHandle) {
            bindFramebuffer(typedHandle, targetLayer);
        }, framebuffer);
    };

    for (const auto& entry : execution.invocationOrder) {
        const auto& pass = execution.graph.getRenderPass(entry.pass);
        auto& invocation = execution.getPass(entry.pass).invocations[entry.invocationIndex];
        const auto& targetFramebuffer = invocation.targetFramebuffer.value_or(pass.targetFramebufferHandle);

        setActiveProgram(pass.program);
        const auto& renderState = compiledGraph.getPassDescription(entry.pass).state;

        const bool framebufferChanged = currentFramebuffer != targetFramebuffer;
        if(RenderGraph::hasFramebuffer(currentFramebuffer) && framebufferChanged)
            unbind(currentFramebuffer);

        resize(targetFramebuffer, invocation.viewportSize);
        bind(targetFramebuffer, invocation.targetLayer);
        currentFramebuffer = targetFramebuffer;

        glClearColor(
            invocation.clearColor.r,
            invocation.clearColor.g,
            invocation.clearColor.b,
            invocation.clearColor.a);
        glViewport(
            invocation.viewportPosition.x,
            invocation.viewportPosition.y,
            invocation.viewportSize.x,
            invocation.viewportSize.y);
        applyState(renderState);

        Program& currentProg = *getActiveProgram();
        // GLX-TODO: no diff between viewParameters and parameters
        for(auto& setParameter : invocation.viewParameters)
            applyShaderParameter(currentProg, setParameter.first, setParameter.second);

        for(auto& setParameter : invocation.parameters)
            applyShaderParameter(currentProg, setParameter.first, setParameter.second);

        for (const auto& update : invocation.updates)
            processCommand(update);

        for(auto& binding : invocation.uniformBindings){
            auto* ubo = m_uboInstances.get(binding.buffer);
            if(ubo != nullptr)
                ubo->bind(binding.bindingPoint);
        }

        for(size_t i = 0; i<pass.inputTextures.size(); i++)
        {
            std::visit([this, &pass, i](const auto& textureHandle) {
                using Handle = std::decay_t<decltype(textureHandle)>;
                if constexpr (std::is_same_v<Handle, TextureHandle>) {
                    Texture* texture = m_textureInstances.tryGet(textureHandle);
                    if (texture == nullptr) {
                        GLX_CORE_ERROR("Cannot bind an invalid texture handle (slot={0}, generation={1})",
                            textureHandle.index(), textureHandle.generation());
                        return;
                    }
                    texture->activate(static_cast<int>(pass.inputTexturesLocations[i]));
                } else {
                    Cubemap* cubemap = m_cubemapInstances.tryGet(textureHandle);
                    if (cubemap == nullptr) {
                        GLX_CORE_ERROR("Cannot bind an invalid cubemap handle (slot={0}, generation={1})",
                            textureHandle.index(), textureHandle.generation());
                        return;
                    }
                    cubemap->activate(static_cast<int>(pass.inputTexturesLocations[i]));
                }
            }, pass.inputTextures[i]);
        }

        for (const auto& binding : invocation.textureBindings) {
            const int location = currentProg.getUniformLocation(binding.samplerName);
            std::visit([this, location](const auto& textureHandle) {
                using Handle = std::decay_t<decltype(textureHandle)>;
                if constexpr (std::is_same_v<Handle, TextureHandle>)
                    m_textureInstances.get(textureHandle)->activate(location);
                else
                    m_cubemapInstances.get(textureHandle)->activate(location);
            }, binding.texture);
        }

        for (const RenderItem& item : invocation.items) {
            if (renderState.bindMaterials) {
                if (item.material)
                    bindMaterial(*item.material);
                else
                    bindPBRMaterial(MaterialInstance {});
            }
            draw(item.geometry, item.transform.getGlobalModelMatrix());
        }
    }
    if(RenderGraph::hasFramebuffer(currentFramebuffer))
        unbind(currentFramebuffer);
}

void Backend::bindFramebuffer(FramebufferHandle fb, int targetLayer){
    // GLX-TODO: redudancy in framebuffer accession
    auto* framebufferInstance = m_frameBufferInstances.tryGet(fb);
    if (framebufferInstance == nullptr) {
        GLX_CORE_ERROR("Cannot attach texture to inexistent framebuffer");
        return;
    }

    framebufferInstance->bind(targetLayer < 0 ? 0 : targetLayer);
}

void Backend::bindFramebuffer(CubemapFramebufferHandle fb, int targetFace)
{
    auto* framebufferInstance = m_cubemapFrameBufferInstances.tryGet(fb);
    if (framebufferInstance == nullptr) {
        GLX_CORE_ERROR("Cannot bind an unknown cubemap framebuffer");
        return;
    }

    framebufferInstance->bind(targetFace < 0 ? 0 : targetFace);
}

void Backend::unbindFramebuffer(FramebufferHandle fb){
    auto* framebufferInstance = m_frameBufferInstances.tryGet(fb);
    if (framebufferInstance == nullptr) {
        GLX_CORE_ERROR("Cannot attach texture to inexistent framebuffer");
        return;
    }

    framebufferInstance->unbind();
}

void Backend::unbindFramebuffer(CubemapFramebufferHandle fb)
{
    auto* framebufferInstance = m_cubemapFrameBufferInstances.tryGet(fb);
    if (framebufferInstance == nullptr) {
        GLX_CORE_ERROR("Cannot unbind an unknown cubemap framebuffer");
        return;
    }

    framebufferInstance->unbind();
}



bool Backend::attachColorTextureToFramebuffer(TextureHandle texture, FramebufferHandle framebuffer, int colorattachmentIdx){
    auto* framebufferInstance = m_frameBufferInstances.tryGet(framebuffer);
    if (framebufferInstance == nullptr) {
        GLX_CORE_ERROR("Cannot attach texture to inexistent framebuffer");
        return false;
    }

    auto* textureInstance = m_textureInstances.tryGet(texture);
    if (textureInstance == nullptr) {
        GLX_CORE_ERROR("Cannot attach inexistent texture to framebuffer");
        return false;
    }

    return framebufferInstance->attachColorTexture(*textureInstance, colorattachmentIdx);
}

bool Backend::attachDepthTextureToFramebuffer(TextureHandle texture, FramebufferHandle framebuffer){
    auto* framebufferInstance = m_frameBufferInstances.tryGet(framebuffer);
    if (framebufferInstance == nullptr) {
        GLX_CORE_ERROR("Cannot attach texture to inexistent framebuffer");
        return false;
    }

    auto* textureInstance = m_textureInstances.tryGet(texture);
    if (textureInstance == nullptr) {
        GLX_CORE_ERROR("Cannot attach inexistent texture to framebuffer");
        return false;
    }

    return framebufferInstance->attachDepthTexture(*textureInstance);
}




GeometryHandle Backend::instanciateMesh(std::vector<Vertex>& vertices, std::vector<short unsigned int>& indices, std::function<void()> destroyCallback)
{
    if (!m_visualInstances.canCreate())
        return {};

    GeometryHandle meshID = m_visualInstances.create();
    m_visualInstances.get(meshID)->init(vertices, indices);

    if (destroyCallback) {
        m_visualDestroyNotifications[meshID] = destroyCallback;
    }

    return meshID;
}

geometry::Sphere Backend::getMeshBoundingVolume(GeometryHandle meshID)
{
    auto* geometry = m_visualInstances.tryGet(meshID);
    if (geometry == nullptr) {
        GLX_CORE_ERROR("Cannot inspect an invalid geometry handle");
        return geometry::Sphere { 0.0f, vec3(0.0f) };
    }
    return geometry->getBoundingVolume();
}

GeometryHandle Backend::instanciateMesh(ResourceHandle<Mesh> mesh, int surfaceIdx)
{
    GeometryHandle subMeshID = mesh.getResource().getGpuGeometryHandle(surfaceIdx);
    if (subMeshID) {
        if (m_visualInstances.contains(subMeshID)) {
            m_visualInstances.retain(subMeshID);
            return subMeshID;
        }
        GLX_WARN("Unvalid GPU handle on mesh {0} for surface index {1}", mesh.getResource().getPath(), surfaceIdx);
        mesh.getResource().notifyGpuInstanceDestroyed(surfaceIdx, subMeshID);
    }
    if (!m_visualInstances.canCreate())
        return {};

    GeometryHandle visualID = m_visualInstances.create();
    mesh.getResource().setGpuGeometryHandle(surfaceIdx, visualID);
    const std::weak_ptr<int> backendLifetime = m_lifetimeToken;

    mesh.getResource().onLoaded([this, backendLifetime, mesh, visualID, surfaceIdx] {
        if (backendLifetime.expired() || mesh.getResource().getGpuGeometryHandle(surfaceIdx) != visualID)
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

void Backend::clearMesh(GeometryHandle meshID)
{
    if (!m_visualInstances.release(meshID))
        return;

    auto it = m_visualDestroyNotifications.find(meshID);
    if (it != m_visualDestroyNotifications.end()) {
        it->second();
        m_visualDestroyNotifications.erase(meshID);
    }
}

TextureHandle Backend::instantiateTexture(ResourceHandle<Image> image)
{
    TextureHandle existingID = image.getResource().getGpuTextureHandle();
    if (existingID) {
        m_textureInstances.retain(existingID);
        return existingID;
    }
    if (!m_textureInstances.canCreate())
        return {};

    TextureHandle textureID = m_textureInstances.create();
    image.getResource().setGpuTextureHandle(textureID);
    const std::weak_ptr<int> backendLifetime = m_lifetimeToken;

    image.getResource().onLoaded([this, backendLifetime, image, textureID] {
        if (backendLifetime.expired() || image.getResource().getGpuTextureHandle() != textureID)
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

void Backend::setTextureData(TextureHandle textureHandle,  ResourceHandle<Image> image)
{
    auto* texture  = m_textureInstances.get(textureHandle);
    if(texture == nullptr){
        GLX_ERROR("texture not found when loading data");
        return;
    }
    

    image.getResource().setGpuTextureHandle(textureHandle);
    const std::weak_ptr<int> backendLifetime = m_lifetimeToken;

    image.getResource().onLoaded([this, backendLifetime, image, textureHandle] {
        if (backendLifetime.expired() || image.getResource().getGpuTextureHandle() != textureHandle)
            return;

        auto& imgRes = image.getResource();
        auto* texture = m_textureInstances.tryGet(textureHandle);
        if (texture == nullptr)
            return;
        texture->init(imgRes.getData(), imgRes.getWidth(), imgRes.getHeight(), imgRes.getNbChannels());
        imgRes.freeCpuData();
    });

    m_textureDestroyNotifications[textureHandle] = [image, textureHandle] { image.getResource().notifyGpuInstanceDestroyed(textureHandle); };
}

void Backend::setTextureWrap(TextureHandle textureHandle, TextureWrap wrapS, TextureWrap wrapT)
{
    auto* texture  = m_textureInstances.get(textureHandle);
    if(texture == nullptr){
        GLX_ERROR("texture not found when changing wrap!");
        return;
    } else {
        texture->setWrap(wrapS, wrapT);
    }
}

void Backend::setTextureWrap(CubemapHandle cubemapHandle, TextureWrap wrapS, TextureWrap wrapT, TextureWrap wrapR)
{
    auto* cubemap = m_cubemapInstances.tryGet(cubemapHandle);
    if (cubemap == nullptr) {
        GLX_ERROR("Cubemap not found when changing wrap");
        return;
    }

    cubemap->setWrap(wrapS, wrapT, wrapR);
}

TextureHandle Backend::instantiateTexture(TextureFormat format, vec2 size, TextureFiltering filter, size_t layerCount)
{
    if (!m_textureInstances.canCreate())
        return {};

    TextureHandle textureID = m_textureInstances.create(format, static_cast<int>(size.x), static_cast<int>(size.y), filter, static_cast<int>(layerCount));
    checkOpenGLErrors("Instantiate texture");
    return textureID;
}

void Backend::clearTexture(TextureHandle textureID)
{
    if (!m_textureInstances.release(textureID))
        return;

    auto it = m_textureDestroyNotifications.find(textureID);
    if (it != m_textureDestroyNotifications.end()) {
        it->second();
        m_textureDestroyNotifications.erase(textureID);
    }
}

void Backend::frameReset()
{
    m_drawCount = 0;
    Texture::resetStaticActivationInt();
    Texture::clearReservedActivationInts();
    
    auto texturesPtrs = m_textureInstances.getAll();
    for(auto texture : texturesPtrs){
        texture->resetActivationInt();
    }

    for (Cubemap* cubemap : m_cubemapInstances.getAll())
        cubemap->resetActivationInt();
}

MaterialHandle Backend::instantiateMaterial(ResourceHandle<BaseMaterial> material)
{
    MaterialHandle existingID = material.getResource().getGpuMaterialHandle();
    if (existingID) {
        m_materialInstances.retain(existingID);
        return existingID;
    }

    if (!m_materialInstances.canCreate())
        return {};

    MaterialHandle materialID = m_materialInstances.create();
    material.getResource().setGpuMaterialHandle(materialID);
    m_materialDestroyNotifications[materialID] = [material, materialID] {
        material.getResource().notifyGpuInstanceDestroyed(materialID);
    };
    const std::weak_ptr<int> backendLifetime = m_lifetimeToken;
    material.getResource().onLoaded([this, backendLifetime, material, materialID] {
        if (backendLifetime.expired() || material.getResource().getGpuMaterialHandle() != materialID)
            return;
        updateMaterial(materialID, material);
    });
    return materialID;
}

void Backend::updateMaterial(MaterialHandle materialID, ResourceHandle<BaseMaterial> material)
{
    auto& resource = material.getResource();
    MaterialInstance updated;
    updated.type = resource.getType();
    updated.transparency = resource.getTransparency();

    switch (updated.type) {
    case MaterialType::PBR: {
        auto& pbr = static_cast<PBRMaterial&>(resource);
        updated.albedo = pbr.getAlbedo();
        updated.metallic = pbr.getMetallic();
        updated.roughness = pbr.getRoughness();
        updated.ambient = pbr.getAmbient();
        for (int type = 0; type < TextureType::COUNT; ++type) {
            auto textureType = static_cast<TextureType>(type);
            updated.useImage[type] = pbr.canUseImage(textureType);
            if (updated.useImage[type])
                updated.images[type] = instantiateTexture(pbr.getImage(textureType));
        }
        break;
    }
    case MaterialType::Texture: {
        auto& textured = static_cast<TextureMaterial&>(resource);
        updated.hasTexture = textured.hasTexture();
        if (updated.hasTexture) {
            const auto& source = textured.getTexture();
            if (const auto* image = std::get_if<ResourceHandle<Image>>(&source)) {
                updated.texture = instantiateTexture(*image);
            } else if (const auto* texture = std::get_if<TextureHandle>(&source)) {
                m_textureInstances.retain(*texture);
                updated.texture = *texture;
            } else {
                const auto cubemap = std::get<CubemapHandle>(source);
                m_cubemapInstances.retain(cubemap);
                updated.texture = cubemap;
            }
        }
        break;
    }
    }

    auto& instance = *m_materialInstances.get(materialID);
    // Acquire the new references before releasing the previous material data.
    releaseMaterialTextures(instance);
    instance = std::move(updated);

    if (m_materialUpdateCallback) {
        MaterialRenderInfo info;
        info.type = instance.type;
        info.isTransparent = resource.isUsingTransparency();
        info.isCubemap = instance.hasTexture && std::holds_alternative<CubemapHandle>(instance.texture);
        m_materialUpdateCallback(materialID, info);
    }
}

void Backend::releaseMaterialTextures(const MaterialInstance& material)
{
    if (material.type == MaterialType::PBR) {
        for (int type = 0; type < TextureType::COUNT; ++type) {
            if (material.useImage[type])
                clearTexture(material.images[type]);
        }
    } else if (material.hasTexture) {
        if (const auto* texture = std::get_if<TextureHandle>(&material.texture))
            clearTexture(*texture);
        else
            clearCubemap(std::get<CubemapHandle>(material.texture));
    }
}

void Backend::clearMaterial(MaterialHandle materialID)
{
    const bool removed = m_materialInstances.release(materialID, [this](MaterialInstance& material) {
        releaseMaterialTextures(material);
    });
    if (!removed)
        return;

    if (m_materialDestroyCallback)
        m_materialDestroyCallback(materialID);

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

GeometryHandle Backend::generateCube(float dimmension, bool inward, std::function<void()> destroyCallback)
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

GeometryHandle Backend::generateQuad(vec2 dimmensions, std::function<void()> destroyCallback)
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

GeometryHandle Backend::generatePyramid(float baseSize, float height, std::function<void()> destroyCallback)
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

void Backend::clearCubemap(CubemapHandle cubemapID)
{
    if (m_cubemapInstances.release(cubemapID))
        m_cubemapUploadLifetimes.erase(cubemapID);
}

FramebufferHandle Backend::instanciateFrameBuffer(unsigned int width, unsigned int height, FramebufferTextureFormat format, unsigned int colorCount, unsigned int depthLayerCount)
{
    if (!m_frameBufferInstances.canCreate())
        return {};

    FramebufferHandle frameBufferID = m_frameBufferInstances.create(width, height, format, colorCount, depthLayerCount);
    m_frameBufferInstances.get(frameBufferID)->unbind();
    checkOpenGLErrors("Instantiate frameBuffer");
    return frameBufferID;
}

CubemapFramebufferHandle Backend::instantiateCubemapFrameBuffer(unsigned int resolution, unsigned int colorCount)
{
    if (!m_cubemapFrameBufferInstances.canCreate())
        return {};

    CubemapFramebufferHandle frameBufferID = m_cubemapFrameBufferInstances.create(resolution, colorCount);
    m_cubemapFrameBufferInstances.get(frameBufferID)->unbind();
    checkOpenGLErrors("Instantiate frameBuffer");
    return frameBufferID;
}

void Backend::clearFrameBuffer(FramebufferHandle frameBufferID)
{
    if (m_frameBufferInstances.release(frameBufferID))
        releaseAttachments(frameBufferID);
    checkOpenGLErrors("Clear frameBuffer");
}

void Backend::clearFrameBuffer(CubemapFramebufferHandle frameBufferID)
{
    if (m_cubemapFrameBufferInstances.release(frameBufferID))
        releaseAttachments(frameBufferID);
    checkOpenGLErrors("Clear cubemap frameBuffer");
}

void Backend::releaseAttachments(FramebufferHandle framebuffer)
{
    const auto found = m_framebufferAttachments.find(framebuffer);
    if (found == m_framebufferAttachments.end())
        return;

    for (TextureHandle texture : found->second.colors) {
        if (texture)
            clearTexture(texture);
    }
    if (found->second.depth)
        clearTexture(found->second.depth);
    m_framebufferAttachments.erase(found);
}

void Backend::releaseAttachments(CubemapFramebufferHandle framebuffer)
{
    const auto found = m_cubemapFramebufferAttachments.find(framebuffer);
    if (found == m_cubemapFramebufferAttachments.end())
        return;

    for (CubemapHandle cubemap : found->second.colors) {
        if (cubemap)
            clearCubemap(cubemap);
    }
    if (found->second.depth)
        clearCubemap(found->second.depth);
    m_cubemapFramebufferAttachments.erase(found);
}

void Backend::resizeFrameBuffer(FramebufferHandle frameBufferID, unsigned int width, unsigned int height, int depthLayerCount)
{
    auto* framebuffer = m_frameBufferInstances.tryGet(frameBufferID);
    if (framebuffer == nullptr) {
        GLX_CORE_ERROR("Trying to resize an unknown framebuffer (slot={0}, generation={1})",
            frameBufferID.index(), frameBufferID.generation());
        return;
    }
    framebuffer->resize(width, height, depthLayerCount);
}

void Backend::resizeCubemapFrameBuffer(CubemapFramebufferHandle frameBufferID, unsigned int size)
{
    auto* framebuffer = m_cubemapFrameBufferInstances.tryGet(frameBufferID);
    if (framebuffer == nullptr) {
        GLX_CORE_ERROR("Trying to resize an unknown cubemap framebuffer (slot={0}, generation={1})",
            frameBufferID.index(), frameBufferID.generation());
        return;
    }
    framebuffer->resize(size);
}

FramebufferTextureFormat Backend::getFramebufferFormat(FramebufferHandle id)
{
    auto* framebuffer = m_frameBufferInstances.tryGet(id);
    if (framebuffer == nullptr) {
        GLX_CORE_ERROR("Trying to inspect an unknown framebuffer (slot={0}, generation={1})",
            id.index(), id.generation());
        return FramebufferTextureFormat::None;
    }
    return framebuffer->getFormat();
}

unsigned int Backend::getFrameBufferTextureID(FramebufferHandle frameBufferID)
{
    auto* framebuffer = m_frameBufferInstances.tryGet(frameBufferID);
    if (framebuffer == nullptr) {
        GLX_CORE_ERROR("Trying to inspect an unknown framebuffer (slot={0}, generation={1})",
            frameBufferID.index(), frameBufferID.generation());
        return 0;
    }
    return framebuffer->getColorTextureID();
}

unsigned int Backend::getFrameBufferDepthTextureID(FramebufferHandle frameBufferID)
{
    auto* framebuffer = m_frameBufferInstances.tryGet(frameBufferID);
    if (framebuffer == nullptr) {
        GLX_CORE_ERROR("Trying to inspect an unknown framebuffer (slot={0}, generation={1})",
            frameBufferID.index(), frameBufferID.generation());
        return 0;
    }
    return framebuffer->getDepthTextureID();
}

void Backend::setCullMode(GeometryHandle visualInstanceID, CullMode mode)
{
    auto* geometry = m_visualInstances.tryGet(visualInstanceID);
    if (geometry == nullptr) {
        GLX_CORE_ERROR("Cannot change cull mode on an invalid geometry handle");
        return;
    }
    geometry->setCullMode(mode);
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

CubemapHandle Backend::instantiateCubemap(std::array<ResourceHandle<Image>, 6> faces)
{
    if (!m_cubemapInstances.canCreate())
        return {};

    CubemapHandle cubemapID = m_cubemapInstances.create();
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
            auto& faceResource = faces[i].getResource();
            cubemapInstance->initFace(
                static_cast<unsigned int>(i),
                faceResource.getData(),
                faceResource.getWidth(),
                faceResource.getHeight(),
                faceResource.getNbChannels());
        });
    }

    return cubemapID;
}

CubemapHandle Backend::instantiateCubemap(int resolution, TextureFormat format,
    TextureFiltering filtering, TextureWrap wrap)
{
    if (!m_cubemapInstances.canCreate() || resolution <= 0)
        return {};

    CubemapHandle cubemapID = m_cubemapInstances.create(
        format, static_cast<unsigned int>(resolution), filtering, wrap);

    return cubemapID;
}

void Backend::setCubemapData(CubemapHandle cubemapID, std::array<ResourceHandle<Image>, 6> faces)
{
    auto* cubemapTexture  = m_cubemapInstances.get(cubemapID);
    if(cubemapTexture == nullptr){
        GLX_ERROR("cubemap not found when loading data");
        return;
    }
    cubemapTexture->beginFaceUpload();
    
    auto uploadLifetime = std::make_shared<int>(0);
    m_cubemapUploadLifetimes[cubemapID] = uploadLifetime;
    const std::weak_ptr<int> weakUploadLifetime = uploadLifetime;
    const std::weak_ptr<int> backendLifetime = m_lifetimeToken;

    for (int i = 0; i < 6; i++) {
        faces[i].getResource().onLoaded([this, backendLifetime, weakUploadLifetime, faces, cubemapID, i] {
            if (backendLifetime.expired() || weakUploadLifetime.expired())
                return;

            auto* cubemapInstance = m_cubemapInstances.tryGet(cubemapID);
            if (cubemapInstance == nullptr){
                GLX_ERROR("cubemap not found when loading data");
                return;
            }
            auto& faceResource = faces[i].getResource();
            cubemapInstance->initFace(
                static_cast<unsigned int>(i),
                faceResource.getData(),
                faceResource.getWidth(),
                faceResource.getHeight(),
                faceResource.getNbChannels());
        });
    }
}

void Backend::setProjectionMatrix(const mat4& projectionMatrix)
{
    const mat4 inverseProjection = inverse(projectionMatrix);
    for (Program* program : m_programInstances.getAll()) {
        program->use();
        program->setUniform("projection", projectionMatrix);
        program->setUniform("inverseProjection", inverseProjection);
    }

    if (Program* activeProgram = getActiveProgram())
        activeProgram->use();
}

void Backend::draw(const GeometryHandle geometryH, const mat4 model)
{
    auto* geometry = m_visualInstances.tryGet(geometryH);
    if (geometry == nullptr) {
        GLX_CORE_ERROR("Skipping draw with an invalid geometry handle (slot={0}, generation={1})",
            geometryH.index(), geometryH.generation());
        return;
    }

    Program* program = getActiveProgram();
    if (program == nullptr) {
        GLX_CORE_ERROR("Skipping draw because no valid program is active");
        return;
    }

    program->setUniform("model", model);
    geometry->draw();
    ++m_drawCount;
}

void Backend::draw(const GeometryHandle geometryH)
{
    if (getActiveProgram() == nullptr) {
        GLX_CORE_ERROR("Skipping draw because no valid program is active");
        return;
    }

    auto* geometry = m_visualInstances.tryGet(geometryH);
    if (geometry == nullptr) {
        GLX_CORE_ERROR("Skipping draw with an invalid geometry handle (slot={0}, generation={1})",
            geometryH.index(), geometryH.generation());
        return;
    }
    geometry->draw();
    ++m_drawCount;
}

bool Backend::attachColorCubemapToFramebuffer(CubemapHandle cubemapHandle,
    CubemapFramebufferHandle fbHandle, int colorAttachmentIdx)
{
    auto* framebuffer = m_cubemapFrameBufferInstances.tryGet(fbHandle);
    auto* cubemap = m_cubemapInstances.tryGet(cubemapHandle);
    if (framebuffer == nullptr || cubemap == nullptr || colorAttachmentIdx < 0) {
        GLX_CORE_ERROR("Cannot attach invalid cubemap/framebuffer GPU handles");
        return false;
    }

    auto& attachments = m_cubemapFramebufferAttachments[fbHandle];
    if (attachments.colors.size() <= static_cast<std::size_t>(colorAttachmentIdx))
        attachments.colors.resize(static_cast<std::size_t>(colorAttachmentIdx) + 1);
    CubemapHandle& destination = attachments.colors[colorAttachmentIdx];

    const CubemapHandle previous = destination;
    if (previous != cubemapHandle)
        m_cubemapInstances.retain(cubemapHandle);

    const bool attached = framebuffer->attachColorCubemap(*cubemap, colorAttachmentIdx);

    if (!attached) {
        if (previous != cubemapHandle)
            clearCubemap(cubemapHandle);
        return false;
    }

    destination = cubemapHandle;
    if (previous && previous != cubemapHandle)
        clearCubemap(previous);
    return true;
}

bool Backend::attachDepthCubemapToFramebuffer(CubemapHandle cubemapHandle,
    CubemapFramebufferHandle fbHandle)
{
    auto* framebuffer = m_cubemapFrameBufferInstances.tryGet(fbHandle);
    auto* cubemap = m_cubemapInstances.tryGet(cubemapHandle);
    if (framebuffer == nullptr || cubemap == nullptr) {
        GLX_CORE_ERROR("Cannot attach invalid cubemap/framebuffer GPU handles");
        return false;
    }

    auto& destination = m_cubemapFramebufferAttachments[fbHandle].depth;
    const CubemapHandle previous = destination;
    if (previous != cubemapHandle)
        m_cubemapInstances.retain(cubemapHandle);

    if (!framebuffer->attachDepthCubemap(*cubemap)) {
        if (previous != cubemapHandle)
            clearCubemap(cubemapHandle);
        return false;
    }

    destination = cubemapHandle;
    if (previous && previous != cubemapHandle)
        clearCubemap(previous);
    return true;
}

void Backend::bindMaterial(MaterialHandle materialHandle)
{
    const auto& material = *m_materialInstances.get(materialHandle);
    if (material.type == MaterialType::PBR) {
        bindPBRMaterial(material);
        return;
    }

    auto& program = *getActiveProgram();
    program.setUniform("transparencyVal", material.transparency);
    program.setUniform("useTexture", material.hasTexture);
    if (material.hasTexture) {
        const int location = program.getUniformLocation("sampledTexture");
        if (const auto* texture = std::get_if<TextureHandle>(&material.texture))
            m_textureInstances.get(*texture)->activate(location);
        else
            m_cubemapInstances.get(std::get<CubemapHandle>(material.texture))->activate(location);
    }
}

void Backend::bindPBRMaterial(const MaterialInstance& material)
{
    auto& program = *getActiveProgram();
    program.setUniform("metallicVal", material.metallic);
    program.setUniform("roughnessVal", material.roughness);
    program.setUniform("aoVal", material.ambient);
    program.setUniform("albedoVal", material.albedo);
    program.setUniform("transparencyVal", material.transparency);

    static const std::array<const char*, TextureType::COUNT> useUniforms {
        "useAlbedoMap", "useNormalMap", "useMetallicMap", "useRoughnessMap", "useAoMap"
    };
    static const std::array<const char*, TextureType::COUNT> samplerUniforms {
        "albedoMap", "normalMap", "metallicMap", "roughnessMap", "aoMap"
    };
    for (int type = 0; type < TextureType::COUNT; ++type) {
        program.setUniform(useUniforms[type], material.useImage[type]);
        if (material.useImage[type])
            m_textureInstances.get(material.images[type])->activate(program.getUniformLocation(samplerUniforms[type]));
    }
}

void Backend::processCommand(const SetUniformCommand& command)
{
    Program* program = getActiveProgram();
    if (program == nullptr) {
        GLX_CORE_ERROR("Cannot set uniform '{0}' because no valid program is active", command.uniformName);
        return;
    }

    bool updated = false;
    if (command.type == SetValueTypes::BOOL) {
        updated = program->setUniform(command.uniformName, command.valueBool);
    } else if (command.type == SetValueTypes::FLOAT) {
        updated = program->setUniform(command.uniformName, command.valueFloat);
    } else if (command.type == SetValueTypes::INT) {
        updated = program->setUniform(command.uniformName, command.valueInt);
    } else if (command.type == SetValueTypes::VEC3) {
        updated = program->setUniform(command.uniformName,
            vec3(command.valueVec3.x, command.valueVec3.y, command.valueVec3.z));
    } else if (command.type == SetValueTypes::IVEC3) {
        updated = program->setUniform(command.uniformName,
            ivec3(command.valueIVec3.x, command.valueIVec3.y, command.valueIVec3.z));
    } else if (command.type == SetValueTypes::VEC2) {
        updated = program->setUniform(command.uniformName,
            vec2(command.valueVec2.x, command.valueVec2.y));
    } else if (command.type == SetValueTypes::MAT4) {
        updated = program->setUniform(command.uniformName, command.matrixValue);
    }

    if (!updated)
        GLX_CORE_ERROR("Active program does not expose uniform '{0}'", command.uniformName);

    checkOpenGLErrors("Set uniform");
}

void Backend::processCommand(const UpdateTextureCommand& command)
{
    auto* texture = m_textureInstances.tryGet(command.texture);
    if (texture == nullptr) {
        GLX_CORE_ERROR("Cannot update an invalid texture handle");
        return;
    }
    if (command.newFormat == TextureFormat::NONE)
        texture->resize(command.width, command.height);
    else
        texture->setFormat(command.newFormat);

    checkOpenGLErrors("Update texture");
}

void Backend::processCommand(const UpdateUBOCommand& command)
{
    auto* ubo = m_uboInstances.tryGet(command.ubo);
    if (ubo == nullptr) {
        GLX_CORE_ERROR("Cannot update an invalid uniform buffer handle");
        return;
    }
    ubo->update(command.data.data(), command.data.size());
}

void Backend::processCommand(const BindUBOCommand& command)
{
    auto* ubo = m_uboInstances.tryGet(command.ubo);
    if (ubo == nullptr) {
        GLX_CORE_ERROR("Cannot bind an invalid uniform buffer handle");
        return;
    }
    ubo->bind(command.idx);
}

void Backend::processCommand(const SaveFrameBufferCommand& command)
{
    auto* framebuffer = m_frameBufferInstances.tryGet(command.framebuffer);
    if (framebuffer == nullptr) {
        GLX_CORE_ERROR("Cannot save an invalid framebuffer handle");
        return;
    }
    framebuffer->savePPM(command.path);
}

} // namespace Galaxy
