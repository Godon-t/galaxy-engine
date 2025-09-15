#pragma once

#include "Texture.hpp"
#include "pch.hpp"

namespace Galaxy {
enum class FramebufferTextureFormat {
    None = 0,

    RGBA8,
    // RED_INTEGER,
    DEPTH24STENCIL8

    // Defaults
    // Depth = DEPTH24STENCIL8
};

class FrameBuffer {
public:
    FrameBuffer(int width, int height, FramebufferTextureFormat format);
    ~FrameBuffer() = default;

    void bind();
    void unbind();

    void destroy();

    inline unsigned int getColorTextureID() { return m_attachedColor; }
    void resize(unsigned int newWidth, unsigned int newHeight);

private:
    FramebufferTextureFormat m_format;
    unsigned int m_fbo, m_rbo;
    unsigned int m_attachedColor;
    int m_width, m_height;

    void invalidate();
};

class CubemapFrameBuffer {
public:
    CubemapFrameBuffer(Cubemap& cubemap);
    ~CubemapFrameBuffer() = default;

    void bind(int idx);
    void unbind();

    void destroy();

    inline unsigned int getCubemapID() { return m_cubemap.cubemapID; }

private:
    Cubemap& m_cubemap;
    unsigned int m_fbo, m_rbo;
};
}