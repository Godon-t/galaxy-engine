#pragma once

#include "types/Render.hpp"

namespace Galaxy {
class Texture {
public:
    Texture()
        : m_id(0)
        , m_format(TextureFormat::RGBA)
    {
    }

    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept
        : m_id(other.m_id)
        , m_format(other.m_format)
        , m_width(other.m_width)
        , m_height(other.m_height)
    {
        other.m_id = 0; // ← CRUCIAL
    }

    Texture& operator=(Texture&& other) noexcept
    {
        if (this != &other) {
            destroy();
            m_id       = other.m_id;
            m_format   = other.m_format;
            m_width    = other.m_width;
            m_height   = other.m_height;
            other.m_id = 0;
        }
        return *this;
    }

    Texture(unsigned char* data, int width, int height, int nbChannels);

    void resize(int width, int height);
    void setFormat(TextureFormat format);

    void init(unsigned char* data, int width, int height, int nbChannels);

    inline unsigned int getId() const { return m_id; }
    inline static void resetActivationInts() { s_currentFreeActivationInt = 0; }
    inline static int getAvailableActivationInt()
    {
        if (s_currentFreeActivationInt >= s_maxActivationInt)
            s_currentFreeActivationInt = 0;
        return s_currentFreeActivationInt++;
    }

    void activate(int textureLocation);

    void destroy();
    unsigned int m_id;

private:
    unsigned int getInternalFormat(TextureFormat format);
    unsigned int getExternalFormat(TextureFormat format);
    unsigned int getType(TextureFormat format);

    TextureFormat m_format;

    unsigned int m_width;
    unsigned int m_height;

    static int s_currentFreeActivationInt;
    static int s_maxActivationInt;
};

struct Cubemap {
    unsigned int cubemapID  = 0;
    unsigned int resolution = 0;
    bool useFloat           = true;
    TextureFormat format;

    Cubemap();

    void activate(unsigned int uniLoc);

    void destroy();
    void allocateFaces();
    void resize(unsigned int res);
    void setFormat(TextureFormat newFormat);
};
} // namespace Galaxy
