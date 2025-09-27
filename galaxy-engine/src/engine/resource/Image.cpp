#include "Image.hpp"

#include "Core.hpp"
#include "Log.hpp"
#include "ResourceSerializer.hpp"
#include "project/Project.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stbi.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

namespace Galaxy {
Image::Image(int width, int height, int nbChannels)
    : m_width(width)
    , m_height(height)
    , m_nbChannels(nbChannels)
{
}

bool Image::load(YAML::Node& data)
{
    if (!data["Type"] || data["Type"].as<std::string>() != std::string("Image")) {
        GLX_CORE_ERROR("File unsupported");
        return false;
    }

    if (data["ExternalFile"]) {
        return loadExtern(data["ExternalFile"].as<std::string>());
    }

    return false;
}

bool Image::save(bool recursive)
{
    return ResourceSerializer::serialize(*this);
}

void Image::destroy()
{
    stbi_image_free(m_data);
}

void Image::freeCpuData()
{
    stbi_image_free(m_data);
    m_freed = true;
}

bool Image::loadExtern(const std::string& path)
{
    m_freed                    = false;
    m_relativeExternalFilePath = path;
    m_isInternal               = false;

    std::string filePath = Project::getProjectRootPath() + path;

    m_data = stbi_load(filePath.c_str(), &m_width, &m_height, &m_nbChannels, 0);

    if (!m_data) {
        GLX_CORE_ERROR("Failed to load: '{0}'", path);
        return false;
    }

    // if (m_width > 2000 || m_height > 2000) {
    //     int newWidth  = m_width / 2;
    //     int newHeight = m_height / 2;
    //     GLX_CORE_WARN("Resizing image (from: {0} x {1}, to: {2} x {3})", m_width, m_height, newWidth, newHeight);

    //     unsigned char* resizedData = new unsigned char[newWidth * newHeight * m_nbChannels];

    //     stbir_resize_uint8_linear(m_data, m_width, m_height, 0,
    //         resizedData, newWidth, newHeight, 0,
    //         m_nbChannels == 3 ? STBIR_RGB : STBIR_RGBA);

    //     stbi_image_free(m_data);

    //     m_data   = resizedData;
    //     m_width  = newWidth;
    //     m_height = newHeight;
    // }

    return true;
}

unsigned char* Image::getData()
{
    if (!m_freed)
        return m_data;
    else {
        loadExtern(m_relativeExternalFilePath);
        return m_data;
    }
}

bool Image::hasTransparency()
{
    if (m_nbChannels <= 3)
        return false;
    for (int i = 0; i < m_width * m_height; ++i) {
        unsigned char alpha = m_data[i * 4 + 3];
        if (alpha < 255) {
            return true;
        }
    }
    return false;
}

}