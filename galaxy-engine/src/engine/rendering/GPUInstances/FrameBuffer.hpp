#pragma once

#include "Texture.hpp"
#include "pch.hpp"
#include "types/Render.hpp"

namespace Galaxy {
class FrameBuffer {
public:
    FrameBuffer();
    FrameBuffer(int width, int height, FramebufferTextureFormat format);
    ~FrameBuffer() { }

    void bind();
    void unbind();

    void destroy();

    inline unsigned int getColorTextureID() { return m_attachedColor; }
    inline unsigned int getDepthTextureID() { return m_attachedDepth; }

    void resize(unsigned int newWidth, unsigned int newHeight);

    inline void setFormat(FramebufferTextureFormat format)
    {
        m_format = format;
        invalidate();
    }
    inline FramebufferTextureFormat getFormat() const { return m_format; }

private:
    FramebufferTextureFormat m_format;
    unsigned int m_fbo;
    unsigned int m_attachedColor;
    unsigned int m_attachedDepth;
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