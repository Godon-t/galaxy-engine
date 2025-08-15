#pragma once

namespace Galaxy {
struct Texture {
    Texture()
        : m_id(0)
        , m_initialized(false) {};

    Texture(int width, int height, unsigned int format);

    bool load(const unsigned char* data,
        size_t size,
        int width,
        int height,
        int channels);

    void activate(int textureLocation);

    void destroy();

    inline static void resetActivationInts() { s_currentFreeActivationInt = 0; }
    inline static int getAvailableActivationInt() { return s_currentFreeActivationInt++; }

    inline unsigned int getId() { return m_id; }

private:
    bool m_initialized;
    unsigned int m_id;
    int m_width;
    int m_height;

    static int s_currentFreeActivationInt;
};
}