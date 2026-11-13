#include "FrameBuffer.hpp"

#include "Core.hpp"
#include "gl_headers.hpp"
#include "rendering/OpenglHelper.hpp"
#include "core/Log.hpp"
#include "rendering/GPUInstances/Texture.hpp"

#include <utility>

namespace Galaxy {
namespace {
class ScopedFramebufferBinding {
public:
    explicit ScopedFramebufferBinding(unsigned int framebuffer)
    {
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &m_draw);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &m_read);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    }

    ~ScopedFramebufferBinding()
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(m_draw));
        glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(m_read));
    }

    ScopedFramebufferBinding(const ScopedFramebufferBinding&) = delete;
    ScopedFramebufferBinding& operator=(const ScopedFramebufferBinding&) = delete;

private:
    GLint m_draw = 0;
    GLint m_read = 0;
};
} // namespace

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
    , m_targetMip(other.m_targetMip)
    , m_targetDepthLayer(other.m_targetDepthLayer)
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
    m_targetMip       = other.m_targetMip;
    m_targetDepthLayer = other.m_targetDepthLayer;
    m_colorAttachments = std::move(other.m_colorAttachments);
    m_depthAttachment  = std::move(other.m_depthAttachment);
    return *this;
}

void FrameBuffer::bind(int depthLayer)
{
    m_targetDepthLayer = depthLayer;
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    applyAttachments();
}

void FrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::resize(unsigned int newWidth, unsigned int newHeight, int depthLayerCount)
{
    const int requestedLayerCount = depthLayerCount < 0
        ? static_cast<int>(m_depthLayerCount)
        : depthLayerCount;

    if (m_width == static_cast<int>(newWidth)
        && m_height == static_cast<int>(newHeight)
        && static_cast<int>(m_depthLayerCount) == requestedLayerCount)
        return;

    m_width  = static_cast<int>(newWidth);
    m_height = static_cast<int>(newHeight);
    m_depthLayerCount = static_cast<unsigned int>(requestedLayerCount);
    invalidate();
}

void FrameBuffer::setMipTarget(unsigned int newMip)
{
    if (m_targetMip == newMip)
        return;

    ScopedFramebufferBinding binding(m_fbo);
    m_targetMip = newMip;
    applyAttachments();
    GLX_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
        "Framebuffer not complete after mip selection");
}

bool FrameBuffer::attachColorTexture(Texture& texture, int idx)
{
    if (!usesColor()) {
        GLX_CORE_ERROR("Can't bind texture to framebuffer's color attachment: {0}", idx);
        return false;
    }

    ScopedFramebufferBinding binding(m_fbo);
    m_colorAttachments[idx].borrow(texture);
    applyAttachments();

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    GLX_CORE_ASSERT(complete, "Framebuffer not complete after texture attach");

    return complete;
}

bool FrameBuffer::attachDepthTexture(Texture& texture)
{
    if (!usesDepth()) {
        GLX_CORE_ERROR("Cannot attach an incompatible depth texture to framebuffer");
        return false;
    }

    ScopedFramebufferBinding binding(m_fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 0, 0);
    m_depthAttachment.borrow(texture);
    m_depthLayerCount = texture.getLayerCount();
    m_targetDepthLayer = 0;
    applyAttachments();

    if (!usesColor()) {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    GLX_CORE_ASSERT(complete, "Framebuffer not complete after texture attach");
    return complete;
}

void FrameBuffer::invalidate()
{
    if (m_fbo == 0)
        glCreateFramebuffers(1, &m_fbo);
    ScopedFramebufferBinding binding(m_fbo);

    for (unsigned int i = 0; i < m_colorAttachments.size(); ++i)
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, 0, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 0, 0);

    if (usesColor()) {
        m_colorAttachments.resize(m_colorsCount);
        for (auto& attachment : m_colorAttachments) {
            if (attachment.get() == nullptr)
                attachment.makeOwned(TextureFormat::RGBA, m_width, m_height);
            else if (attachment.isOwned())
                attachment.get()->resize(m_width, m_height);
        }
    } else {
        m_colorAttachments.clear();
    }

    if (usesDepth()) {
        if (m_depthAttachment.get() == nullptr)
            m_depthAttachment.makeOwned(depthFormat(), m_width, m_height, m_depthLayerCount);
        else if (m_depthAttachment.isOwned()) {
            m_depthAttachment.get()->setFormat(depthFormat());
            m_depthAttachment.get()->resize(m_width, m_height, m_depthLayerCount);
        }
        const unsigned int layers = m_depthAttachment.get()->getLayerCount();
        if (layers == 0 || m_targetDepthLayer < 0
            || static_cast<unsigned int>(m_targetDepthLayer) >= layers)
            m_targetDepthLayer = 0;
    } else {
        m_depthAttachment.reset();
    }

    if (!isMipTargetValid(m_targetMip))
        m_targetMip = 0;
    applyAttachments();

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
        glReadBuffer(drawBuffers.empty() ? GL_NONE : GL_COLOR_ATTACHMENT0);
    }

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    GLX_CORE_ASSERT(complete, "Framebuffer not complete");

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
    const auto* depth = m_depthAttachment.get();
    const TextureFormat format = depth != nullptr ? depth->getFormat() : depthFormat();
    return format == TextureFormat::DEPTH24STENCIL8 ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
}

bool FrameBuffer::isMipTargetValid(unsigned int mip) const
{
    bool selectsMip = false;
    for (const auto& attachment : m_colorAttachments) {
        if (attachment.get() != nullptr) {
            selectsMip = true;
            if (mip >= attachment.get()->getMipLevels())
                return false;
        }
    }
    if (m_depthAttachment.get() != nullptr && !m_depthAttachment.isOwned()) {
        selectsMip = true;
        if (mip >= m_depthAttachment.get()->getMipLevels())
            return false;
    }
    return mip == 0 || selectsMip;
}

void FrameBuffer::applyAttachments()
{
    if (usesColor()) {
        for (unsigned int i = 0; i < m_colorsCount; ++i) {
            const auto& attachment = m_colorAttachments[i];
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, attachment.get()->getId(), m_targetMip);
        }
    }

    if (usesDepth() && m_depthAttachment.get() != nullptr) {
        const auto& depth = *m_depthAttachment.get();
        const unsigned int mip = m_depthAttachment.isOwned() ? 0 : m_targetMip;
        if (depth.getLayerCount() > 0)
            glFramebufferTextureLayer(GL_FRAMEBUFFER, depthAttachmentPoint(), depth.getId(), mip, m_targetDepthLayer);
        else
            glFramebufferTexture2D(GL_FRAMEBUFFER, depthAttachmentPoint(), GL_TEXTURE_2D, depth.getId(), mip);
    }
}

unsigned int FrameBuffer::readPixel(unsigned int attachment, math::ivec2 coords)
{
    ScopedFramebufferBinding binding(m_fbo);
    GLuint res;

    glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment);

    glReadPixels(coords.x, coords.y, 1, 1,
                GL_RED_INTEGER, GL_UNSIGNED_INT, &res);
    return res;
}


CubemapFrameBuffer::CubemapFrameBuffer()
    : CubemapFrameBuffer(512)
{
}

void CubemapFrameBuffer::CubemapAttachment::makeOwned(TextureFormat format, unsigned int size)
{
    borrowed = nullptr;
    owned = std::make_unique<Cubemap>(format, size);
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
    , m_targetMip(other.m_targetMip)
    , m_targetFace(other.m_targetFace)
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
    m_targetMip     = other.m_targetMip;
    m_targetFace    = other.m_targetFace;
    m_colorCubemaps = std::move(other.m_colorCubemaps);
    m_depthCubemap  = std::move(other.m_depthCubemap);
    return *this;
}

bool CubemapFrameBuffer::attachDepthCubemap(Cubemap& cubemap)
{
    if (
        cubemap.getFormat() != TextureFormat::DEPTH && 
        cubemap.getFormat() != TextureFormat::DEPTH24STENCIL8
    ) {
        GLX_CORE_ERROR("Cannot attach an incompatible depth cubemap to framebuffer");
        return false;
    }

    ScopedFramebufferBinding binding(m_fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 0, 0);
    m_depthCubemap.borrow(cubemap);
    applyAttachments();
    return true;
}

bool CubemapFrameBuffer::attachColorCubemap(Cubemap& cubemap, int idx)
{
    if (idx >= static_cast<int>(m_colorCubemaps.size()))
        m_colorCubemaps.resize(idx + 1);

    ScopedFramebufferBinding binding(m_fbo);
    m_colorCubemaps[idx].borrow(cubemap);
    applyAttachments();
    return true;
}

void CubemapFrameBuffer::bind(int idx)
{
    if (idx < 0 || idx >= 6) {
        GLX_CORE_ERROR("Invalid cubemap face index: {0}", idx);
        return;
    }

    m_targetFace = idx;
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    applyAttachments();

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
    if (m_size == newSize)
        return;
    m_size = newSize;
    invalidate();
}

void CubemapFrameBuffer::setMipTarget(unsigned int newMip)
{
    if (m_targetMip == newMip)
        return;
    if (!isMipTargetValid(newMip)) {
        GLX_CORE_ERROR("Cubemap framebuffer mip {0} is not allocated on its attachments", newMip);
        return;
    }

    ScopedFramebufferBinding binding(m_fbo);
    m_targetMip = newMip;
    applyAttachments();
    GLX_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
        "Cubemap framebuffer not complete after mip selection");
}

void CubemapFrameBuffer::invalidate()
{
    if (m_fbo == 0)
        glCreateFramebuffers(1, &m_fbo);
    ScopedFramebufferBinding binding(m_fbo);

    if (m_depthCubemap.isOwned())
        m_depthCubemap.get()->resize(m_size);
    for (auto& attachment : m_colorCubemaps) {
        if (attachment.isOwned())
            attachment.get()->resize(m_size);
    }
    if (!isMipTargetValid(m_targetMip))
        m_targetMip = 0;

    applyAttachments();
    checkOpenGLErrors("Cubemap frame buffer initialization");
}

bool CubemapFrameBuffer::isMipTargetValid(unsigned int mip) const
{
    bool selectsMip = false;
    for (const auto& attachment : m_colorCubemaps) {
        if (attachment.get() != nullptr) {
            selectsMip = true;
            if (mip >= attachment.get()->getMipLevels())
                return false;
        }
    }
    if (m_depthCubemap.get() != nullptr && !m_depthCubemap.isOwned()) {
        selectsMip = true;
        if (mip >= m_depthCubemap.get()->getMipLevels())
            return false;
    }
    return mip == 0 || selectsMip;
}

void CubemapFrameBuffer::applyAttachments()
{
    std::vector<GLenum> attachments(m_colorCubemaps.size(), GL_NONE);
    GLenum readBuffer = GL_NONE;
    for (unsigned int i = 0; i < m_colorCubemaps.size(); ++i) {
        const auto* cubemap = m_colorCubemaps[i].get();
        if (cubemap != nullptr) {
            attachments[i] = GL_COLOR_ATTACHMENT0 + i;
            if (readBuffer == GL_NONE)
                readBuffer = attachments[i];
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[i],
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + m_targetFace, cubemap->getId(), m_targetMip);
        }
    }

    const auto* depth = m_depthCubemap.get();
    if (depth != nullptr) {
        const unsigned int attachment = depth->getFormat() == TextureFormat::DEPTH24STENCIL8
            ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
        const unsigned int mip = m_depthCubemap.isOwned() ? 0 : m_targetMip;
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + m_targetFace, depth->getId(), mip);
    }

    if (attachments.empty())
        glDrawBuffer(GL_NONE);
    else
        glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());
    glReadBuffer(readBuffer);
}

void CubemapFrameBuffer::destroyFramebuffer()
{
    if (m_fbo != 0)
        glDeleteFramebuffers(1, &m_fbo);
    m_fbo = 0;
}
}
