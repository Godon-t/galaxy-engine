#include "FrameBuffer.hpp"

#include "Core.hpp"
#include "gl_headers.hpp"
#include "rendering/OpenglHelper.hpp"

namespace Galaxy {
void bindColorAttachmentTexture(GLuint* id, int width, int height, GLenum internalFormat, GLenum format)
{
    glCreateTextures(GL_TEXTURE_2D, 1, id);
    glBindTexture(GL_TEXTURE_2D, *id);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *id, 0);
}

void bindDepthAttachment(GLuint* id, int width, int height, GLenum format)
{
    glGenTextures(1, id);
    glBindTexture(GL_TEXTURE_2D, *id);

    // Decide pixel format and type based on requested internal format
    GLenum pixelFormat = GL_DEPTH_COMPONENT;
    GLenum pixelType   = GL_UNSIGNED_BYTE;

    if (format == GL_DEPTH24_STENCIL8) {
        pixelFormat = GL_DEPTH_STENCIL;
        pixelType   = GL_UNSIGNED_INT_24_8;
    } else if (format == GL_DEPTH_COMPONENT24) {
        pixelFormat = GL_DEPTH_COMPONENT;
        pixelType   = GL_UNSIGNED_INT;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
        pixelFormat, pixelType, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D, *id, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

FrameBuffer::FrameBuffer()
    : FrameBuffer(2, 2, FramebufferTextureFormat::RGBA8)
{
}

FrameBuffer::FrameBuffer(int width, int height, FramebufferTextureFormat format)
{
    m_format = format;
    m_width  = width;
    m_height = height;
    m_fbo    = 0;
    invalidate();
}
void FrameBuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void FrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::resize(unsigned int newWidth, unsigned int newHeight)
{
    m_width  = newWidth;
    m_height = newHeight;
    invalidate();
}

void FrameBuffer::attachTexture(unsigned int attachment, Texture& texture, unsigned int target)
{
    unsigned int textureId = texture.getId();
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    if (attachment == GL_COLOR_ATTACHMENT0) {
        if (!m_externalColor)
            glDeleteTextures(1, &m_attachedColor);

        texture.resize(m_width, m_height);
        texture.setFormat(TextureFormat::RGBA);

        m_externalColor = true;
        m_attachedColor = texture.getId();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, m_attachedColor, 0);
    } else if (attachment == GL_DEPTH_ATTACHMENT) {
        if (!m_externalDepth)
            glDeleteTextures(1, &m_attachedDepth);

        m_externalDepth = true;

        texture.resize(m_width, m_height);
        texture.setFormat(TextureFormat::DEPTH);

        m_attachedDepth = texture.getId();

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, m_attachedDepth, 0);
    }

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    GLX_CORE_ASSERT(complete, "Framebuffer not complete after texture attach");
}

void FrameBuffer::attachColorTexture(Texture& texture)
{
    attachTexture(GL_COLOR_ATTACHMENT0, texture, GL_TEXTURE_2D);
}

void FrameBuffer::attachDepthTexture(Texture& texture)
{
    attachTexture(GL_DEPTH_ATTACHMENT, texture, GL_TEXTURE_2D);
}

void FrameBuffer::invalidate()
{
    if (m_fbo != 0)
        destroy();

    glCreateFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // TODO: 3 cases, rgba, depth or rgba and depth
    if (m_format == FramebufferTextureFormat::RGBA8) {
        bindColorAttachmentTexture(&m_attachedColor, m_width, m_height, GL_RGBA8, GL_RGBA);
    } else if (m_format == FramebufferTextureFormat::DEPTH24STENCIL8) {
        bindDepthAttachment(&m_attachedDepth, m_width, m_height, GL_DEPTH24_STENCIL8);
    } else if (m_format == FramebufferTextureFormat::DEPTH24RGBA8) {
        bindColorAttachmentTexture(&m_attachedColor, m_width, m_height, GL_RGBA8, GL_RGBA);
        bindDepthAttachment(&m_attachedDepth, m_width, m_height, GL_DEPTH24_STENCIL8);
    } else if (m_format == FramebufferTextureFormat::DEPTH) {
        bindDepthAttachment(&m_attachedDepth, m_width, m_height, GL_DEPTH_COMPONENT24);
    } else {
        GLX_CORE_ASSERT(false, "Framebuffer format not handled");
    }

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    GLX_CORE_ASSERT(complete, "Framebuffer not complete");

    unbind();
    checkOpenGLErrors("Frame buffer creation");
}

void FrameBuffer::destroy()
{
    glDeleteTextures(1, &m_attachedColor);
    glDeleteTextures(1, &m_attachedDepth);
    glDeleteFramebuffers(1, &m_fbo);
    m_attachedColor = 0;
    m_attachedDepth = 0;
    m_fbo           = 0;
}

CubemapFrameBuffer::CubemapFrameBuffer()
    : CubemapFrameBuffer(512)
{
}

CubemapFrameBuffer::CubemapFrameBuffer(int size)
{
    m_size = size;
    invalidate();
}

void CubemapFrameBuffer::attachCubemap(Cubemap cubemap)
{
    m_cubemap = cubemap;
    resize(cubemap.resolution);
}

void CubemapFrameBuffer::bind(int idx)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    // TODO: Depend on cubemap mode: color, depth or both
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx, m_cubemap.cubemapID, 0);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx, m_cubemap.cubemapID, 0);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    GLX_CORE_ASSERT(complete, "Cubemap framebuffer not complete after bind face {0}", idx);
    checkOpenGLErrors("Bind framebuffer face idx");
}

void CubemapFrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CubemapFrameBuffer::destroy()
{
    glDeleteRenderbuffers(1, &m_rbo);
    glDeleteFramebuffers(1, &m_fbo);
}

void CubemapFrameBuffer::resize(unsigned int newSize)
{
    m_size = newSize;
    invalidate();
}

void CubemapFrameBuffer::invalidate()
{
    if (m_fbo != 0)
        destroy();

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_size, m_size);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkOpenGLErrors("Cubemap frame buffer initialization");
}
}
