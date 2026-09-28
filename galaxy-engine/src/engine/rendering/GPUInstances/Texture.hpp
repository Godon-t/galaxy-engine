#pragma once

#include "types/Render.hpp"

#include <array>

namespace Galaxy {
namespace detail {

// Owns the OpenGL texture name and the texture unit assigned to it. Storage
// layout stays in Texture and Cubemap because it is target-specific.
class TextureObject {
public:
    TextureObject() = default;
    ~TextureObject();

    TextureObject(const TextureObject&)            = delete;
    TextureObject& operator=(const TextureObject&) = delete;
    TextureObject(TextureObject&& other) noexcept;
    TextureObject& operator=(TextureObject&& other) noexcept;

    void create(unsigned int target);
    void destroy();

    void activate(unsigned int target, int uniformLocation);
    void resetActivationUnit();
    void reserveActivationUnit();

    [[nodiscard]] unsigned int id() const { return m_id; }

    static void activate(unsigned int id, unsigned int target, int uniformLocation);
    static void resetActivationUnits();
    static void clearReservedActivationUnits();
    [[nodiscard]] static int getAvailableActivationUnit();

private:
    inline static constexpr int MaxActivationUnits = 64;

    unsigned int m_id = 0;
    int m_activationUnit = -1;

    static int s_currentFreeActivationUnit;
    static std::array<bool, MaxActivationUnits> s_reservedActivationUnits;
};

[[nodiscard]] unsigned int toOpenGLInternalFormat(TextureFormat format);
[[nodiscard]] unsigned int toOpenGLExternalFormat(TextureFormat format);
[[nodiscard]] unsigned int toOpenGLType(TextureFormat format);
[[nodiscard]] unsigned int toOpenGLWrap(TextureWrap wrap);
[[nodiscard]] unsigned int toOpenGLFilter(TextureFiltering filtering);

} // namespace detail

class Texture {
public:
    Texture() = default;
    Texture(unsigned char* data, unsigned int width, unsigned int height, unsigned int nbChannels, unsigned int depthLayerCount = 0);
    Texture(TextureFormat format, unsigned int width, unsigned int height, unsigned int depthLayerCount = 0);
    Texture(TextureFormat format, unsigned int width, unsigned int height, TextureFiltering filter, unsigned int depthLayerCount = 0);
    ~Texture() = default;

    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void regenerate();

    void resize(unsigned int width, unsigned int height);
    void resize(unsigned int width, unsigned int height, unsigned int depthLayerCount);
    void setFormat(TextureFormat format);
    void setFiltering(TextureFiltering filtering);
    void setWrap(TextureWrap wrapS, TextureWrap wrapT);

    void init(unsigned char* data, unsigned int width, unsigned int height, unsigned int nbChannels, unsigned int depthLayerCount = 0);
    void resetActivationInt();
    void reserveActivationInt();

    [[nodiscard]] unsigned int getId() const { return m_object.id(); }
    [[nodiscard]] unsigned int getLayerCount() const { return m_layerCount; }
    [[nodiscard]] unsigned int getWidth() const { return m_width; }
    [[nodiscard]] unsigned int getHeight() const { return m_height; }
    [[nodiscard]] TextureFormat getFormat() const { return m_format; }
    [[nodiscard]] TextureFiltering getFiltering() const { return m_filter; }

    static void resetStaticActivationInt() { detail::TextureObject::resetActivationUnits(); }
    static void clearReservedActivationInts() { detail::TextureObject::clearReservedActivationUnits(); }
    [[nodiscard]] static int getAvailableActivationInt() { return detail::TextureObject::getAvailableActivationUnit(); }

    void activate(int textureLocation);
    static void activate(unsigned int id, int layerCount, int textureLocation);

    void destroy();

private:
    [[nodiscard]] unsigned int target() const;
    void applySamplerState();

    detail::TextureObject m_object;
    TextureFormat m_format = TextureFormat::RGBA;
    TextureFiltering m_filter = TextureFiltering::LINEAR;
    TextureWrap m_wrapS = TextureWrap::REPEAT;
    TextureWrap m_wrapT = TextureWrap::REPEAT;

    unsigned int m_width  = 0;
    unsigned int m_height = 0;
    unsigned int m_layerCount = 0;
};

class Cubemap {
public:
    Cubemap() = default;
    Cubemap(TextureFormat format, unsigned int resolution,
        TextureFiltering filtering = TextureFiltering::LINEAR,
        TextureWrap wrap = TextureWrap::CLAMP_TO_EDGE);
    ~Cubemap() = default;

    Cubemap(const Cubemap&)            = delete;
    Cubemap& operator=(const Cubemap&) = delete;
    Cubemap(Cubemap&& other) noexcept;
    Cubemap& operator=(Cubemap&& other) noexcept;

    void regenerate();
    void allocateFaces();
    void beginFaceUpload();
    void initFace(unsigned int face, unsigned char* data, int width, int height, int nbChannels);

    void resize(unsigned int resolution);
    void setFormat(TextureFormat format);
    void setFiltering(TextureFiltering filtering);
    void setWrap(TextureWrap wrapS, TextureWrap wrapT,
        TextureWrap wrapR = TextureWrap::CLAMP_TO_EDGE);

    void activate(int uniformLocation);
    void resetActivationInt();
    void reserveActivationInt();

    [[nodiscard]] unsigned int getId() const { return m_object.id(); }
    [[nodiscard]] unsigned int getResolution() const { return m_resolution; }
    [[nodiscard]] TextureFormat getFormat() const { return m_format; }
    [[nodiscard]] TextureFiltering getFiltering() const { return m_filter; }

    void destroy();

private:
    void applySamplerState();

    detail::TextureObject m_object;
    unsigned int m_resolution = 0;
    TextureFormat m_format = TextureFormat::RGBA;
    TextureFiltering m_filter = TextureFiltering::LINEAR;
    TextureWrap m_wrapS = TextureWrap::CLAMP_TO_EDGE;
    TextureWrap m_wrapT = TextureWrap::CLAMP_TO_EDGE;
    TextureWrap m_wrapR = TextureWrap::CLAMP_TO_EDGE;
    std::array<bool, 6> m_initializedFaces {};
};

} // namespace Galaxy
