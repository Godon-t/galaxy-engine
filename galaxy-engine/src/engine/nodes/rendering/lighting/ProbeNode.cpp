#include "ProbeNode.hpp"

#include "rendering/Renderer.hpp"
#include "resource/ResourceManager.hpp"

#include <filesystem>

namespace Galaxy {
ProbeNode::ProbeNode(std::string name)
    : Node3D(name)
{
}

ProbeNode::~ProbeNode()
{
    if (!m_cubemap)
        return;

    auto& backend = Renderer::getInstance().getBackend();
    backend.clearFrameBuffer(m_captureFramebuffer);
    backend.clearCubemap(m_cubemap);
}

void ProbeNode::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}

void ProbeNode::capture()
{
    m_capturePending = true;
}

uuid ProbeNode::getCubemapResourceID() const
{
    return m_cubemapResource.hasResource() ? m_cubemapResource.getResource().getResourceID() : uuid(0);
}

bool ProbeNode::loadCubemap(const std::string& resourcePath)
{
    if (!Project::doesPathExist(resourcePath)) {
        GLX_CORE_ERROR("Unknown cubemap resource: '{0}'", resourcePath);
        return false;
    }
    m_cubemapResource = ResourceManager::getInstance().load<CubemapResource>(resourcePath);
    m_capturePending = false;
    loadCubemapData();
    return true;
}

bool ProbeNode::saveCapture(const std::string& folder)
{
    if (!m_cubemap)
        return false;

    namespace fs = std::filesystem;
    try {
        const fs::path root = fs::absolute(Project::getProjectRootPath()).lexically_normal();
        const bool hasResource = m_cubemapResource.hasResource();
        const fs::path oldFile = (root / getCubemapResourcePath()).lexically_normal();
        const fs::path destination = folder.empty() && hasResource
            ? oldFile.parent_path() : (root / folder).lexically_normal();
        const fs::path resourceFile = (destination / (hasResource ? oldFile.filename() : fs::path("cubemap.gres"))).lexically_normal();
        const std::string resourcePath = fs::relative(resourceFile, root).generic_string();

        auto& manager = ResourceManager::getInstance();
        if (!hasResource) {
            CubemapResource resource;
            m_cubemapResource = manager.registerNewResource(resource, resourcePath, true);
        } else if (oldFile.parent_path() != resourceFile.parent_path()) {
            m_cubemapResource.getResource().removeFiles();
        }
        manager.setResourcePath(m_cubemapResource, resourcePath);

        auto& resource = m_cubemapResource.getResource();
        const bool hdr = fs::path(resource.getFacePaths()[0]).extension() == ".hdr";
        const fs::path prefix = resourceFile.parent_path() / resourceFile.stem();
        fs::create_directories(resourceFile.parent_path());
        if (!Renderer::getInstance().getBackend().saveCubemap(m_cubemap, prefix.string(), hdr))
            return false;

        CubemapResource::FacePaths paths;
        for (std::size_t i = 0; i < paths.size(); ++i)
            paths[i] = fs::relative(prefix.string() + "_" + CubemapResource::FaceNames[i]
                + (hdr ? ".hdr" : ".png"), root).generic_string();
        if (!resource.loadFaces(paths) || !resource.save(false))
            return false;

        Project::savePaths();
        return true;
    } catch (const std::exception& error) {
        GLX_CORE_ERROR("Cannot save probe cubemap: {0}", error.what());
        return false;
    }
}

void ProbeNode::loadCubemapData()
{
    if (!m_cubemap || !m_cubemapResource.hasResource())
        return;

    m_cubemapResource.getResource().onLoaded([this] {
        auto& resource = m_cubemapResource.getResource();
        m_resolution = static_cast<unsigned int>(resource.getResolution());
        auto& backend = Renderer::getInstance().getBackend();
        backend.resizeCubemapFrameBuffer(m_captureFramebuffer, m_resolution);
        backend.setCubemapData(m_cubemap, resource.getFaces());
    });
}

void ProbeNode::draw()
{
    if (m_capturePending) {
        Camera capture;
        capture.position = getTransform().getGlobalPosition();
        capture.viewportSize = vec2(m_resolution);
        capture.targetFramebuffer = m_captureFramebuffer;
        capture.frustumCulling = false;
        Renderer::getInstance().getFrontend().addCamera(std::move(capture));
        m_capturePending = false;
    }

    Node3D::draw();
}

void ProbeNode::enteredRoot()
{
    auto& backend = Renderer::getInstance().getBackend();
    m_cubemap = backend.instantiateCubemap(m_resolution);
    m_captureFramebuffer = backend.instantiateCubemapFrameBuffer(m_resolution);
    backend.attachColorCubemapToFramebuffer(m_cubemap, m_captureFramebuffer, 0);
    loadCubemapData();
}
} // namespace Galaxy
