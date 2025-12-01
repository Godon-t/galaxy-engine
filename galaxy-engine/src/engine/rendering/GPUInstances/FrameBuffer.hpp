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

    inline unsigned int getColorTextureID(int idx=0) { return m_attachedColors[idx]; }
    inline unsigned int getDepthTextureID() { return m_attachedDepth; }

    void setAsTextureUniform(unsigned int uniLocation, int textureIdx = -1);

    void resize(unsigned int newWidth, unsigned int newHeight);

    inline void setColorsCount(unsigned int count)
    {
        m_colorsCount = count;
        invalidate();
    }

    inline void setFormat(FramebufferTextureFormat format)
    {
        m_format = format;
        invalidate();
    }
    inline FramebufferTextureFormat getFormat() const { return m_format; }
    void attachColorTexture(Texture& texture, int idx);
    void attachDepthTexture(Texture& texture);
    void savePPM(char* filename);

private:
    FramebufferTextureFormat m_format;
    unsigned int m_colorsCount;
    unsigned int m_fbo;
    int m_width, m_height;

    std::vector<unsigned int> m_attachedColors;
    std::vector<bool> m_externalColors;

    unsigned int m_attachedDepth;
    bool m_externalDepth = false;

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
    Cubemap m_cubemap;

    void invalidate();
};
}