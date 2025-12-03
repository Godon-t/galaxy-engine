#include "Texture.hpp"

#include "Log.hpp"
#include "rendering/OpenglHelper.hpp"

#include "gl_headers.hpp"

namespace Galaxy {
int Texture::s_currentFreeActivationInt = 0;
int Texture::s_maxActivationInt         = 64;

Texture::Texture(unsigned char* data, int width, int height, int nbChannels)
{
    init(data, width, height, nbChannels);
}

void Texture::resize(int width, int height)
{
    if (width == m_width && height == m_height)
        return;

    m_width  = width;
    m_height = height;

    unsigned int internalFormat = getInternalFormat(m_format);
    unsigned int format         = getExternalFormat(m_format);
    unsigned int type           = getType(m_format);

    if (m_id != 0)
        glDeleteTextures(1, &m_id);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (m_format == TextureFormat::DEPTH24STENCIL8 || m_format == TextureFormat::DEPTH) {
        float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    }

    glTextureStorage2D(m_id, 1, internalFormat, width, height);
    checkOpenGLErrors("Texture resize");
}

void Texture::setFormat(TextureFormat format)
{
    if (format != m_format) {
        m_format = format;

        if (m_id != 0) {
            glDeleteTextures(1, &m_id);
            m_id = 0;
        }

        int keepWidth  = m_width;
        int keepHeight = m_height;
        m_width        = 0;
        m_height       = 0;
        resize(keepWidth, keepHeight);
        checkOpenGLErrors("Texture set format");
    }
}

void Texture::init(unsigned char* data, int width, int height, int nbChannels)
{
    m_width  = width;
    m_height = height;

    switch (nbChannels) {
    case 1:
        m_format = TextureFormat::RED;
        break;
    case 3:
        m_format = TextureFormat::RGB;
        break;
    case 4:
        m_format = TextureFormat::RGBA;
        break;
    default:
        GLX_CORE_ERROR("Warning: Unsupported texture format, defaulting to GL_RGB8\n");
    }

    GLenum internalFormat = getInternalFormat(m_format);
    GLenum format         = getExternalFormat(m_format);
    unsigned int type     = getType(m_format);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);

    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureStorage2D(m_id, 1, internalFormat, width, height);
    glTextureSubImage2D(m_id, 0, 0, 0, width, height, format, type, data);
    glGenerateTextureMipmap(m_id);

    checkOpenGLErrors("Texture load");
}

void Texture::activate(int textureLocation)
{
    int actInt = getAvailableActivationInt();
    glActiveTexture(GL_TEXTURE0 + actInt);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glUniform1i(textureLocation, actInt);
    checkOpenGLErrors("Texture activation");
}

void Texture::destroy()
{
    glDeleteTextures(1, &m_id);
    m_id = 0;
}

unsigned int Texture::getInternalFormat(TextureFormat format)
{
    if (format == TextureFormat::RGBA)
        return GL_RGBA8;
    if (format == TextureFormat::RED)
        return GL_R8;
    if (format == TextureFormat::RGB)
        return GL_RGB8;
    if (format == TextureFormat::DEPTH)
        return GL_DEPTH_COMPONENT24;
    if (format == TextureFormat::DEPTH24STENCIL8)
        return GL_DEPTH24_STENCIL8;
    return 0;
}

unsigned int Texture::getExternalFormat(TextureFormat format)
{
    if (format == TextureFormat::RGBA)
        return GL_RGBA;
    if (format == TextureFormat::RED)
        return GL_RED;
    if (format == TextureFormat::RGB)
        return GL_RGB;
    if (format == TextureFormat::DEPTH)
        return GL_DEPTH_COMPONENT;
    if (format == TextureFormat::DEPTH24STENCIL8)
        return GL_DEPTH_STENCIL;
    return 0;
}

unsigned int Texture::getType(TextureFormat format)
{
    if (format == TextureFormat::RGBA || format == TextureFormat::RGB || format == TextureFormat::RED)
        return GL_UNSIGNED_BYTE;
    if (format == TextureFormat::DEPTH)
        return GL_FLOAT;
    if (format == TextureFormat::DEPTH24STENCIL8)
        return GL_UNSIGNED_INT_24_8;
    return 0;
}

Cubemap::Cubemap()
    : format(TextureFormat::RGBA)
{
    glGenTextures(1, &cubemapID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
    resize(16);
}

void Cubemap::activate(unsigned int uniLoc)
{
    int actInt = Texture::getAvailableActivationInt();
    glActiveTexture(GL_TEXTURE0 + actInt);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
    glUniform1i(uniLoc, actInt);
    checkOpenGLErrors("activate cubemap");
}

void Cubemap::destroy()
{
    if (cubemapID) {
        glDeleteTextures(1, &cubemapID);
        cubemapID = 0;
    }
}

void Cubemap::allocateFaces()
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

    for (int i = 0; i < 6; i++) {
        if (format == TextureFormat::RGB) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8,
                resolution, resolution, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        } else if (format == TextureFormat::RGBA) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8,
                resolution, resolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else if (format == TextureFormat::DEPTH) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24,
                resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    checkOpenGLErrors("Allocate faces");
}

void Cubemap::resize(unsigned int res)
{
    if (res != resolution) {
        resolution = res;
        allocateFaces();
    }
}

void Cubemap::setFormat(TextureFormat newFormat)
{
    if (newFormat != format) {
        format = newFormat;
        allocateFaces();
    }
}

} // namespace Galaxy
