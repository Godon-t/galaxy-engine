#pragma once

#include "Texture.hpp"
#include "pch.hpp"
#include "types/Render.hpp"

namespace Galaxy {

// TODO: Change to use Texture object directly
// TODO: Add option for border
//    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
class FrameBuffer {
public:
    FrameBuffer();
    FrameBuffer(int width, int height);
    ~FrameBuffer() { destroy(); }

    // Move semantics
    FrameBuffer(const FrameBuffer&)            = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;

    FrameBuffer(FrameBuffer&& other) noexcept
        : m_fbo(other.m_fbo)
        , m_width(other.m_width)
        , m_height(other.m_height)
        , m_attachedColors(std::move(other.m_attachedColors))
        , m_attachedDepth(other.m_attachedDepth)
    {
        other.m_fbo           = 0;
        other.m_attachedDepth = nullptr;
    }

    FrameBuffer& operator=(FrameBuffer&& other) noexcept
    {
        if (this != &other) {
            destroy();
            m_fbo            = other.m_fbo;
            m_width          = other.m_width;
            m_height         = other.m_height;
            m_attachedColors = std::move(other.m_attachedColors);
            m_attachedDepth  = other.m_attachedDepth;

            other.m_fbo           = 0;
            other.m_attachedDepth = nullptr;
        }
        return *this;
    }

    void bind();
    void unbind();
    void destroy();

    inline unsigned int getColorTextureID(int idx = 0)
    {
        auto it = m_attachedColors.find(idx);
        return (it != m_attachedColors.end()) ? it->second->getId() : 0;
    }
    inline unsigned int getDepthTextureID()
    {
        return m_attachedDepth ? m_attachedDepth->getId() : 0;
    }

    void setAsTextureUniform(unsigned int uniLocation, int textureIdx = -1);
    void resize(unsigned int newWidth, unsigned int newHeight);
    void attachColorTexture(Texture& texture, int idx);
    void attachDepthTexture(Texture& texture);
    void savePPM(char* filename);

private:
    unsigned int m_fbo;
    int m_width, m_height;

    std::unordered_map<int, Texture*> m_attachedColors;
    Texture* m_attachedDepth = nullptr;

    void invalidate();
};

class CubemapFrameBuffer {
public:
    CubemapFrameBuffer();
    CubemapFrameBuffer(int size);
    ~CubemapFrameBuffer() = default;

    void attachDepthCubemap(Cubemap cubemap);
    void attachColorCubemap(Cubemap cubemap, int idx);

    void bind(int idx);
    void unbind();

    void destroy();

    void resize(unsigned int newSize);

private:
    unsigned int m_fbo;
    unsigned int m_size;

    std::vector<Cubemap> m_colorCubemaps;
    Cubemap m_depthCubemap;

    void invalidate();
};
}