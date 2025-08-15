#include "Texture.hpp"

#include "Core.hpp"
#include "OpenglHelper.hpp"
#include "gl_headers.hpp"

namespace Galaxy {
int Texture::s_currentFreeActivationInt = 0;

Texture::Texture(int width, int height, GLenum format)
    : m_width(width)
    , m_height(height)
{
    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    glTextureStorage2D(m_id, 1, format, m_width, m_height);

    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // glGenerateTextureMipmap(m_id);

    m_initialized = true;

    checkOpenGLErrors("Texture init");
}

bool Texture::load(const unsigned char* data, size_t size, int width, int height, int channels)
{
    if (m_initialized)
        destroy();

    unsigned char* imageData = nullptr;
    m_width                  = width;
    m_height                 = height;

    // if (size > 0 && height == 0) {
    //     imageData = stbi_load_from_memory(data, static_cast<int>(size), &m_width, &m_height, &channels, 0);
    //     if (!imageData) {
    //         GLX_CORE_ERROR("Failed to load embedded texture from memory");
    //         return false;
    //     }
    // } else {
    //     imageData = const_cast<unsigned char*>(data);
    // }

    GLenum dataFormat, internalFormat;
    if (channels == 4) {
        internalFormat = GL_RGBA8;
        dataFormat     = GL_RGBA;
    } else if (channels == 3) {
        internalFormat = GL_RGB8;
        dataFormat     = GL_RGB8;
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    glTextureStorage2D(m_id, 1, internalFormat, m_width, m_height);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, dataFormat, GL_UNSIGNED_BYTE, imageData);

    // if (size > 0 && height == 0 && imageData)
    //     stbi_image_free(imageData);

    m_initialized = true;

    checkOpenGLErrors("Texture load");
    return true;
}

void Texture::activate(int textureLocation)
{
    int actInt = getAvailableActivationInt();
    glBindTexture(actInt, m_id);
    glUniform1i(textureLocation, actInt);
}

void Texture::destroy()
{
    glDeleteTextures(1, &m_id);
    m_initialized = false;
}
}