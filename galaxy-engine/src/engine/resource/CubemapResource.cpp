#include "CubemapResource.hpp"

#include "Log.hpp"
#include "ResourceManager.hpp"
#include "ResourceSerializer.hpp"
#include "project/Project.hpp"

#include <utility>

namespace Galaxy {
namespace {
std::filesystem::path faceFilePath(const std::string& path)
{
    const std::filesystem::path file(path);
    return file.is_absolute() ? file : std::filesystem::path(Project::getProjectRootPath()) / file;
}
}

CubemapResource::CubemapResource(CubemapResource&& other) noexcept
    : m_faces(std::move(other.m_faces))
    , m_facePaths(std::move(other.m_facePaths))
    , m_resolution(other.m_resolution)
{
    m_isInternal = other.m_isInternal;
    m_resourceID = other.m_resourceID;
    m_resourcePath = std::move(other.m_resourcePath);
}

bool CubemapResource::loadFaces(const FacePaths& paths)
{
    Faces faces;
    int resolution = 0;
    int channels = 0;
    for (std::size_t i = 0; i < faces.size(); ++i) {
        if (paths[i].empty()) {
            GLX_CORE_ERROR("Missing cubemap face: {0}", FaceNames[i]);
            return false;
        }
        auto image = ResourceManager::getInstance().createInternalResource<Image>();
        auto& pixels = image.getResource();
        if (!pixels.loadExtern(faceFilePath(paths[i]).string()))
            return false;
        if (i == 0) {
            resolution = pixels.getWidth();
            channels = pixels.getNbChannels();
        }
        if (resolution <= 0 || pixels.getWidth() != resolution || pixels.getHeight() != resolution
            || pixels.getNbChannels() != channels) {
            GLX_CORE_ERROR("Cubemap faces must be square and share the same resolution and channel count");
            return false;
        }
        faces[i] = std::move(image);
    }
    m_faces = std::move(faces);
    m_facePaths = paths;
    m_resolution = resolution;
    return true;
}

bool CubemapResource::load(YAML::Node& data)
{
    try {
        if (!data["Type"] || data["Type"].as<std::string>() != "CubemapResource"
            || !data["Faces"].IsMap() || data["Faces"].size() != FaceNames.size()
            || !data["Resolution"] || data["Resolution"].as<int>() <= 0) {
            GLX_CORE_ERROR("Invalid cubemap resource");
            return false;
        }
        FacePaths paths;
        for (std::size_t i = 0; i < paths.size(); ++i) {
            if (!data["Faces"][FaceNames[i]]) {
                GLX_CORE_ERROR("Missing cubemap face: {0}", FaceNames[i]);
                return false;
            }
            paths[i] = data["Faces"][FaceNames[i]].as<std::string>();
        }
        if (!loadFaces(paths))
            return false;
        if (m_resolution != data["Resolution"].as<int>()) {
            GLX_CORE_ERROR("Cubemap resource resolution does not match its face images");
            return false;
        }
        return true;
    } catch (const YAML::Exception& error) {
        GLX_CORE_ERROR("Invalid cubemap resource: {0}", error.what());
        return false;
    }
}

bool CubemapResource::save(bool recursive)
{
    if (m_resolution <= 0 || m_resourcePath.empty()) {
        GLX_CORE_ERROR("Cannot save an unregistered or empty cubemap resource");
        return false;
    }
    if (recursive) {
        for (std::size_t i = 0; i < m_faces.size(); ++i) {
            if (!m_faces[i] || !m_faces[i].getResource().save(faceFilePath(m_facePaths[i]).string()))
                return false;
        }
    }
    return ResourceSerializer::serialize(*this);
}

void CubemapResource::removeFiles() const
{
    for (const auto& path : m_facePaths) {
        if (!path.empty())
            std::filesystem::remove(faceFilePath(path));
    }
    if (!m_resourcePath.empty())
        std::filesystem::remove(faceFilePath(m_resourcePath));
}
} // namespace Galaxy
