#pragma once

#include "Resource.hpp"

namespace Galaxy {
struct Texture : public ResourceBase {
    Texture()
        : m_id(0) {};

    Texture(int width, int height, unsigned int format);
    bool load(const std::string& file) override;

    bool load(const unsigned char* data,
        size_t size,
        int width,
        int height,
        int channels);

    void activate(int textureLocation);

    void destroy();

    inline static void resetActivationInts() { s_currentFreeActivationInt = 0; }
    inline static int getAvailableActivationInt() { return s_currentFreeActivationInt++; }

    inline unsigned int getId() const { return m_id; }
    inline int getWidth() const { return m_width; }
    inline int getHeight() const { return m_height; }

private:
    unsigned int m_id;
    int m_width;
    int m_height;

    static int s_currentFreeActivationInt;
};
}