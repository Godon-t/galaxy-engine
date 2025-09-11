#include "Backend.hpp"

#include "Helper.hpp"
#include "Log.hpp"
#include "gl_headers.hpp"
#include "rendering/OpenglHelper.hpp"

namespace Galaxy {
Backend::Backend(size_t maxSize)
    : m_visualInstances(maxSize)
    , m_textureInstances(maxSize)
    , m_materialInstances(maxSize)
{
    GLenum error = glGetError();

    checkOpenGLErrors("error before glewInit");
    glewExperimental    = true; // Needed for core profile
    int glewInitialized = glewInit();
    GLX_CORE_ASSERT(glewInitialized == GLEW_OK, "Failed to initialize GLEW")

    checkOpenGLErrors("known error after glewInit");

    glClearColor(1.f, 0.f, 0.2f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LEQUAL);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    m_mainProgram = std::move(ProgramPBR(engineRes("shaders/base.glsl")));
    // m_textureProgram = std::move(Program(engineRes("shaders/texture.glsl")));

    checkOpenGLErrors("Renderer constructor");
}

renderID Backend::instanciateMesh(std::vector<Vertex>& vertices, std::vector<short unsigned int>& indices, std::function<void()> destroyCallback)
{
    if (!m_visualInstances.canAddInstance())
        return -1;

    renderID meshID = m_visualInstances.createResourceInstance();
    m_visualInstances.get(meshID)->init(vertices, indices);

    if (destroyCallback) {
        m_gpuDestroyNotifications[meshID] = destroyCallback;
    }

    return meshID;
}

renderID Backend::instanciateMesh(ResourceHandle<Mesh> mesh, int surfaceIdx)
{
    renderID subMeshID = mesh.getResource().getVisualID(surfaceIdx);
    if (subMeshID != 0) {
        m_visualInstances.increaseCount(subMeshID);
        return subMeshID;
    }

    renderID visualID = m_visualInstances.createResourceInstance();

    mesh.getResource().onLoaded([this, mesh, visualID, surfaceIdx] {
        const auto& meshRes = mesh.getResource();
        m_visualInstances.get(visualID)->init(
            meshRes.getVertices(surfaceIdx),
            meshRes.getIndices(surfaceIdx));
    });

    m_gpuDestroyNotifications[visualID] = [surfaceIdx, mesh] mutable { mesh.getResource().notifyGpuInstanceDestroyed(surfaceIdx); };

    return visualID;
}

void Backend::clearMesh(renderID meshID)
{
    if (!m_visualInstances.tryRemove(meshID))
        return;

    auto it = m_gpuDestroyNotifications.find(meshID);
    if (it != m_gpuDestroyNotifications.end()) {
        it->second();
        m_gpuDestroyNotifications.erase(meshID);
    }
}

renderID Backend::instanciateTexture(ResourceHandle<Image> image)
{
    renderID existingID = image.getResource().getTextureID();
    if (existingID != 0) {
        m_textureInstances.increaseCount(existingID);
        return existingID;
    }

    renderID textureID = m_textureInstances.createResourceInstance();

    image.getResource().onLoaded([this, image, textureID] {
        const auto& imgRes = image.getResource();
        m_textureInstances.get(textureID)->init(imgRes.getData(), imgRes.getWidth(), imgRes.getHeight(), imgRes.getNbChannels());
    });

    m_gpuDestroyNotifications[textureID] = [image] mutable { image.getResource().notifyGpuInstanceDestroyed(); };

    return textureID;
}

void Backend::clearTexture(renderID textureID)
{
    if (!m_textureInstances.tryRemove(textureID))
        return;

    auto it = m_gpuDestroyNotifications.find(textureID);
    if (it != m_gpuDestroyNotifications.end()) {
        it->second();
        m_gpuDestroyNotifications.erase(textureID);
    }
}

renderID Backend::instanciateMaterial(ResourceHandle<Material> material)
{

    renderID existingID = material.getResource().getRenderID();
    if (existingID) {
        m_materialInstances.increaseCount(existingID);
        return existingID;
    }

    renderID materialID = m_materialInstances.createResourceInstance();

    material.getResource().onLoaded([this, material, materialID] {
        auto matInstance        = m_materialInstances.get(materialID);
        const auto& matResource = material.getResource();

        auto setupTexture = [this, &matInstance, &matResource](TextureType type) {
            matInstance->useImage[type] = matResource.canUseImage(type);
            if (matInstance->useImage[type]) {
                matInstance->images[type] = instanciateTexture(matResource.getImage(type));
            }
        };

        setupTexture(ALBEDO);
        setupTexture(METALLIC);
        setupTexture(ROUGHNESS);
        setupTexture(NORMAL);
        setupTexture(AO);

        matInstance->albedo    = matResource.getAlbedo();
        matInstance->metallic  = matResource.getMetallic();
        matInstance->ambient   = matResource.getAmbient();
        matInstance->roughness = matResource.getRoughness();
    });

    return materialID;
}

void Backend::clearMaterial(renderID materialID)
{
    if (!m_materialInstances.tryRemove(materialID))
        return;

    auto it = m_gpuDestroyNotifications.find(materialID);
    if (it != m_gpuDestroyNotifications.end()) {
        it->second();
        m_gpuDestroyNotifications.erase(materialID);
    }
}

void Backend::processCommands(std::vector<RenderCommand>& commands)
{
    for (auto& command : commands) {
        processCommand(command);
    }

    Texture::resetActivationInts();
}

void Backend::processCommand(ClearCommand& clearCommand)
{
    auto& clearColor = clearCommand.color;
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Backend::processCommand(SetViewCommand& setViewCommand)
{
    m_mainProgram.updateViewMatrix(setViewCommand.view);
}

void Backend::processCommand(SetProjectionCommand& command)
{
    m_mainProgram.updateProjectionMatrix(command.projection);
}

void Backend::processCommand(SetActiveProgramCommand& command)
{
    // if (command.program == BaseProgramEnum::TEXTURE)
    //     m_mainProgram = &m_textureProgram;
    // else if (command.program == BaseProgramEnum::PBR)
    //     m_mainProgram = &m_mainProgram;
    // else
    //     GLX_CORE_ASSERT(false, "unknown asked program!");

    m_mainProgram.use();
}

void Backend::processCommand(DrawCommand& command)
{
    auto& modelMatrix = command.model;
    m_mainProgram.updateModelMatrix(modelMatrix);
    m_visualInstances.get(command.instanceId)->draw();
}

void Backend::processCommand(BindTextureCommand& command)
{
    auto uniLoc = glGetUniformLocation(m_mainProgram.getProgramID(), command.uniformName);
    m_textureInstances.get(command.instanceID)->activate(uniLoc);
    checkOpenGLErrors("Bind texture");
}

void Backend::processCommand(BindMaterialCommand& command)
{
    // GLX_CORE_ASSERT(m_activeProgram == &m_mainProgram, "PBR Program not active!");
    MaterialInstance& material = *m_materialInstances.get(command.materialRenderID);
    std::array<Texture, TextureType::COUNT> materialTextures;
    auto addTexture = [&material, &materialTextures, this](TextureType type) {
        if (material.useImage[type]) {
            materialTextures[type] = *m_textureInstances.get(material.images[type]);
        }
    };
    addTexture(ALBEDO);
    addTexture(NORMAL);
    addTexture(METALLIC);
    addTexture(ROUGHNESS);
    addTexture(AO);
    m_mainProgram.updateMaterial(material, materialTextures);

    checkOpenGLErrors("Bind material");
}

void Backend::processCommand(RenderCommand& command)
{
    if (command.type == RenderCommandType::setActiveProgram)
        processCommand(command.setActiveProgram);
    else if (command.type == RenderCommandType::clear)
        processCommand(command.clear);
    else if (command.type == RenderCommandType::setView)
        processCommand(command.setView);
    else if (command.type == RenderCommandType::setProjection)
        processCommand(command.setProjection);
    else if (command.type == RenderCommandType::draw)
        processCommand(command.draw);
    else if (command.type == RenderCommandType::bindTexture)
        processCommand(command.bindTexture);
    else if (command.type == RenderCommandType::bindMaterial)
        processCommand(command.bindMaterial);
    else
        GLX_CORE_ERROR("Unknown render command");
}

} // namespace Galaxy