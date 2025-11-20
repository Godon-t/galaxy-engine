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
    void attachTexture(unsigned int attachment, Texture& texture, unsigned int target);
    void attachColorTexture(Texture texture);
    void attachDepthTexture(Texture texture);

private:
    FramebufferTextureFormat m_format;
    unsigned int m_fbo;
    unsigned int m_attachedColor;
    unsigned int m_attachedDepth;
    int m_width, m_height;

    bool m_externalColor = false;
    bool m_externalDepth = false;

    void invalidate();
};

class CubemapFrameBuffer {
public:
    CubemapFrameBuffer();
    CubemapFrameBuffer(int size);
    ~CubemapFrameBuffer() = default;

    void attachCubemap(Cubemap cubemap);

    void bind(int idx);
    void unbind();

    void destroy();

    void resize(unsigned int newSize);

private:
    unsigned int m_fbo, m_rbo;
    unsigned int m_size;

    Cubemap m_cubemap;

    void invalidate();
};
}