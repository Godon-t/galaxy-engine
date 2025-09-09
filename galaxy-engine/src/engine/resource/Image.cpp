#include "Image.hpp"

#include "Core.hpp"
#include "Log.hpp"
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

bool Image::load(YAML::Node& data)
{
    return ResourceDeserializer::deserialize(*this, data);
}

bool Image::save()
{
    return ResourceSerializer::serialize(*this);
}

void Image::destroy()
{
    stbi_image_free(m_data);
}

bool Image::loadExtern(const std::string& path)
{
    m_relativeExternalFilePath = path;
    m_isInternal               = false;

    std::string filePath = Project::getProjectRootPath() + path;

    m_data = stbi_load(filePath.c_str(), &m_width, &m_height, &m_nbChannels, 0);
    if (!m_data) {
        GLX_CORE_ERROR("Failed to load: '{0}'", path);
        return false;
    }

    return true;
}

}