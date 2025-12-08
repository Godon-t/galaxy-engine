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
    , m_frameBufferInstances(maxSize)
    , m_cubemapFrameBufferInstances(maxSize)
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

    // TODO: Change the way Program object are created
    m_mainProgram              = std::move(ProgramPBR(engineRes("shaders/base.glsl")));
    m_skyboxProgram            = std::move(ProgramSkybox(engineRes("shaders/skybox.glsl")));
    m_irradianceProgram        = std::move(ProgramSkybox(engineRes("shaders/filters/irradiance.glsl")));
    m_textureProgram           = std::move(ProgramTexture(engineRes("shaders/texture.glsl")));
    m_unicolorProgram          = std::move(ProgramUnicolor(engineRes("shaders/unicolor.glsl")));
    m_postProcessingProgram    = std::move(ProgramPostProc(engineRes("shaders/post_processing.glsl")));
    m_postProcessingSSGIProgram    = std::move(ProgramPostProcSSGI(engineRes("shaders/ssgi.glsl")));
    m_shadowProgram            = std::move(ProgramShadow(engineRes("shaders/shadow_depth.glsl")));
    m_computeOctahedralProgram = std::move(ProgramComputeOctahedral(engineRes("shaders/compute_octahedral.glsl")));

    m_debugLinesProgram = std::move(ProgramDebugLines(engineRes("shaders/debug/line_draw.glsl")));

    m_activeProgram = &m_mainProgram;

    m_debugLines.init();

    checkOpenGLErrors("Renderer constructor");
}

void Backend::destroy()
{
    m_debugLines.destroy();

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

renderID Backend::instantiateTexture(ResourceHandle<Image> image)
{
    renderID existingID = image.getResource().getTextureID();
    if (existingID != 0) {
        m_textureInstances.increaseCount(existingID);
        return existingID;
    }

    renderID textureID = m_textureInstances.createResourceInstance();
    image.getResource().setTextureID(textureID);

    image.getResource().onLoaded([this, image, textureID] {
        auto& imgRes = image.getResource();
        m_textureInstances.get(textureID)->init(imgRes.getData(), imgRes.getWidth(), imgRes.getHeight(), imgRes.getNbChannels());
        imgRes.freeCpuData();
    });

    m_gpuDestroyNotifications[textureID] = [image] mutable { image.getResource().notifyGpuInstanceDestroyed(); };

    return textureID;
}

renderID Backend::instantiateTexture()
{
    if (!m_textureInstances.canAddInstance())
        return 0;

    renderID textureID = m_textureInstances.createResourceInstance();
    m_textureInstances.get(textureID)->resize(1024, 1024);
    checkOpenGLErrors("Instantiate texture");
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
    if (!m_cubemapInstances.tryRemove(cubemapID))
        return;

    auto it = m_gpuDestroyNotifications.find(cubemapID);
    if (it != m_gpuDestroyNotifications.end()) {
        it->second();
        m_gpuDestroyNotifications.erase(cubemapID);
    }
}

renderID Backend::instanciateFrameBuffer(unsigned int width, unsigned int height, FramebufferTextureFormat format, unsigned int colorCount)
{
    renderID frameBufferID = m_frameBufferInstances.createResourceInstance();
    m_frameBufferInstances.get(frameBufferID)->setFormat(format);
    m_frameBufferInstances.get(frameBufferID)->setColorsCount(colorCount);
    m_frameBufferInstances.get(frameBufferID)->resize(width, height);
    m_frameBufferInstances.get(frameBufferID)->unbind();
    checkOpenGLErrors("Instantiate frameBuffer");
    return frameBufferID;
}

renderID Backend::instantiateCubemapFrameBuffer(unsigned int size)
{
    renderID frameBufferID = m_cubemapFrameBufferInstances.createResourceInstance();
    m_cubemapFrameBufferInstances.get(frameBufferID)->resize(size);
    m_cubemapFrameBufferInstances.get(frameBufferID)->unbind();
    checkOpenGLErrors("Instantiate frameBuffer");
    return frameBufferID;
}

void Backend::clearFrameBuffer(renderID frameBufferID)
{
    if (!m_frameBufferInstances.tryRemove(frameBufferID))
        return;

    auto it = m_gpuDestroyNotifications.find(frameBufferID);
    if (it != m_gpuDestroyNotifications.end()) {
        it->second();
        m_gpuDestroyNotifications.erase(frameBufferID);
    }
    checkOpenGLErrors("Clear frameBuffer");
}

void Backend::resizeFrameBuffer(renderID frameBufferID, unsigned int width, unsigned int height)
{
    m_frameBufferInstances.get(frameBufferID)->resize(width, height);
}

void Backend::resizeCubemapFrameBuffer(renderID frameBufferID, unsigned int size)
{
    m_cubemapFrameBufferInstances.get(frameBufferID)->resize(size);
}

FramebufferTextureFormat Backend::getFramebufferFormat(renderID id)
{
    return m_frameBufferInstances.get(id)->getFormat();
}

unsigned int Backend::getFrameBufferTextureID(renderID frameBufferID)
{
    return m_frameBufferInstances.get(frameBufferID)->getColorTextureID();
}

unsigned int Backend::getFrameBufferDepthTextureID(renderID frameBufferID)
{
    return m_frameBufferInstances.get(frameBufferID)->getDepthTextureID();
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

        char* severityChr;
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

        if (severity == GL_DEBUG_SEVERITY_HIGH) {
            raise(SIGTRAP);
        }
    },
        nullptr);
}

renderID Backend::instanciateCubemap(std::array<ResourceHandle<Image>, 6> faces)
{
    renderID cubemapID = instanciateCubemap();

    auto& cubemapInstance = *m_cubemapInstances.get(cubemapID);

    for (int i = 0; i < 6; i++) {

        faces[i].getResource().onLoaded([faces, &cubemapInstance, i] {
            int w = faces[0].getResource().getWidth();
            int h = faces[0].getResource().getHeight();
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

renderID Backend::instanciateCubemap(int resolution)
{
    renderID cubemapID = m_cubemapInstances.createResourceInstance();
    m_cubemapInstances.get(cubemapID)->resize(resolution);
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

    m_postProcessingProgram.use();
    m_postProcessingProgram.updateViewMatrix(setViewCommand.view);
    m_postProcessingProgram.updateInverseViewMatrix(inverse(setViewCommand.view));

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

    m_postProcessingProgram.use();
    m_postProcessingProgram.updateProjectionMatrix(projectionMatrix);
    m_postProcessingProgram.updateInverseProjectionMatrix(inverse(projectionMatrix));

    m_postProcessingSSGIProgram.use();
    m_postProcessingSSGIProgram.updateProjectionMatrix(projectionMatrix);
    m_postProcessingSSGIProgram.updateInverseProjectionMatrix(inverse(projectionMatrix));

    m_debugLinesProgram.use();
    m_debugLinesProgram.updateProjectionMatrix(projectionMatrix);

    m_activeProgram->use();
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
    else if (command.program == UNICOLOR)
        m_activeProgram = &m_unicolorProgram;
    else if (command.program == POST_PROCESSING)
        m_activeProgram = &m_postProcessingProgram;
    else if (command.program == FILTER_IRRADIANCE)
        m_activeProgram = &m_irradianceProgram;
    else if (command.program == SHADOW_DEPTH)
        m_activeProgram = &m_shadowProgram;
    else if (command.program == COMPUTE_OCTAHEDRAL)
        m_activeProgram = &m_computeOctahedralProgram;
    else if (command.program == POST_PROCESSING_SSGI)
        m_activeProgram = &m_postProcessingSSGIProgram;
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

void Backend::processCommand(RawDrawCommand& command)
{
    m_visualInstances.get(command.instanceID)->draw();
}

void Backend::processCommand(UseTextureCommand& command)
{
    auto uniLoc = glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName);
    m_textureInstances.get(command.instanceID)->activate(uniLoc);
    checkOpenGLErrors("Bind texture");
}

void Backend::processCommand(UseCubemapCommand& command)
{
    auto uniLoc   = glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName);
    auto& cubemap = *m_cubemapInstances.get(command.instanceID);
    cubemap.activate(uniLoc);
    checkOpenGLErrors("Bind cubemap");
}

void Backend::processCommand(AttachTextureToFramebufferCommand& command)
{
    auto& framebuffer = *m_frameBufferInstances.get(command.framebufferID);
    auto& texture     = *m_textureInstances.get(command.textureID);
    if (command.attachmentIdx < 0)
        framebuffer.attachDepthTexture(texture);
    else
        framebuffer.attachColorTexture(texture, command.attachmentIdx);

    checkOpenGLErrors("Attach texture to framebuffer");
}

void Backend::processCommand(AttachCubemapToFramebufferCommand& command)
{
    // TODO: Beware of memory handling !!!
    if (command.colorIdx < 0)
        m_cubemapFrameBufferInstances.get(command.framebufferID)->attachDepthCubemap(*m_cubemapInstances.get(command.cubemapID));
    else
        m_cubemapFrameBufferInstances.get(command.framebufferID)->attachColorCubemap(*m_cubemapInstances.get(command.cubemapID), command.colorIdx);
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
    checkOpenGLErrors("Binding material");
}

void Backend::processCommand(BindFrameBufferCommand& command, bool bind)
{
    if (bind)
        if (command.cubemapFaceIdx >= 0)
            m_cubemapFrameBufferInstances.get(command.frameBufferID)->bind(command.cubemapFaceIdx);
        else
            m_frameBufferInstances.get(command.frameBufferID)->bind();
    else {
        if (command.cubemapFaceIdx >= 0)
            m_cubemapFrameBufferInstances.get(command.frameBufferID)->unbind();
        else
            m_frameBufferInstances.get(command.frameBufferID)->unbind();
    }

    checkOpenGLErrors("Binding framebuffer");
}

// TODO: Rework post processing logic
void Backend::processCommand(InitPostProcessCommand& command)
{
    GLX_CORE_ASSERT(m_activeProgram->type() == ProgramType::POST_PROCESSING || m_activeProgram->type() == ProgramType::POST_PROCESSING_SSGI, "Post processing Program not active!");

    auto& fb = *m_frameBufferInstances.get(command.frameBufferID);
    ((ProgramPostProc*)m_activeProgram)->setTextures(fb.getColorTextureID(), fb.getColorTextureID(1), fb.getDepthTextureID());

    checkOpenGLErrors("Init post process");
}

void Backend::processCommand(SetUniformCommand& command)
{
    if (command.type == SetValueTypes::BOOL) {
        glUniform1i(glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName), command.valueBool ? GL_TRUE : GL_FALSE);
    } else if (command.type == SetValueTypes::FLOAT) {
        glUniform1f(glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName), command.valueFloat);
    } else if (command.type == SetValueTypes::INT) {
        glUniform1i(glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName), command.valueInt);
    } else if (command.type == SetValueTypes::VEC3) {
        glUniform3f(glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName),
            command.valueVec3.x, command.valueVec3.y, command.valueVec3.z);
    } else if (command.type == SetValueTypes::IVEC3) {
        glUniform3i(glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName),
            command.valueIVec3.x, command.valueIVec3.y, command.valueIVec3.z);
    } else if (command.type == SetValueTypes::VEC2) {
        glUniform2f(glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName),
            command.valueVec2.x, command.valueVec2.y);
    } else if (command.type == SetValueTypes::MAT4) {
        glUniformMatrix4fv(glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName), 1, GL_FALSE, &command.matrixValue[0][0]);
    }

    free(command.uniformName);

    checkOpenGLErrors("Set uniform");
}

void Backend::processCommand(SetViewportCommand& command)
{
    glViewport((int)command.position.x, (int)command.position.y, (int)command.size.x, (int)command.size.y);
}

void Backend::processCommand(UpdateTextureCommand& command)
{
    if (command.newFormat == TextureFormat::NONE)
        m_textureInstances.get(command.targetID)->resize(command.width, command.height);
    else
        m_textureInstances.get(command.targetID)->setFormat(command.newFormat);

    checkOpenGLErrors("Update texture");
}

void Backend::processCommand(UpdateCubemapCommand& command)
{
    m_cubemapInstances.get(command.targetID)->resize(command.resolution);
    checkOpenGLErrors("Update cubemap");
}

void Backend::processCommand(SetFramebufferAsTextureUniformCommand& command)
{
    auto uniLoc       = glGetUniformLocation(m_activeProgram->getProgramID(), command.uniformName);
    auto& framebuffer = *m_frameBufferInstances.get(command.framebufferID);
    framebuffer.setAsTextureUniform(uniLoc, command.textureIdx);
    checkOpenGLErrors("Bind framebuffer texture as uniform");
    free(command.uniformName);
}

void Backend::processCommand(DebugMsgCommand& command)
{
    GLX_CORE_TRACE(command.msg);
    free(command.msg);
}

void Backend::processCommand(DrawDebugLineCommand& command)
{
    m_debugLines.addLine(command.start, command.end, vec3(0, 1, 0));
}

void Backend::processCommand(SaveFrameBufferCommand& command)
{
    m_frameBufferInstances.get(command.frameBufferID)->savePPM(command.path);

    free(command.path);
}

void Backend::debugDraw()
{
    m_debugLinesProgram.use();
    m_debugLines.draw();
}

void Backend::processCommand(RenderCommand& command)
{
    switch (command.type) {
    case RenderCommandType::setActiveProgram:
        processCommand(command.setActiveProgram);
        break;
    case RenderCommandType::clear:
        processCommand(command.clear);
        break;
    case RenderCommandType::depthMask:
        processCommand(command.depthMask);
        break;
    case RenderCommandType::setView:
        processCommand(command.setView);
        break;
    case RenderCommandType::setProjection:
        processCommand(command.setProjection);
        break;
    case RenderCommandType::rawDraw:
        processCommand(command.rawDraw);
        break;
    case RenderCommandType::draw:
        processCommand(command.draw);
        break;
    case RenderCommandType::useTexture:
        processCommand(command.useTexture);
        break;
    case RenderCommandType::useCubemap:
        processCommand(command.useCubemap);
        break;
    case RenderCommandType::attachTextureToFramebuffer:
        processCommand(command.attachTextureToFramebuffer);
        break;
    case RenderCommandType::attachCubemapToFramebuffer:
        processCommand(command.attachCubemapToFramebuffer);
        break;
    case RenderCommandType::bindMaterial:
        processCommand(command.bindMaterial);
        break;
    case RenderCommandType::bindFrameBuffer:
        processCommand(command.bindFrameBuffer, true);
        break;
    case RenderCommandType::unbindFrameBuffer:
        processCommand(command.bindFrameBuffer, false);
        break;
    case RenderCommandType::initPostProcess:
        processCommand(command.initPostProcess);
        break;
    case RenderCommandType::setUniform:
        processCommand(command.setUniform);
        break;
    case RenderCommandType::setViewport:
        processCommand(command.setViewport);
        break;
    case RenderCommandType::updateCubemap:
        processCommand(command.updateCubemap);
        break;
    case RenderCommandType::updateTexture:
        processCommand(command.updateTexture);
        break;
    case RenderCommandType::setFramebufferAsTextureUniformCommand:
        processCommand(command.setFramebufferAsTextureUniform);
        break;
    case RenderCommandType::debugMsg:
        processCommand(command.debugMsg);
        break;
    case RenderCommandType::drawDebugLine:
        processCommand(command.drawDebugLine);
        break;
    case RenderCommandType::executeDebugCommands:
        debugDraw();
        break;
    case RenderCommandType::saveFrameBuffer:
        processCommand(command.saveFrameBuffer);
        break;
    default:
        GLX_CORE_ERROR("Unknown render command");
        break;
    }

    checkOpenGLErrors("Process command");
}

} // namespace Galaxy
