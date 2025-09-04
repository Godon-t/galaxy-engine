#pragma once

#include "Resource.hpp"

namespace Galaxy {
struct Image : public ResourceBase {
    Image() {};

    Image(int width, int height, int nbChannels);
    bool load(const std::string& file) override;
    bool load(const unsigned char* data, size_t size) override;

    inline int getWidth() const { return m_width; }
    inline int getHeight() const { return m_height; }
    inline int getNbChannels() const { return m_nbChannels; }
    inline unsigned char* getData() const { return m_data; }

    void destroy();

private:
    int m_width;
    int m_height;
    unsigned char* m_data;
    int m_nbChannels;
};
}