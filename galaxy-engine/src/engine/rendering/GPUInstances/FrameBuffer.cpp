#include "FrameBuffer.hpp"

#include "Core.hpp"
#include "gl_headers.hpp"
#include "rendering/OpenglHelper.hpp"
#include "core/Log.hpp"

#include <fstream>
#include <utility>

namespace Galaxy {
void FrameBuffer::TextureAttachment::makeOwned(TextureFormat format, int width, int height, int depthLayerCount)
{
    borrowed = nullptr;
    owned    = std::make_unique<Texture>(format, width, height, depthLayerCount);
}

void FrameBuffer::TextureAttachment::borrow(Texture& texture)
{
    owned.reset();
    borrowed = &texture;
}

void FrameBuffer::TextureAttachment::reset()
{
    owned.reset();
    borrowed = nullptr;
}

FrameBuffer::FrameBuffer()
    : FrameBuffer(2, 2, FramebufferTextureFormat::RGBA8)
{
}

FrameBuffer::FrameBuffer(unsigned int width, unsigned int height, FramebufferTextureFormat format, unsigned int colorCount, unsigned int depthLayerCount)
    : m_format(format)
    , m_colorsCount(colorCount)
    , m_depthLayerCount(depthLayerCount)
    , m_width(static_cast<int>(width))
    , m_height(static_cast<int>(height))
{
    invalidate();
}

FrameBuffer::~FrameBuffer()
{
    destroy();
}

FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
    : m_format(other.m_format)
    , m_colorsCount(other.m_colorsCount)
    , m_depthLayerCount(other.m_depthLayerCount)
    , m_fbo(std::exchange(other.m_fbo, 0))
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_colorAttachments(std::move(other.m_colorAttachments))
    , m_depthAttachment(std::move(other.m_depthAttachment))
{
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept
{
    if (this == &other)
        return *this;

    destroy();
    m_format          = other.m_format;
    m_colorsCount     = other.m_colorsCount;
    m_depthLayerCount = other.m_depthLayerCount;
    m_fbo             = std::exchange(other.m_fbo, 0);
    m_width           = other.m_width;
    m_height          = other.m_height;
    m_colorAttachments = std::move(other.m_colorAttachments);
    m_depthAttachment  = std::move(other.m_depthAttachment);
    return *this;
}

void FrameBuffer::bind(int depthLayer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    if (m_depthLayerCount > 0 && m_depthAttachment.get() != nullptr)
        glFramebufferTextureLayer(GL_FRAMEBUFFER, depthAttachmentPoint(), m_depthAttachment.get()->getId(), 0, depthLayer);
}

void FrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::resize(unsigned int newWidth, unsigned int newHeight, unsigned int depthLayerCount)
{
    if (m_width == static_cast<int>(newWidth) && m_height == static_cast<int>(newHeight) && m_depthLayerCount == depthLayerCount)
        return;

    m_width  = static_cast<int>(newWidth);
    m_height = static_cast<int>(newHeight);
    m_depthLayerCount = depthLayerCount;
    invalidate();
}

void FrameBuffer::savePPM(const std::string& filename)
{
    for (int i = 0; i < m_colorsCount; i++) {
        std::string outputPath = filename + "_c" + std::to_string(i) + ".ppm";
        std::ofstream output_image(outputPath.c_str());

        /// READ THE CONTENT FROM THE FBO
        glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
        std::vector<float> pixels(static_cast<size_t>(m_width) * m_height * 4);
        glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_FLOAT, pixels.data());

        output_image << "P3" << std::endl;
        output_image << m_width << " " << m_height << std::endl;
        output_image << "255" << std::endl;

        int k = 0;
        for (int i = 0; i < m_width; i++) {
            for (int j = 0; j < m_height; j++) {
                output_image << (unsigned int)(255 * pixels[k]) << " " << (unsigned int)(255 * pixels[k + 1]) << " " << (unsigned int)(255 * pixels[k + 2]) << " ";
                k = k + 4;
            }
            output_image << std::endl;
        }
    }

    // If we have a depth component alongside color, save a _d.pgm file
    if (usesDepth() && getDepthTextureID() != 0) {
        std::string outputPath = filename + "_d.pgm";
        std::ofstream outputImage(outputPath.c_str());

        /// READ THE DEPTH CONTENT FROM THE FBO
        std::vector<float> depthPixels(static_cast<size_t>(m_width) * m_height);
        glReadPixels(0, 0, m_width, m_height, GL_DEPTH_COMPONENT, GL_FLOAT, depthPixels.data());

        outputImage << "P2" << std::endl;
        outputImage << m_width << " " << m_height << std::endl;
        outputImage << "255" << std::endl;

        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                int k            = y * m_width + x;
                unsigned int val = (unsigned int)(255.0f * depthPixels[k]);
                outputImage << val << " ";
            }
            outputImage << std::endl;
        }
    }
}

bool FrameBuffer::attachColorTexture(Texture& texture, int idx)
{
    if (idx < 0 || idx >= static_cast<int>(m_colorsCount)) {
        GLX_CORE_ERROR("Can't bind texture to framebuffer's color attachment: {0}", idx);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    texture.resize(m_width, m_height);
    m_colorAttachments[idx].borrow(texture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx, GL_TEXTURE_2D, texture.getId(), 0);

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    GLX_CORE_ASSERT(complete, "Framebuffer not complete after texture attach");
    return true;
}

bool FrameBuffer::attachDepthTexture(Texture& texture)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    texture.setFormat(depthFormat());
    texture.resize(m_width, m_height, m_depthLayerCount);
    m_depthAttachment.borrow(texture);

    if (m_depthLayerCount > 0)
        glFramebufferTextureLayer(GL_FRAMEBUFFER, depthAttachmentPoint(), texture.getId(), 0, 0);
    else
        glFramebufferTexture2D(GL_FRAMEBUFFER, depthAttachmentPoint(), GL_TEXTURE_2D, texture.getId(), 0);

    if (!usesColor()) {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    GLX_CORE_ASSERT(complete, "Framebuffer not complete after texture attach");
    return true;
}

// TODO: Should work with texture bound
void FrameBuffer::setAsTextureUniform(unsigned int uniLocation, int textureIdx)
{
    if (textureIdx >= 0) {
        if (textureIdx >= static_cast<int>(m_colorAttachments.size())) {
            GLX_CORE_ERROR("Unknown framebuffer color attachment: {0}", textureIdx);
            return;
        }
        Texture::activate(getColorTextureID(textureIdx), 0, uniLocation);
    } else {
        Texture::activate(getDepthTextureID(), m_depthLayerCount, uniLocation);
    }
}

void FrameBuffer::invalidate()
{
    destroyFramebuffer();

    glCreateFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    if (usesColor()) {
        m_colorAttachments.resize(m_colorsCount);
        for (unsigned int i = 0; i < m_colorsCount; ++i) {
            auto& attachment = m_colorAttachments[i];
            if (attachment.get() == nullptr)
                attachment.makeOwned(TextureFormat::RGBA, m_width, m_height);
            else
                attachment.get()->resize(m_width, m_height);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, attachment.get()->getId(), 0);
        }
    } else {
        m_colorAttachments.clear();
    }

    if (usesDepth()) {
        if (m_depthAttachment.get() == nullptr)
            m_depthAttachment.makeOwned(depthFormat(), m_width, m_height, m_depthLayerCount);
        else {
            m_depthAttachment.get()->setFormat(depthFormat());
            m_depthAttachment.get()->resize(m_width, m_height, m_depthLayerCount);
        }

        if (m_depthLayerCount > 0)
            glFramebufferTextureLayer(GL_FRAMEBUFFER, depthAttachmentPoint(), m_depthAttachment.get()->getId(), 0, 0);
        else
            glFramebufferTexture2D(GL_FRAMEBUFFER, depthAttachmentPoint(), GL_TEXTURE_2D, m_depthAttachment.get()->getId(), 0);
    } else {
        m_depthAttachment.reset();
    }

    if (m_format == FramebufferTextureFormat::DEPTH || m_format == FramebufferTextureFormat::DEPTH24STENCIL8) {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    if (m_format == FramebufferTextureFormat::DEPTH24RGBA8 || m_format == FramebufferTextureFormat::RGBA8) {
        std::vector<GLenum> drawBuffers;
        for (int i = 0; i < m_colorsCount; i++) {
            drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
        }
        glDrawBuffers(drawBuffers.size(), drawBuffers.data());
    }

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    GLX_CORE_ASSERT(complete, "Framebuffer not complete");

    unbind();
    checkOpenGLErrors("Frame buffer creation");
}

void FrameBuffer::destroy()
{
    destroyFramebuffer();
    m_colorAttachments.clear();
    m_depthAttachment.reset();
}

void FrameBuffer::destroyFramebuffer()
{
    if (m_fbo != 0)
        glDeleteFramebuffers(1, &m_fbo);
    m_fbo = 0;
}

unsigned int FrameBuffer::getColorTextureID(int idx) const
{
    if (idx < 0 || idx >= static_cast<int>(m_colorAttachments.size()) || m_colorAttachments[idx].get() == nullptr)
        return 0;
    return m_colorAttachments[idx].get()->getId();
}

unsigned int FrameBuffer::getDepthTextureID() const
{
    return m_depthAttachment.get() != nullptr ? m_depthAttachment.get()->getId() : 0;
}

void FrameBuffer::setColorsCount(unsigned int count)
{
    if (m_colorsCount == count)
        return;
    m_colorsCount = count;
    invalidate();
}

void FrameBuffer::setFormat(FramebufferTextureFormat format)
{
    if (m_format == format)
        return;
    m_format = format;
    invalidate();
}

bool FrameBuffer::usesColor() const
{
    return m_format == FramebufferTextureFormat::RGBA8 || m_format == FramebufferTextureFormat::DEPTH24RGBA8;
}

bool FrameBuffer::usesDepth() const
{
    return m_format == FramebufferTextureFormat::DEPTH || m_format == FramebufferTextureFormat::DEPTH24STENCIL8 || m_format == FramebufferTextureFormat::DEPTH24RGBA8;
}

TextureFormat FrameBuffer::depthFormat() const
{
    if (m_format == FramebufferTextureFormat::DEPTH24STENCIL8 || m_format == FramebufferTextureFormat::DEPTH24RGBA8)
        return TextureFormat::DEPTH24STENCIL8;
    return TextureFormat::DEPTH;
}

unsigned int FrameBuffer::depthAttachmentPoint() const
{
    return depthFormat() == TextureFormat::DEPTH24STENCIL8 ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
}

CubemapFrameBuffer::CubemapFrameBuffer()
    : CubemapFrameBuffer(512)
{
}

void CubemapFrameBuffer::CubemapAttachment::makeOwned(TextureFormat format, unsigned int size)
{
    borrowed = nullptr;
    owned    = std::make_unique<Cubemap>();
    owned->setFormat(format);
    owned->resize(size);
}

void CubemapFrameBuffer::CubemapAttachment::borrow(Cubemap& cubemap)
{
    owned.reset();
    borrowed = &cubemap;
}

void CubemapFrameBuffer::CubemapAttachment::reset()
{
    owned.reset();
    borrowed = nullptr;
}

CubemapFrameBuffer::CubemapFrameBuffer(unsigned int size, unsigned int colorCount)
    : m_size(size)
{
    m_depthCubemap.makeOwned(TextureFormat::DEPTH, size);
    m_colorCubemaps.resize(colorCount);
    for (auto& attachment : m_colorCubemaps)
        attachment.makeOwned(TextureFormat::RGB, size);
    invalidate();
}

CubemapFrameBuffer::~CubemapFrameBuffer()
{
    destroy();
}

CubemapFrameBuffer::CubemapFrameBuffer(CubemapFrameBuffer&& other) noexcept
    : m_fbo(std::exchange(other.m_fbo, 0))
    , m_size(other.m_size)
    , m_colorCubemaps(std::move(other.m_colorCubemaps))
    , m_depthCubemap(std::move(other.m_depthCubemap))
{
}

CubemapFrameBuffer& CubemapFrameBuffer::operator=(CubemapFrameBuffer&& other) noexcept
{
    if (this == &other)
        return *this;

    destroy();
    m_fbo           = std::exchange(other.m_fbo, 0);
    m_size          = other.m_size;
    m_colorCubemaps = std::move(other.m_colorCubemaps);
    m_depthCubemap  = std::move(other.m_depthCubemap);
    return *this;
}

bool CubemapFrameBuffer::attachDepthCubemap(Cubemap& cubemap)
{
    cubemap.setFormat(TextureFormat::DEPTH);
    cubemap.resize(m_size);
    m_depthCubemap.borrow(cubemap);
    return true;
}

bool CubemapFrameBuffer::attachColorCubemap(Cubemap& cubemap, int idx)
{
    if (idx < 0)
        return false;
    if (idx >= static_cast<int>(m_colorCubemaps.size()))
        m_colorCubemaps.resize(idx + 1);

    cubemap.setFormat(TextureFormat::RGB);
    cubemap.resize(m_size);
    m_colorCubemaps[idx].borrow(cubemap);
    return true;
}

void CubemapFrameBuffer::setAsCubemapUniform(unsigned int uniLocation, int textureIdx)
{
    if (textureIdx >= 0) {
        if (textureIdx >= static_cast<int>(m_colorCubemaps.size()) || m_colorCubemaps[textureIdx].get() == nullptr) {
            GLX_CORE_ERROR("Unknown cubemap framebuffer color attachment: {0}", textureIdx);
            return;
        }
        m_colorCubemaps[textureIdx].get()->activate(uniLocation);
    }
    else if (m_depthCubemap.get() != nullptr)
        m_depthCubemap.get()->activate(uniLocation);
}

void CubemapFrameBuffer::bind(int idx)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    // TODO: Depend on cubemap mode: color, depth or both
    std::vector<GLenum> attachments(m_colorCubemaps.size());
    for (int i = 0; i < m_colorCubemaps.size(); i++) {
        attachments[i] = GL_COLOR_ATTACHMENT0 + i;
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[i], GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx, m_colorCubemaps[i].get()->getId(), 0);
    }

    if (m_depthCubemap.get() != nullptr && m_depthCubemap.get()->getId() != 0) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx, m_depthCubemap.get()->getId(), 0);
    }

    if (attachments.empty()) {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    } else {
        glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());
    }

    checkOpenGLErrors("Bind framebuffer face idx");
    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    GLX_CORE_ASSERT(complete, "Cubemap framebuffer not complete after bind face {0}", idx);
}

void CubemapFrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CubemapFrameBuffer::destroy()
{
    destroyFramebuffer();
    m_colorCubemaps.clear();
    m_depthCubemap.reset();
}

void CubemapFrameBuffer::resize(unsigned int newSize)
{
    m_size = newSize;
    if (m_depthCubemap.get() != nullptr)
        m_depthCubemap.get()->resize(newSize);
    for(auto& cubemap : m_colorCubemaps){
        if (cubemap.get() != nullptr)
            cubemap.get()->resize(newSize);
    }
    invalidate();
}

void CubemapFrameBuffer::invalidate()
{
    destroyFramebuffer();

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glDrawBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkOpenGLErrors("Cubemap frame buffer initialization");
}

void CubemapFrameBuffer::destroyFramebuffer()
{
    if (m_fbo != 0)
        glDeleteFramebuffers(1, &m_fbo);
    m_fbo = 0;
}
}
