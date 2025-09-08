#include "Image.hpp"

#include "Core.hpp"
#include "ResourceSerializer.hpp"
#include "project/Project.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stbi.h"

namespace Galaxy {
Image::Image(int width, int height, int nbChannels)
    : m_width(width)
    , m_height(height)
    , m_nbChannels(nbChannels)
{
}

bool Image::load(const unsigned char* data, size_t size)
{
    if (getState() == ResourceState::LOADED)
        destroy();

    m_data = stbi_load_from_memory(data, size, &m_width, &m_height, &m_nbChannels, 0);

    if (!m_data) {
        GLX_CORE_ERROR("Failed to load embedded texture from memory");
        return false;
    }

    return true;
}

bool Image::save()
{
    return ResourceSerializer::serialize(*this, m_resourcePath);
}

bool Image::import(const std::string& file)
{
    GLX_CORE_ASSERT(loadExtern(file), "Failed to import");

    std::string path, extension;
    Project::extractExtension(file, path, extension);
    path += std::string(".gres");

    m_resourcePath             = path;
    m_relativeExternalFilePath = file;

    if (save()) {
        // Succesfully created resource file
        Project::deletePath(ProjectPathTypes::RESOURCE, m_resourceID);
        m_resourceID = Project::registerNewPath(ProjectPathTypes::RESOURCE, m_resourcePath);
        return true;
    } else {
        return false;
    }
}

void Image::destroy()
{
    stbi_image_free(m_data);
}
bool Image::loadExtern(const std::string& path)
{
    if (getState() == ResourceState::LOADED)
        destroy();

    m_data                     = stbi_load((Project::getProjectRootPath() + path).c_str(), &m_width, &m_height, &m_nbChannels, 0);
    m_relativeExternalFilePath = path;
    m_isInternal               = false;

    if (!m_data) {
        GLX_CORE_ERROR("Failed to load: '{0}'", path);
    }

    return true;
}
bool Image::loadGres(const std::string& file)
{
    return ResourceDeserializer::deserialize(*this, file);
}
}