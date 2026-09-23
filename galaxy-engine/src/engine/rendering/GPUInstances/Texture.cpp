#include "Texture.hpp"

#include "Log.hpp"
#include "rendering/OpenglHelper.hpp"

#include "gl_headers.hpp"

#include <utility>

namespace Galaxy {
int Texture::s_currentFreeActivationInt = 0;
std::array<bool, Texture::s_maxActivationInt> Texture::s_reservedActivationInt = std::array<bool, Texture::s_maxActivationInt>();

Texture::Texture(unsigned char* data, int width, int height, int nbChannels, int depthLayerCount)
{
    init(data, width, height, nbChannels, depthLayerCount);
}

Texture::Texture(TextureFormat format, int width, int height, int depthLayerCount)
    : m_format(format)
    , m_layerCount(depthLayerCount)
    , m_width(width)
    , m_height(height)
{
    regenerate();
}

Texture::~Texture()
{
    destroy();
}

Texture::Texture(Texture&& other) noexcept
    : m_id(std::exchange(other.m_id, 0))
    , m_format(other.m_format)
    , m_width(std::exchange(other.m_width, 0))
    , m_height(std::exchange(other.m_height, 0))
    , m_activationInt(std::exchange(other.m_activationInt, -1))
    , m_layerCount(std::exchange(other.m_layerCount, 0))
{
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this == &other)
        return *this;

    destroy();
    m_id            = std::exchange(other.m_id, 0);
    m_format        = other.m_format;
    m_width         = std::exchange(other.m_width, 0);
    m_height        = std::exchange(other.m_height, 0);
    m_activationInt = std::exchange(other.m_activationInt, -1);
    m_layerCount    = std::exchange(other.m_layerCount, 0);
    return *this;
}

void setOpenglWrap(GLuint id, bool horizontal, TextureWrap wrap){
    GLenum glWrap;

    if(wrap == TextureWrap::REPEAT)
        glWrap = GL_REPEAT;
    else if(wrap == TextureWrap::CLAMP_TO_BORDER)
        glWrap = GL_CLAMP_TO_BORDER;
    else if(wrap == TextureWrap::CLAMP_TO_EDGE)
        glWrap = GL_CLAMP_TO_EDGE;
    
    if(horizontal)
        glTextureParameteri(id, GL_TEXTURE_WRAP_T, glWrap);
    else
        glTextureParameteri(id, GL_TEXTURE_WRAP_S, glWrap);
}

void Texture::regenerate()
{
    destroy();

    if (m_width <= 0 || m_height <= 0) {
        return;
    }

    unsigned int internalFormat = getInternalFormat(m_format);

    const GLenum target = m_layerCount > 0 ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
    glCreateTextures(target, 1, &m_id);
    
    setOpenglWrap(m_id, true, m_wrapS);
    setOpenglWrap(m_id, false, m_wrapT);

    if (m_layerCount > 0)
        glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (m_format == TextureFormat::DEPTH24STENCIL8 || m_format == TextureFormat::DEPTH) {
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTextureParameterfv(m_id, GL_TEXTURE_BORDER_COLOR, borderColor);
    }

    if (m_layerCount > 0)
        glTextureStorage3D(m_id, 1, internalFormat, m_width, m_height, m_layerCount);
    else
        glTextureStorage2D(m_id, 1, internalFormat, m_width, m_height);
    checkOpenGLErrors("Texture resize");
}

void Texture::resize(int width, int height)
{
    if (width <= 0 || height <= 0) {
        return;
    }

    if (m_id != 0 && width == m_width && height == m_height)
        return;

    m_width  = width;
    m_height = height;

    regenerate();
}

void Texture::resize(int width, int height, int depthLayerCount)
{
    if (
        m_id != 0 && 
        width == m_width && 
        height == m_height &&
        depthLayerCount == static_cast<int>(m_layerCount))
        return;

    m_width = width;
    m_height = height;
    m_layerCount = depthLayerCount;
    regenerate();
}

void Texture::setFormat(TextureFormat format)
{
    if (format != m_format) {
        m_format = format;
        regenerate();
        checkOpenGLErrors("Texture set format");
    }
}

void Texture::setWrap(TextureWrap wrapS, TextureWrap wrapT)
{
    if(m_wrapS != wrapS || m_wrapT != wrapT){
        m_wrapS = wrapS;
        m_wrapT = wrapT;
        regenerate();
    }
}

void Texture::init(unsigned char* data, int width, int height, int nbChannels, int depthLayerCount)
{
    destroy();

    switch (nbChannels) {
    case 1:
        m_format = TextureFormat::RED;
        break;
    case 2:
        m_format = TextureFormat::RG;
        break;
    case 3:
        m_format = TextureFormat::RGB;
        break;
    case 4:
        m_format = TextureFormat::RGBA;
        break;
    default:
        GLX_CORE_ERROR("Warning: Unsupported texture format, defaulting to GL_RGBA\n");
        m_format = TextureFormat::RGBA;
    }

    GLenum format         = getExternalFormat(m_format);
    unsigned int type     = getType(m_format);

    m_layerCount = depthLayerCount;
    m_width = width;
    m_height = height;
    regenerate();

    if(data != nullptr && depthLayerCount > 0){
        glTextureSubImage3D(m_id, 0, 0, 0, 0, width, height, depthLayerCount, format, type, data);
    }
    else if (data != nullptr) {
        glTextureSubImage2D(m_id, 0, 0, 0, width, height, format, type, data);
    }

    checkOpenGLErrors("Texture load");
}

void Texture::resetActivationInt()
{
    m_activationInt = -1;
}

void Texture::reserveActivationInt()
{
    if (m_activationInt >= 0)
        s_reservedActivationInt[m_activationInt] = true;
}

void Texture::clearReservedActivationInts()
{
    for(int i=0; i<s_maxActivationInt; i++){
        s_reservedActivationInt[i] = false;
    }
}

int Texture::getAvailableActivationInt()
{
    //TODO: brute force way of checking available int
    for (int attempt = 0; attempt < s_maxActivationInt; ++attempt) {
        const int idx = s_currentFreeActivationInt;
        s_currentFreeActivationInt = (s_currentFreeActivationInt + 1) % s_maxActivationInt;
        if (!s_reservedActivationInt[idx])
            return idx;
    }

    GLX_CORE_ASSERT(false, "No texture unit available");
    return 0;
}

void Texture::activate(int textureLocation)
{
    if(m_activationInt == -1)
        m_activationInt = getAvailableActivationInt();
    glActiveTexture(GL_TEXTURE0 + m_activationInt);
    if(m_layerCount > 0)
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_id);
    else
        glBindTexture(GL_TEXTURE_2D, m_id);
    glUniform1i(textureLocation, m_activationInt);
    bool check = checkOpenGLErrors("Texture activation");
    if(check)
        GLX_CORE_ERROR("eror");
}

void Texture::activate(unsigned int id, int layerCount, int textureLocation)
{
    const int activationInt = getAvailableActivationInt();
    glActiveTexture(GL_TEXTURE0 + activationInt);
    glBindTexture(layerCount > 0 ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, id);
    glUniform1i(textureLocation, activationInt);
    s_reservedActivationInt[activationInt] = true;
}

void Texture::destroy()
{
    if (m_activationInt >= 0 && m_activationInt < s_maxActivationInt)
        s_reservedActivationInt[m_activationInt] = false;
    if (m_id != 0)
        glDeleteTextures(1, &m_id);
    m_id = 0;
    m_activationInt = -1;
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
    if (format == TextureFormat::RED)
        return GL_RED;
    if (format == TextureFormat::RG)
        return GL_RG;
    if (format == TextureFormat::RGB)
        return GL_RGB;
    if (format == TextureFormat::RGBA)
        return GL_RGBA;
    
    if (format == TextureFormat::DEPTH)
        return GL_DEPTH_COMPONENT;
    if (format == TextureFormat::DEPTH24STENCIL8)
        return GL_DEPTH_STENCIL;
    return 0;
}

unsigned int Texture::getType(TextureFormat format)
{
    if (format == TextureFormat::RED || format == TextureFormat::RG || format == TextureFormat::RGB || format == TextureFormat::RGBA)
        return GL_UNSIGNED_BYTE;
    if (format == TextureFormat::DEPTH)
        return GL_FLOAT;
    if (format == TextureFormat::DEPTH24STENCIL8)
        return GL_UNSIGNED_INT_24_8;
    return 0;
}

Cubemap::~Cubemap()
{
    destroy();
}

Cubemap::Cubemap(Cubemap&& other) noexcept
    : m_id(std::exchange(other.m_id, 0))
    , m_resolution(std::exchange(other.m_resolution, 0))
    , m_format(other.m_format)
{
}

Cubemap& Cubemap::operator=(Cubemap&& other) noexcept
{
    if (this == &other)
        return *this;

    destroy();
    m_id         = std::exchange(other.m_id, 0);
    m_resolution = std::exchange(other.m_resolution, 0);
    m_format     = other.m_format;
    return *this;
}

void Cubemap::activate(unsigned int uniLoc)
{
    int actInt = Texture::getAvailableActivationInt();
    glActiveTexture(GL_TEXTURE0 + actInt);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
    glUniform1i(uniLoc, actInt);
    checkOpenGLErrors("activate cubemap");
}

void Cubemap::destroy()
{
    if (m_id) {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
    m_resolution = 0;
}

void Cubemap::allocateFaces()
{
    if (m_id == 0)
        glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

    for (int i = 0; i < 6; i++) {
        if (m_format == TextureFormat::RGB) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8,
                m_resolution, m_resolution, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

        } else if (m_format == TextureFormat::RGBA) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8,
                m_resolution, m_resolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        } else if (m_format == TextureFormat::DEPTH) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24,
                m_resolution, m_resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        } else
            GLX_CORE_ERROR("(cubemap) unsupported texture format");
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    checkOpenGLErrors("Allocate faces");
}

void Cubemap::resize(unsigned int res)
{
    if (res != m_resolution || m_id == 0) {
        m_resolution = res;
        allocateFaces();
    }
}

void Cubemap::setFormat(TextureFormat newFormat)
{
    if (newFormat != m_format) {
        m_format = newFormat;
        if (m_resolution > 0)
            allocateFaces();
    }
}

} // namespace Galaxy
