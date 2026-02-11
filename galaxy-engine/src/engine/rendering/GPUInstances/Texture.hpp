#pragma once

#include "types/Render.hpp"
#include "pch.hpp"
#include "Log.hpp"

namespace Galaxy {
class Texture {
public:
    Texture()
        : m_id(0)
        , m_format(TextureFormat::RGBA)
    {
    }

    Texture(unsigned char* data, int width, int height, int nbChannels);

    void resize(int width, int height);
    void setFormat(TextureFormat format);

    void init(unsigned char* data, int width, int height, int nbChannels);
    void resetActivationInt();
    void reserveActivationInt();

    inline unsigned int getId() const { return m_id; }
    inline static void resetStaticActivationInt() { s_currentFreeActivationInt = 0; }
    static void clearReservedActivationInts();
    inline static int getAvailableActivationInt()
    {
        int idx = s_currentFreeActivationInt;
        s_currentFreeActivationInt = (s_currentFreeActivationInt + 1) % s_maxActivationInt;

        if (!s_reservedActivationInt[idx])
            return idx;
        
        return getAvailableActivationInt();
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

    int m_activationInt = -1;

    static int s_currentFreeActivationInt;
    static const int s_maxActivationInt = 64;
    static std::array<bool, s_maxActivationInt>s_reservedActivationInt;
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
