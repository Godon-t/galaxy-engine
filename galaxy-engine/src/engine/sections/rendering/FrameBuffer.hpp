#pragma once

#include "pch.hpp"

namespace Galaxy {
enum class FramebufferTextureFormat {
    None = 0,

    // Color
    RGBA8,
    // RED_INTEGER,

    // Depth/stencil
    // DEPTH24STENCIL8,

    // Defaults
    // Depth = DEPTH24STENCIL8
    Depth = RGBA8
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
    unsigned int m_fbo;
    unsigned int m_attachedColor;
    int m_width, m_height;

    void invalidate();
};
}