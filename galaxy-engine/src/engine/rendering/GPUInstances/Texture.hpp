#pragma once

#include "types/Render.hpp"
#include "pch.hpp"
#include "Log.hpp"

namespace Galaxy {
class Texture {
public:
    Texture() = default;
    Texture(unsigned char* data, int width, int height, int nbChannels, int depthLayerCount = 0);
    Texture(TextureFormat format, int width, int height, int depthLayerCount = 0);
    ~Texture();

    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void resize(int width, int height);
    void resize(int width, int height, int depthLayerCount);
    void setFormat(TextureFormat format);

    void init(unsigned char* data, int width, int height, int nbChannels, int depthLayerCount = 0);
    void resetActivationInt();
    void reserveActivationInt();

    inline unsigned int getId() const { return m_id; }
    inline unsigned int getLayerCount() const { return m_layerCount; }
    inline static void resetStaticActivationInt() { s_currentFreeActivationInt = 0; }
    static void clearReservedActivationInts();
    static int getAvailableActivationInt();

    void activate(int textureLocation);
    static void activate(unsigned int id, int layerCount, int textureLocation);

    void destroy();

private:
    unsigned int getInternalFormat(TextureFormat format);
    unsigned int getExternalFormat(TextureFormat format);
    unsigned int getType(TextureFormat format);

    unsigned int m_id = 0;
    TextureFormat m_format = TextureFormat::RGBA;

    unsigned int m_width  = 0;
    unsigned int m_height = 0;

    int m_activationInt = -1;
    unsigned int m_layerCount = 0;

    static int s_currentFreeActivationInt;
    static const int s_maxActivationInt = 64;
    static std::array<bool, s_maxActivationInt>s_reservedActivationInt;
};

struct Cubemap {
    Cubemap() = default;
    ~Cubemap();

    Cubemap(const Cubemap&)            = delete;
    Cubemap& operator=(const Cubemap&) = delete;
    Cubemap(Cubemap&& other) noexcept;
    Cubemap& operator=(Cubemap&& other) noexcept;

    void activate(unsigned int uniLoc);
    inline unsigned int getId() const { return m_id; }
    inline unsigned int getResolution() const { return m_resolution; }

    void destroy();
    void allocateFaces();
    void resize(unsigned int res);
    void setFormat(TextureFormat newFormat);

private:
    unsigned int m_id         = 0;
    unsigned int m_resolution = 0;
    TextureFormat m_format    = TextureFormat::RGBA;
};
} // namespace Galaxy
