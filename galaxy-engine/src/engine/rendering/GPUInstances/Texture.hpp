#pragma once

namespace Galaxy {
class Texture {
public:
    Texture()
        : m_id(0)
    {
    }

    Texture(unsigned char* data, int width, int height, int nbChannels);

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

private:
    unsigned int m_id;
    unsigned int m_format;

    static int s_currentFreeActivationInt;
    static int s_maxActivationInt;
};

struct Cubemap {
    unsigned int cubemapID  = 0;
    unsigned int resolution = 0;
    bool useFloat           = false;

    Cubemap();

    void activate(unsigned int uniLoc);

    void destroy();
    void allocateFaces(unsigned int res);
    void resize(unsigned int res);
};
} // namespace Galaxy
