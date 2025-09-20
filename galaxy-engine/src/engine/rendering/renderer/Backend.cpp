#include "Backend.hpp"

#include "Helper.hpp"
#include "Log.hpp"
#include "gl_headers.hpp"
#include "pch.hpp"
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
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    // glDisable(GL_CULL_FACE);

    m_mainProgram    = std::move(ProgramPBR(engineRes("shaders/base.glsl")));
    m_skyboxProgram  = std::move(ProgramSkybox(engineRes("shaders/skybox.glsl")));
    m_textureProgram = std::move(ProgramTexture(engineRes("shaders/texture.glsl")));

    checkOpenGLErrors("Renderer constructor");
}

void Backend::destroy()
{
    m_materialInstances.removeAll([this](MaterialInstance& mat) {
        for (auto& text : mat.images) {
            m_textureInstances.tryRemove(text);
        }
    });
    m_visualInstances.removeAll([](VisualInstance& visu) {
        visu.~VisualInstance();
    });
    m_cubemapInstances.removeAll([](Cubemap& cubemap) {
        cubemap.destroy();
    });
    m_textureInstances.removeAll([](Texture& texture) {
        texture.destroy();
    });
}

renderID Backend::instanciateMesh(std::vector<Vertex>& vertices, std::vector<short unsigned int>& indices, std::function<void()> destroyCallback)
{
    if (!m_visualInstances.canAddInstance())
        return 0;

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

        matInstance->albedo       = matResource.getAlbedo();
        matInstance->metallic     = matResource.getMetallic();
        matInstance->ambient      = matResource.getAmbient();
        matInstance->roughness    = matResource.getRoughness();
        matInstance->transparency = matResource.getTransparency();
    });

    return materialID;
}

void Backend::updateMaterial(renderID materialID, ResourceHandle<Material> material)
{
    m_materialInstances.get(materialID)->transparency = material.getResource().getTransparency();
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
    v1.texCoord = vec2(0, 0);

    v2.position = vec3(half.x, half.y, 0);
    v2.texCoord = vec2(1, 0);

    v3.position = vec3(-half.x, -half.y, 0);
    v3.texCoord = vec2(0, 1);

    v4.position = vec3(half.x, -half.y, 0);
    v4.texCoord = vec2(1, 1);

    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);
    vertices.push_back(v4);

    std::vector<short unsigned int> indices;
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(1);

    indices.push_back(1);
    indices.push_back(3);
    indices.push_back(2);

    return instanciateMesh(vertices, indices, destroyCallback);
}

renderID Backend::instanciateCubemap()
{
    renderID cubemapID = m_cubemapInstances.createResourceInstance();
    return cubemapID;
}

void Backend::clearCubemap(renderID cubemapID)
{
    if (!m_cubemapInstances.tryRemove(cubemapID))
        return;

    auto it = m_gpuDestroyNotifications.find(cubemapID);
    if (it != m_gpuDestroyNotifications.end()) {
        it->second();
        m_gpuDestroyNotifications.erase(cubemapID);
    }
}

renderID Backend::instanciateCubemap(std::array<ResourceHandle<Image>, 6> faces)
{
    renderID cubemapID = instanciateCubemap();

    auto& cubemapInstance = *m_cubemapInstances.get(cubemapID);

    for (int i = 0; i < 6; i++) {

        faces[i].getResource().onLoaded([faces, &cubemapInstance, i] {
            int w = faces[0].getResource().getWidth();
            int h = faces[0].getResource().getHeight();
            // TODO: Cube map can only take shape of cube ?
            cubemapInstance.resize(w);

            auto& faceResource = faces[i].getResource();
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapInstance.cubemapID);

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, w, w, 0, GL_RGB, GL_UNSIGNED_BYTE, faceResource.getData());
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        });
    }

    return cubemapID;
}

void Backend::processCommand(ClearCommand& clearCommand)
{
    auto& clearColor = clearCommand.color;
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Backend::processCommand(DepthMaskCommand& command)
{
    if (command.state)
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);
}

void Backend::processCommand(SetViewCommand& setViewCommand)
{
    m_activeProgram->updateViewMatrix(setViewCommand.view);
}
void Backend::setProjectionMatrix(const mat4& projectionMatrix)
{
    m_projectionMatrix = projectionMatrix;
    m_activeProgram->updateProjectionMatrix(projectionMatrix);
}
void Backend::processCommand(SetProjectionCommand& command)
{
    setProjectionMatrix(command.projection);
}

void Backend::processCommand(SetActiveProgramCommand& command)
{
    if (command.program == SKYBOX)
        m_activeProgram = &m_skyboxProgram;
    else if (command.program == PBR)
        m_activeProgram = &m_mainProgram;
    else if (command.program == TEXTURE)
        m_activeProgram = &m_textureProgram;
    else
        GLX_CORE_ASSERT(false, "unknown asked program!");

    m_activeProgram->use();
}

void Backend::processCommand(DrawCommand& command)
{
    auto& modelMatrix = command.model;
    m_activeProgram->updateModelMatrix(modelMatrix);
    m_visualInstances.get(command.instanceId)->draw();
}

void Backend::processCommand(BindTextureCommand& command)
{
    auto uniLoc = glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName);
    m_textureInstances.get(command.instanceID)->activate(uniLoc);
    checkOpenGLErrors("Bind texture");
}

void Backend::processCommand(BindCubemapCommand& command)
{
    auto uniLoc   = glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName);
    auto& cubemap = *m_cubemapInstances.get(command.instanceID);
    cubemap.activate(uniLoc);
    checkOpenGLErrors("Bind cubemap");
}

void Backend::processCommand(BindMaterialCommand& command)
{
    GLX_CORE_ASSERT(m_activeProgram->type() == ProgramType::PBR, "PBR Program not active!");

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

    ((ProgramPBR*)m_activeProgram)->updateMaterial(material, materialTextures);

    checkOpenGLErrors("Bind material");
}

void Backend::processCommand(RenderCommand& command)
{
    if (command.type == RenderCommandType::setActiveProgram)
        processCommand(command.setActiveProgram);
    else if (command.type == RenderCommandType::clear)
        processCommand(command.clear);
    else if (command.type == RenderCommandType::depthMask)
        processCommand(command.depthMask);
    else if (command.type == RenderCommandType::setView)
        processCommand(command.setView);
    else if (command.type == RenderCommandType::setProjection)
        processCommand(command.setProjection);
    else if (command.type == RenderCommandType::draw)
        processCommand(command.draw);
    else if (command.type == RenderCommandType::bindTexture)
        processCommand(command.bindTexture);
    else if (command.type == RenderCommandType::bindCubemap)
        processCommand(command.bindCubemap);
    else if (command.type == RenderCommandType::bindMaterial)
        processCommand(command.bindMaterial);
    else
        GLX_CORE_ERROR("Unknown render command");
}

} // namespace Galaxy