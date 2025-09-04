#include "Image.hpp"

#include "Core.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stbi.h"

namespace Galaxy {
Image::Image(int width, int height, int nbChannels)
    : m_width(width)
    , m_height(height)
    , m_nbChannels(nbChannels)
{
}

bool Image::load(const std::string& path)
{
    if (getState() == ResourceState::LOADED)
        destroy();

    m_data = stbi_load(path.c_str(), &m_width, &m_height, &m_nbChannels, 0);

    if (!m_data) {
        GLX_CORE_ERROR("Failed to load: '{0}'", path);
    }

    return true;
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

void Image::destroy()
{
    stbi_image_free(m_data);
}

}