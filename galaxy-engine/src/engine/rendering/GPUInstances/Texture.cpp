#include "Texture.hpp"

#include "Log.hpp"
#include "gl_headers.hpp"
#include "rendering/OpenglHelper.hpp"

#include <algorithm>
#include <utility>

namespace Galaxy {
namespace detail {

int TextureObject::s_currentFreeActivationUnit = 0;
std::array<bool, TextureObject::MaxActivationUnits> TextureObject::s_reservedActivationUnits {};

TextureObject::~TextureObject()
{
    destroy();
}

TextureObject::TextureObject(TextureObject&& other) noexcept
    : m_id(std::exchange(other.m_id, 0))
    , m_activationUnit(std::exchange(other.m_activationUnit, -1))
{
}

TextureObject& TextureObject::operator=(TextureObject&& other) noexcept
{
    if (this == &other)
        return *this;

    destroy();
    m_id = std::exchange(other.m_id, 0);
    m_activationUnit = std::exchange(other.m_activationUnit, -1);
    return *this;
}

void TextureObject::create(unsigned int target)
{
    destroy();
    glCreateTextures(target, 1, &m_id);
}

void TextureObject::destroy()
{
    resetActivationUnit();
    if (m_id != 0)
        glDeleteTextures(1, &m_id);
    m_id = 0;
}

void TextureObject::activate(unsigned int target, int uniformLocation)
{
    if (m_activationUnit < 0)
        m_activationUnit = getAvailableActivationUnit();

    glActiveTexture(GL_TEXTURE0 + m_activationUnit);
    glBindTexture(target, m_id);
    glUniform1i(uniformLocation, m_activationUnit);
}

void TextureObject::resetActivationUnit()
{
    if (m_activationUnit >= 0 && m_activationUnit < MaxActivationUnits)
        s_reservedActivationUnits[m_activationUnit] = false;
    m_activationUnit = -1;
}

void TextureObject::reserveActivationUnit()
{
    if (m_activationUnit >= 0 && m_activationUnit < MaxActivationUnits)
        s_reservedActivationUnits[m_activationUnit] = true;
}

void TextureObject::activate(unsigned int id, unsigned int target, int uniformLocation)
{
    const int activationUnit = getAvailableActivationUnit();
    glActiveTexture(GL_TEXTURE0 + activationUnit);
    glBindTexture(target, id);
    glUniform1i(uniformLocation, activationUnit);
    s_reservedActivationUnits[activationUnit] = true;
}

void TextureObject::resetActivationUnits()
{
    s_currentFreeActivationUnit = 0;
}

void TextureObject::clearReservedActivationUnits()
{
    std::fill(s_reservedActivationUnits.begin(), s_reservedActivationUnits.end(), false);
}

int TextureObject::getAvailableActivationUnit()
{
    for (int attempt = 0; attempt < MaxActivationUnits; ++attempt) {
        const int index = s_currentFreeActivationUnit;
        s_currentFreeActivationUnit = (s_currentFreeActivationUnit + 1) % MaxActivationUnits;
        if (!s_reservedActivationUnits[index])
            return index;
    }

    GLX_CORE_ASSERT(false, "No texture unit available");
    return 0;
}

unsigned int toOpenGLInternalFormat(TextureFormat format)
{
    switch (format) {
    case TextureFormat::RED:
        return GL_R8;
    case TextureFormat::RG:
        return GL_RG8;
    case TextureFormat::RGB:
        return GL_RGB8;
    case TextureFormat::RGBA:
        return GL_RGBA8;
    case TextureFormat::DEPTH:
        return GL_DEPTH_COMPONENT24;
    case TextureFormat::DEPTH24STENCIL8:
        return GL_DEPTH24_STENCIL8;
    case TextureFormat::NONE:
        return 0;
    }
    return 0;
}

unsigned int toOpenGLExternalFormat(TextureFormat format)
{
    switch (format) {
    case TextureFormat::RED:
        return GL_RED;
    case TextureFormat::RG:
        return GL_RG;
    case TextureFormat::RGB:
        return GL_RGB;
    case TextureFormat::RGBA:
        return GL_RGBA;
    case TextureFormat::DEPTH:
        return GL_DEPTH_COMPONENT;
    case TextureFormat::DEPTH24STENCIL8:
        return GL_DEPTH_STENCIL;
    case TextureFormat::NONE:
        return 0;
    }
    return 0;
}

unsigned int toOpenGLType(TextureFormat format)
{
    switch (format) {
    case TextureFormat::RED:
    case TextureFormat::RG:
    case TextureFormat::RGB:
    case TextureFormat::RGBA:
        return GL_UNSIGNED_BYTE;
    case TextureFormat::DEPTH:
        return GL_FLOAT;
    case TextureFormat::DEPTH24STENCIL8:
        return GL_UNSIGNED_INT_24_8;
    case TextureFormat::NONE:
        return 0;
    }
    return 0;
}

unsigned int toOpenGLWrap(TextureWrap wrap)
{
    switch (wrap) {
    case TextureWrap::CLAMP_TO_EDGE:
        return GL_CLAMP_TO_EDGE;
    case TextureWrap::CLAMP_TO_BORDER:
        return GL_CLAMP_TO_BORDER;
    case TextureWrap::REPEAT:
        return GL_REPEAT;
    }
    return GL_REPEAT;
}

unsigned int toOpenGLFilter(TextureFiltering filtering)
{
    switch (filtering) {
    case TextureFiltering::NEAREST:
        return GL_NEAREST;
    case TextureFiltering::LINEAR:
        return GL_LINEAR;
    }
    return GL_LINEAR;
}

} // namespace detail

TextureFormat formatFromChannelCount(int channelCount)
{
    switch (channelCount) {
    case 1:
        return TextureFormat::RED;
    case 2:
        return TextureFormat::RG;
    case 3:
        return TextureFormat::RGB;
    case 4:
        return TextureFormat::RGBA;
    default:
        GLX_CORE_ERROR("Unsupported texture channel count: {0}", channelCount);
        return TextureFormat::RGBA;
    }
}

void setDepthBorderColor(unsigned int id, TextureFormat format)
{
    if (format != TextureFormat::DEPTH && format != TextureFormat::DEPTH24STENCIL8)
        return;

    constexpr float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTextureParameterfv(id, GL_TEXTURE_BORDER_COLOR, borderColor);
}

Texture::Texture(unsigned char* data, unsigned int width, unsigned int height, unsigned int nbChannels, unsigned int depthLayerCount)
{
    init(data, width, height, nbChannels, depthLayerCount);
}

Texture::Texture(TextureFormat format, unsigned int width, unsigned int height, unsigned int depthLayerCount)
    : m_format(format)
    , m_layerCount(depthLayerCount)
    , m_width(width)
    , m_height(height)
{
    regenerate();
}

Texture::Texture(TextureFormat format, unsigned int width, unsigned int height, TextureFiltering filter, unsigned int depthLayerCount)
    : m_format(format)
    , m_filter(filter)
    , m_layerCount(depthLayerCount)
    , m_width(width)
    , m_height(height)
{
    regenerate();
}

Texture::Texture(Texture&& other) noexcept
    : m_object(std::move(other.m_object))
    , m_format(other.m_format)
    , m_filter(other.m_filter)
    , m_wrapS(other.m_wrapS)
    , m_wrapT(other.m_wrapT)
    , m_width(std::exchange(other.m_width, 0))
    , m_height(std::exchange(other.m_height, 0))
    , m_layerCount(std::exchange(other.m_layerCount, 0))
{
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this == &other)
        return *this;

    m_object = std::move(other.m_object);
    m_format = other.m_format;
    m_filter = other.m_filter;
    m_wrapS = other.m_wrapS;
    m_wrapT = other.m_wrapT;
    m_width = std::exchange(other.m_width, 0);
    m_height = std::exchange(other.m_height, 0);
    m_layerCount = std::exchange(other.m_layerCount, 0);
    return *this;
}

unsigned int Texture::target() const
{
    return m_layerCount > 0 ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
}

void Texture::applySamplerState()
{
    if (getId() == 0)
        return;

    glTextureParameteri(getId(), GL_TEXTURE_WRAP_S, detail::toOpenGLWrap(m_wrapS));
    glTextureParameteri(getId(), GL_TEXTURE_WRAP_T, detail::toOpenGLWrap(m_wrapT));
    if (m_layerCount > 0)
        glTextureParameteri(getId(), GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    const unsigned int glFilter = detail::toOpenGLFilter(m_filter);
    glTextureParameteri(getId(), GL_TEXTURE_MIN_FILTER, glFilter);
    glTextureParameteri(getId(), GL_TEXTURE_MAG_FILTER, glFilter);
}

void Texture::regenerate()
{
    if (m_width == 0 || m_height == 0) {
        m_object.destroy();
        return;
    }

    const unsigned int internalFormat = detail::toOpenGLInternalFormat(m_format);
    if (internalFormat == 0) {
        GLX_CORE_ERROR("Cannot allocate a texture with an unsupported format");
        m_object.destroy();
        return;
    }

    m_object.create(target());
    applySamplerState();
    setDepthBorderColor(getId(), m_format);

    if (m_layerCount > 0)
        glTextureStorage3D(getId(), 1, internalFormat, m_width, m_height, m_layerCount);
    else
        glTextureStorage2D(getId(), 1, internalFormat, m_width, m_height);

    checkOpenGLErrors("Texture regeneration");
}

void Texture::resize(unsigned int width, unsigned int height)
{
    resize(width, height, m_layerCount);
}

void Texture::resize(unsigned int width, unsigned int height, unsigned int depthLayerCount)
{
    if (width <= 0 || height <= 0 || depthLayerCount < 0)
        return;

    if (getId() != 0
        && width == m_width
        && height == m_height
        && depthLayerCount == m_layerCount) {
        return;
    }

    m_width = width;
    m_height = height;
    m_layerCount =  depthLayerCount;
    regenerate();
}

void Texture::setFormat(TextureFormat format)
{
    if (format == m_format)
        return;

    m_format = format;
    regenerate();
}

void Texture::setFiltering(TextureFiltering filtering)
{
    if (filtering == m_filter)
        return;

    m_filter = filtering;
    applySamplerState();
}

void Texture::setWrap(TextureWrap wrapS, TextureWrap wrapT)
{
    if (wrapS == m_wrapS && wrapT == m_wrapT)
        return;

    m_wrapS = wrapS;
    m_wrapT = wrapT;
    applySamplerState();
}

void Texture::init(unsigned char* data, unsigned int width, unsigned int height, unsigned int nbChannels, unsigned int depthLayerCount)
{
    if (width <= 0 || height <= 0 || depthLayerCount < 0) {
        GLX_CORE_ERROR("Cannot initialize a texture with invalid dimensions");
        return;
    }

    m_format = formatFromChannelCount(nbChannels);
    m_width = width;
    m_height = height;
    m_layerCount = depthLayerCount;
    regenerate();

    if (data == nullptr || getId() == 0)
        return;

    const unsigned int externalFormat = detail::toOpenGLExternalFormat(m_format);
    const unsigned int type = detail::toOpenGLType(m_format);
    if (m_layerCount > 0) {
        glTextureSubImage3D(getId(), 0, 0, 0, 0,
            m_width, m_height, m_layerCount, externalFormat, type, data);
    } else {
        glTextureSubImage2D(getId(), 0, 0, 0,
            m_width, m_height, externalFormat, type, data);
    }

    checkOpenGLErrors("Texture upload");
}

void Texture::resetActivationInt()
{
    m_object.resetActivationUnit();
}

void Texture::reserveActivationInt()
{
    m_object.reserveActivationUnit();
}

void Texture::activate(int textureLocation)
{
    m_object.activate(target(), textureLocation);
    checkOpenGLErrors("Texture activation");
}

void Texture::activate(unsigned int id, int layerCount, int textureLocation)
{
    const unsigned int textureTarget = layerCount > 0 ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
    detail::TextureObject::activate(id, textureTarget, textureLocation);
}

void Texture::destroy()
{
    m_object.destroy();
}

Cubemap::Cubemap(TextureFormat format, unsigned int resolution,
    TextureFiltering filtering, TextureWrap wrap)
    : m_resolution(resolution)
    , m_format(format)
    , m_filter(filtering)
    , m_wrapS(wrap)
    , m_wrapT(wrap)
    , m_wrapR(wrap)
{
    regenerate();
}

Cubemap::Cubemap(Cubemap&& other) noexcept
    : m_object(std::move(other.m_object))
    , m_resolution(std::exchange(other.m_resolution, 0))
    , m_format(other.m_format)
    , m_filter(other.m_filter)
    , m_wrapS(other.m_wrapS)
    , m_wrapT(other.m_wrapT)
    , m_wrapR(other.m_wrapR)
    , m_initializedFaces(std::exchange(other.m_initializedFaces, std::array<bool, 6> {}))
{
}

Cubemap& Cubemap::operator=(Cubemap&& other) noexcept
{
    if (this == &other)
        return *this;

    m_object = std::move(other.m_object);
    m_resolution = std::exchange(other.m_resolution, 0);
    m_format = other.m_format;
    m_filter = other.m_filter;
    m_wrapS = other.m_wrapS;
    m_wrapT = other.m_wrapT;
    m_wrapR = other.m_wrapR;
    m_initializedFaces = std::exchange(other.m_initializedFaces, std::array<bool, 6> {});
    return *this;
}

void Cubemap::applySamplerState()
{
    if (getId() == 0)
        return;

    glTextureParameteri(getId(), GL_TEXTURE_WRAP_S, detail::toOpenGLWrap(m_wrapS));
    glTextureParameteri(getId(), GL_TEXTURE_WRAP_T, detail::toOpenGLWrap(m_wrapT));
    glTextureParameteri(getId(), GL_TEXTURE_WRAP_R, detail::toOpenGLWrap(m_wrapR));

    const unsigned int glFilter = detail::toOpenGLFilter(m_filter);
    glTextureParameteri(getId(), GL_TEXTURE_MIN_FILTER, glFilter);
    glTextureParameteri(getId(), GL_TEXTURE_MAG_FILTER, glFilter);
}

void Cubemap::regenerate()
{
    m_initializedFaces.fill(false);

    if (m_resolution == 0) {
        m_object.destroy();
        return;
    }

    const unsigned int internalFormat = detail::toOpenGLInternalFormat(m_format);
    if (internalFormat == 0) {
        GLX_CORE_ERROR("Cannot allocate a cubemap with an unsupported format");
        m_object.destroy();
        return;
    }

    m_object.create(GL_TEXTURE_CUBE_MAP);
    applySamplerState();
    setDepthBorderColor(getId(), m_format);
    glTextureStorage2D(getId(), 1, internalFormat, m_resolution, m_resolution);
    checkOpenGLErrors("Cubemap regeneration");
}

void Cubemap::allocateFaces()
{
    regenerate();
}

void Cubemap::beginFaceUpload()
{
    m_initializedFaces.fill(false);
}

void Cubemap::initFace(unsigned int face, unsigned char* data,
    int width, int height, int nbChannels)
{
    if (face >= 6 || width <= 0 || height <= 0 || width != height) {
        GLX_CORE_ERROR("Cannot upload cubemap face {0} with dimensions {1}x{2}", face, width, height);
        return;
    }

    const TextureFormat format = formatFromChannelCount(nbChannels);
    const bool hasInitializedFace = std::any_of(
        m_initializedFaces.begin(), m_initializedFaces.end(), [](bool initialized) { return initialized; });
    const bool storageDoesNotMatch =
        m_resolution != static_cast<unsigned int>(width) || m_format != format;

    if (hasInitializedFace && storageDoesNotMatch) {
        GLX_CORE_ERROR("Cubemap faces must share the same resolution and format");
        return;
    }

    if (getId() == 0 || storageDoesNotMatch) {
        m_resolution = static_cast<unsigned int>(width);
        m_format = format;
        regenerate();
    }

    if (data == nullptr || getId() == 0)
        return;

    glTextureSubImage3D(getId(), 0, 0, 0, face,
        width, height, 1,
        detail::toOpenGLExternalFormat(m_format),
        detail::toOpenGLType(m_format), data);
    m_initializedFaces[face] = true;
    checkOpenGLErrors("Cubemap face upload");
}

void Cubemap::resize(unsigned int resolution)
{
    if (resolution == 0)
        return;
    if (getId() != 0 && resolution == m_resolution)
        return;

    m_resolution = resolution;
    regenerate();
}

void Cubemap::setFormat(TextureFormat format)
{
    if (format == m_format)
        return;

    m_format = format;
    if (m_resolution > 0)
        regenerate();
}

void Cubemap::setFiltering(TextureFiltering filtering)
{
    if (filtering == m_filter)
        return;

    m_filter = filtering;
    applySamplerState();
}

void Cubemap::setWrap(TextureWrap wrapS, TextureWrap wrapT, TextureWrap wrapR)
{
    if (wrapS == m_wrapS && wrapT == m_wrapT && wrapR == m_wrapR)
        return;

    m_wrapS = wrapS;
    m_wrapT = wrapT;
    m_wrapR = wrapR;
    applySamplerState();
}

void Cubemap::activate(int uniformLocation)
{
    m_object.activate(GL_TEXTURE_CUBE_MAP, uniformLocation);
    checkOpenGLErrors("Cubemap activation");
}

void Cubemap::resetActivationInt()
{
    m_object.resetActivationUnit();
}

void Cubemap::reserveActivationInt()
{
    m_object.reserveActivationUnit();
}

void Cubemap::destroy()
{
    m_initializedFaces.fill(false);
    m_object.destroy();
}

} // namespace Galaxy
