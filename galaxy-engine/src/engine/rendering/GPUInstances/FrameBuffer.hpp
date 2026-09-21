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
    FrameBuffer(unsigned int width, unsigned int height, FramebufferTextureFormat format, unsigned int colorCount = 1, unsigned int depthLayerCount = 0);
    ~FrameBuffer();

    FrameBuffer(const FrameBuffer&)            = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;
    FrameBuffer(FrameBuffer&& other) noexcept;
    FrameBuffer& operator=(FrameBuffer&& other) noexcept;

    void bind(int depthLayer = 0);
    void unbind();

    void destroy();

    unsigned int getColorTextureID(int idx = 0) const;
    unsigned int getDepthTextureID() const;

    void setAsTextureUniform(unsigned int uniLocation, int textureIdx = -1);

    void resize(unsigned int newWidth, unsigned int newHeight, int depthLayerCount = -1);

    void setColorsCount(unsigned int count);
    void setFormat(FramebufferTextureFormat format);
    inline FramebufferTextureFormat getFormat() const { return m_format; }
    // Attached textures are borrowed and must outlive this framebuffer.
    bool attachColorTexture(Texture& texture, int idx);
    bool attachDepthTexture(Texture& texture);
    void savePPM(const std::string& filename);

private:
    struct TextureAttachment {
        std::unique_ptr<Texture> owned;
        Texture* borrowed = nullptr;

        Texture* get() { return borrowed != nullptr ? borrowed : owned.get(); }
        const Texture* get() const { return borrowed != nullptr ? borrowed : owned.get(); }
        void makeOwned(TextureFormat format, int width, int height, int depthLayerCount = 0);
        void borrow(Texture& texture);
        void reset();
    };

    FramebufferTextureFormat m_format = FramebufferTextureFormat::RGBA8;
    unsigned int m_colorsCount         = 1;
    unsigned int m_depthLayerCount     = 0;
    unsigned int m_fbo                 = 0;
    int m_width                        = 0;
    int m_height                       = 0;

    std::vector<TextureAttachment> m_colorAttachments;
    TextureAttachment m_depthAttachment;

    void invalidate();
    void destroyFramebuffer();
    bool usesColor() const;
    bool usesDepth() const;
    TextureFormat depthFormat() const;
    unsigned int depthAttachmentPoint() const;
};

class CubemapFrameBuffer {
public:
    CubemapFrameBuffer();
    CubemapFrameBuffer(unsigned int size, unsigned int colorCount = 0);
    ~CubemapFrameBuffer();

    CubemapFrameBuffer(const CubemapFrameBuffer&)            = delete;
    CubemapFrameBuffer& operator=(const CubemapFrameBuffer&) = delete;
    CubemapFrameBuffer(CubemapFrameBuffer&& other) noexcept;
    CubemapFrameBuffer& operator=(CubemapFrameBuffer&& other) noexcept;

    // Borrowed cubemaps must outlive this framebuffer.
    bool attachDepthCubemap(Cubemap& cubemap);
    bool attachColorCubemap(Cubemap& cubemap, int idx);

    void setAsCubemapUniform(unsigned int uniLocation, int textureIdx);


    void bind(int idx);
    void unbind();

    void destroy();

    void resize(unsigned int newSize);

private:
    struct CubemapAttachment {
        std::unique_ptr<Cubemap> owned;
        Cubemap* borrowed = nullptr;

        Cubemap* get() { return borrowed != nullptr ? borrowed : owned.get(); }
        const Cubemap* get() const { return borrowed != nullptr ? borrowed : owned.get(); }
        void makeOwned(TextureFormat format, unsigned int size);
        void borrow(Cubemap& cubemap);
        void reset();
    };

    unsigned int m_fbo  = 0;
    unsigned int m_size = 0;

    std::vector<CubemapAttachment> m_colorCubemaps;
    CubemapAttachment m_depthCubemap;

    void invalidate();
    void destroyFramebuffer();
};
}
